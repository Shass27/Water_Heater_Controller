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

#define BUTTON_PIN 26
#define RELAY_PIN 25

enum AutoState {
  AUTO_SET_TEMP,
  AUTO_WAIT_START,
  AUTO_HEAT_USER,
  AUTO_HEAT_DEFAULT
};

AutoState autoState = AUTO_SET_TEMP;

float temperatureC = 0;
float setTemp = 0;
float lowCut = 0;
float highCut = 0;

bool userPressed = false;

unsigned long lastTempRead = 0;
unsigned long stateTimer = 0;
unsigned long idleTimer = 0;

bool lastButton = HIGH;
unsigned long buttonPressTime = 0;
bool longPressState = false;

bool blinkState = true;
unsigned long blinkTimer = 0;

void readTemperature() {
    if ((millis() - lastTempRead) >= 1000) {
        lastTempRead = millis();
        double t = thermocouple.getCelsius();
        if (!isnan(t)) temperatureC = t;
        else Serial.println("Error reading temp");
    }
}

void handleButton() {
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

void controlRelay() {
  if (isnan(temperatureC)) {
    digitalWrite(RELAY_PIN, HIGH);
    return;
  }

  if (autoState == AUTO_HEAT_USER ||
      autoState == AUTO_HEAT_DEFAULT) {

    if (temperatureC <= lowCut) {
      digitalWrite(RELAY_PIN, LOW);
    }
    else if (temperatureC >= highCut) {
      digitalWrite(RELAY_PIN, HIGH);
    }
  } else {
    digitalWrite(RELAY_PIN, HIGH);
  }
}

void resetAuto() {
  digitalWrite(RELAY_PIN, HIGH);
  autoState = AUTO_SET_TEMP;
  userPressed = false;
  setTemp = temperatureC;
  idleTimer = millis();
}

void showOLED() {
  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(32, 0);
  display.print("Water Heater");

  display.setCursor(36, 12);
  display.print("Controller");

  display.setCursor(34, 24);
  display.print("AUTO MODE");

  display.setCursor(0, 36);
  display.print("Heater:");
  display.print(digitalRead(RELAY_PIN) == LOW ? " ON " : " OFF ");
  display.print("T:");
  display.print(temperatureC, 1);
  display.print("C");

  display.setCursor(0, 48);

  if (autoState == AUTO_SET_TEMP) {
    display.print("Set Temp: ");
    if (!userPressed && blinkState) {
      display.print(setTemp, 0);
    } else if (userPressed) {
      display.print(setTemp, 0);
    } else {
      display.print("  ");
    }
    display.print("C");
  }
  else {
    display.print("Range: ");
    display.print(lowCut, 0);
    display.print("-");
    display.print(highCut, 0);
    display.print("C");
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
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
}

void loop() {
  readTemperature();
  handleButton();
  handleAutoState();
  controlRelay();
  showOLED();
  delay(50);
}