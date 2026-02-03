#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MAX6675.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define SDA 21
#define SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define thermoSO 19
#define thermoCS 5
#define thermoSCK 18

MAX6675 thermocouple(thermoSCK, thermoCS, thermoSO);

#define MANUAL_PIN 14
#define AUTO_PIN   27
#define BUTTON_PIN 26
#define RELAY_PIN 25

enum ManualState {
    MANUAL_S1_IDLE,
    MANUAL_S1_INPUT,
    MANUAL_S1_RUN,
    MANUAL_S1_DONE,

    MANUAL_TEMP_SET,
    MANUAL_TEMP_WAIT,
    MANUAL_TEMP_HEAT,
    MANUAL_TEMP_DONE
};

enum SystemMode {
    MODE_MANUAL,
    MODE_AUTO
};

ManualState manualState = MANUAL_S1_IDLE;
SystemMode currentMode = MODE_MANUAL;
SystemMode lastMode = MODE_MANUAL;

float temperatureC = 0;
float targetTemp = 0;

unsigned long remainingSeconds = 0;
unsigned long lastSecond = 0;
unsigned long stateTimer = 0;
unsigned long idleTimer = 0;
unsigned long situation1StartTime = 0;

bool tempConfirmed = false;

bool blinkState = true;
unsigned long blinkTimer = 0;

bool lastButton = HIGH;
unsigned long buttonPressTime = 0;
bool longPressState = false;

unsigned long lastTempRead = 0;
#define TEMP_READ_INTERVAL 500

void readTemperature() {
    if ((millis() - lastTempRead) >= TEMP_READ_INTERVAL) {
        lastTempRead = millis();
        double t = thermocouple.getCelsius();
        if (!isnan(t)) temperatureC = t;
        else Serial.println("Error reading temp");
    }
}

void readModeSwitch() {
    if (digitalRead(MANUAL_PIN) == LOW &&
        digitalRead(AUTO_PIN) == HIGH) {
        currentMode = MODE_MANUAL;
    } else if (digitalRead(AUTO_PIN) == LOW &&
               digitalRead(MANUAL_PIN) == HIGH) {
        currentMode = MODE_AUTO;
    }
}

void resetManual() {
//    manualState = MANUAL_S1_IDLE;
//    remainingSeconds = 0;
//    tempConfirmed = false;
//
//    situation1StartTime = millis();
//    idleTimer = millis();
//
//    digitalWrite(RELAY_PIN, HIGH);
}

void resetAuto() {
  digitalWrite(RELAY_PIN, HIGH);
  autoState = AUTO_SET_TEMP;
  userPressed = false;
  setTemp = temperatureC;
  idleTimer = millis();
}

void readModeSwitch() {
    if (digitalRead(MANUAL_PIN) == LOW &&
        digitalRead(AUTO_PIN) == HIGH) {
        currentMode = MODE_MANUAL;
    } else if (digitalRead(AUTO_PIN) == LOW &&
               digitalRead(MANUAL_PIN) == HIGH) {
        currentMode = MODE_AUTO;
    }
}

void handleManualState() {
    //if user is idle for more than 10s they would like to manually set temp
    if (manualState == MANUAL_S1_IDLE && (millis() - situation1StartTime >= 10000)) {
        targetTemp = (int) temperatureC; //UX friendly (starting from the detected temp)
        tempConfirmed = false;
        manualState = MANUAL_TEMP_SET;
        idleTimer = millis();
    }

    //starting situation-1
    if (manualState == MANUAL_S1_INPUT && (millis() - stateTimer) >= 3000) {
        manualState = MANUAL_S1_RUN;
        lastSecond = millis();
    }

    //timer countdown
    if (manualState == MANUAL_S1_RUN) {
        if ((millis() - lastSecond) >= 1000) {
            lastSecond = millis();
            if (remainingSeconds > 0) remainingSeconds--;
        }

        if (remainingSeconds == 0) {
            manualState = MANUAL_S1_DONE;
            stateTimer = millis();
        }
    }
    // Situation-2 timeout
    if (manualState == MANUAL_TEMP_SET && !tempConfirmed && (millis() - idleTimer) >= 10000) {
        resetManual();
    }
    if (manualState == MANUAL_TEMP_SET &&
        tempConfirmed &&
        millis() - stateTimer >= 3000) {
        manualState = MANUAL_TEMP_HEAT;
    }

    if (manualState == MANUAL_TEMP_HEAT &&
        temperatureC >= targetTemp) {
        manualState = MANUAL_TEMP_DONE;
        stateTimer = millis();
    }

    if (manualState == MANUAL_TEMP_DONE &&
        millis() - stateTimer >= 5000) {
        resetManual();
    }
}

void handleButtonManual() {
    bool btn = digitalRead(BUTTON_PIN);
    //checking if pressed
    if (lastButton == HIGH && btn == LOW) {
        buttonPressTime = millis();
        longPressState = false;
    }
    //long press check
    if (btn == LOW && !longPressState && (millis() - buttonPressTime) > 2000) {
        resetManual();
        longPressState = true;
    }

    if (lastButton == LOW && btn == HIGH && !longPressState) {
        if (manualState == MANUAL_S1_IDLE ||
            manualState == MANUAL_S1_INPUT ||
            manualState == MANUAL_S1_RUN) {
            remainingSeconds += 60;
            //you can add time when the heater is idle/running/or while input

            if (manualState != MANUAL_S1_RUN) {
                manualState = MANUAL_S1_INPUT;
                stateTimer = millis();
            }
        } else if (manualState == MANUAL_TEMP_SET) {
            targetTemp++;
            tempConfirmed = true;
            stateTimer = millis();
        }
    }
    lastButton = btn;
}

void handleAutoState() {

  if (millis() - blinkTimer >= 600) {
    blinkTimer = millis();
    blinkState = !blinkState;
  }

  // Situation-1 timeout -> Situation-2 ( Default automatic mode when no user input is detected.)
  if (autoState == AUTO_SET_TEMP &&
    !userPressed &&
    millis() - idleTimer >= 10000) {

      lowCut = temperatureC + 2;
      highCut = temperatureC + 7;
      stateTimer = millis();
      autoState = AUTO_WAIT_START;
    }

  // Situation-1 confirmed
  // Initial temperature selection window after reset.
  if (autoState == AUTO_SET_TEMP &&
      userPressed &&
      millis() - stateTimer >=3000) {
        lowCut = setTemp - 2;
        highCut = setTemp + 2;
        autoState = AUTO_HEAT_USER;
    }

  // Default wait -> heat
  if (autoState == AUTO_WAIT_START && 
      millis() - stateTimer >=3000) {
        autoState = AUTO_HEAT_DEFAULT;
      }
}

void handleButtonAuto() {
    bool btn = digitalRead(BUTTON_PIN);
    //checking if pressed
    if (lastButton == HIGH && btn == LOW) {
        buttonPressTime = millis();
        longPressState = false;
    }
    //long press check
    if (btn == LOW && !longPressState && (millis() - buttonPressTime) >= 2000) {
        resetAuto();
        longPressState = true;
    }

    // short pressed
    if (lastButton == LOW && btn == HIGH && !longPressState) {
        idleTimer = millis();

        if (autoState == AUTO_SET_TEMP) {
          setTemp++;
          userPressed = true;
          stateTimer = millis();
        }
    }

    lastButton = btn;
}

void setup() {
    Serial.begin(115200);

    pinMode(MANUAL_PIN, INPUT_PULLUP);
    pinMode(AUTO_PIN, INPUT_PULLUP);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);

    Wire.begin(SDA, SCL);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED not found");
        while (true);
    }
    display.clearDisplay();
    display.setTextColor(WHITE);

    idleTimer = millis();
    situation1StartTime = millis();
}

void loop() {
    // put your main code here, to run repeatedly:
    readTemperature();
    readModeSwitch();

    if (currentMode != lastMode) {
        resetManual();
        digitalWrite(RELAY_PIN, HIGH);
        lastMode = currentMode;
    }

    if (currentMode == MODE_MANUAL) {
        handleButtonManual();
        handleManualState();
        controlRelay();
    } else {
//        digitalWrite(RELAY_PIN, HIGH);
        handleButtonAuto();
        handleAutoState();
        controlRelay();
    }

    showOLED();
    delay(50);
}
