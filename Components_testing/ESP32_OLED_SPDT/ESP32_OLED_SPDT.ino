#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define SDA 21
#define SCL 22

// Switch pins
#define MANUAL_PIN 14
#define AUTO_PIN   27

void setup() {
  pinMode(MANUAL_PIN, INPUT_PULLUP);
  pinMode(AUTO_PIN, INPUT_PULLUP);

  Wire.begin(SDA, SCL);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  bool manualState = digitalRead(MANUAL_PIN);
  bool autoState   = digitalRead(AUTO_PIN);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 20);

  if (manualState == LOW && autoState == HIGH) {
    display.println("MANUAL");
  }
  else if (autoState == LOW && manualState == HIGH) {
    display.println("AUTO");
  }
  else {
    display.println("SELECT");
  }

  display.display();
  delay(200);
}
