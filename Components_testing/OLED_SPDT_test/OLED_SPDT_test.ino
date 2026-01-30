#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

#define SDA 21
#define SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define MANUAL_PIN 14
#define AUTO_PIN   27
#define BUTTON_PIN 26

unsigned long remainingSeconds = 0;
unsigned long lastSecond = 0;

bool lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
bool longPressState = false;

void handleButton() {
  bool buttonState = digitalRead(BUTTON_PIN);

  //Button_press_detection
  if (lastButtonState == HIGH && buttonState == LOW) {
    buttonPressTime = millis();
    longPressState = false;
  }

  //Long_press_detection
  if (buttonState == LOW && !longPressState) {
    if (millis()-buttonPressTime>=2000) {
      remainingSeconds = 0;
      longPressState = true;
    }
  }

  //Short_press_detection
  if (lastButtonState == LOW && buttonState == HIGH) {
    if (!longPressState) {
    remainingSeconds += 60;
    lastSecond = millis();
    }
  }

  lastButtonState = buttonState;
}

void runTimer () {
  if (remainingSeconds > 0) {
    if (millis() - lastSecond >= 1000) {
      lastSecond = millis();
      remainingSeconds--;
    }
  }
}

void showManualScreen() {
  display.clearDisplay();
  display.setTextSize(1);

  int minCount = (remainingSeconds + 59) / 60;
  int mins = remainingSeconds/60;
  int secs = remainingSeconds % 60;

  display.setCursor(0, 0);
  display.print("MANUAL MODE");

  display.setCursor(0,16);
  display.print("Count: ");
  display.print(minCount);
  display.print(" min");

  display.setCursor(0, 32);
  display.print("Time: ");
  if (mins < 10) display.print("0");
  display.print(mins);
  display.print(":");
  if (secs < 10) display.print("0");
  display.print(secs);

  display.display();
}

void showAutoScreen() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);
  display.print("AUTO");
  display.display();
}

void setup() {
  Serial.begin(115200);

  pinMode(MANUAL_PIN, INPUT_PULLUP);
  pinMode(AUTO_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  Wire.begin(SDA, SCL);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED not found");
    while(true);
  }

  display.clearDisplay();
  display.setTextColor(WHITE);
}

void loop() {
  bool manualMode = (digitalRead(MANUAL_PIN) == LOW && digitalRead(AUTO_PIN) == HIGH);
  
  if (manualMode) {
    handleButton();
    runTimer();
    showManualScreen();
  }
  else {
    if (remainingSeconds != 0) remainingSeconds = 0;
    showAutoScreen();
  }

  delay(50);
}