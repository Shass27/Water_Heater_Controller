#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//OLED
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//Switch
#define MANUAL_PIN 14
#define AUTO_PIN 27

//PushButton
#define BUTTON_PIN 26
#define RELAY_PIN 25

//timer
unsigned long lastSecond = 0;
unsigned long remainingSeconds = 0;

//Button logic
bool lastButtonState = HIGH;
unsigned long buttonPressTime = 0;
bool longPressHeld = false;

void setup() {
  pinMode(MANUAL_PIN, INPUT_PULLUP);
  pinMode(AUTO_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH); // Relay OFF initially

  Wire.begin();
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
}

void handleButton(){
  bool buttonState = digitalRead(BUTTON_PIN);

  if(buttonState == LOW && lastButtonState == HIGH){
    buttonPressTime = millis();
    longPressHeld = false;
  }//pressed

  if(buttonState == LOW && !longPressHeld){
    if(millis() - buttonPressTime >= 2000){
      resetAll();
      longPressHeld = true;
    }
  }//held

  if(lastButtonState == LOW && buttonState == HIGH){
    if(!longPressHeld){
      remainingSeconds += 60;
      lastSecond = millis();
    }
  }//short release

  lastButtonState = buttonState;
}

void runTimer(){
  if(remainingSeconds>0){
    if(millis()-lastSecond>1000){
      remainingSeconds--;
      lastSecond = millis();
    }
  }
}

void controlRelay(){
  if(remainingSeconds>0){
    digitalWrite(RELAY_PIN, LOW);
  } else{
    digitalWrite(RELAY_PIN, HIGH);
  }
}

void resetAll(){
  remainingSeconds = 0;
  digitalWrite(RELAY_PIN, HIGH);
}

void showManualScreen() {
  display.clearDisplay();
  display.setTextSize(1);

  int minuteCount = (remainingSeconds + 59) / 60;  // ✅ FIX
  int mins = remainingSeconds / 60;
  int secs = remainingSeconds % 60;

  display.setCursor(0, 0);
  display.print("MANUAL MODE");

  display.setCursor(0, 16);
  display.print("Count: ");
  display.print(minuteCount);
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

void loop() {
  bool manualMode = (digitalRead(MANUAL_PIN) == LOW && digitalRead(AUTO_PIN) == HIGH);

  if (manualMode) {
    handleButton();
    runTimer();
    controlRelay();
    showManualScreen();
  } else {
    resetAll();
    showAutoScreen();
  }

  delay(30);
}
