#include <LiquidCrystal.h>
#include <RotaryEncoder.h>

enum class Status {
  ACTIVE,
  OFF
};

enum class Screen {
  STATUS,
  CALIBRATE
};

RotaryEncoder encoder(33, 31);
LiquidCrystal lcd(16, 17, 23, 25, 27, 29);
const int encoderButtonPin = 35;
int lastEncoderButtonState = HIGH;
int lastEncoderButtonStateStable = HIGH;
unsigned long lastEncoderButtonSwitchTime = 0;
const int buzzer = 37;
const int lcdCols = 20;
const int lcdRows = 4;
Screen screen = Screen::STATUS;

int calibrateValue = 100;
int calibrateEncoderAdjustment = 0;

void setup() {
  pinMode(encoderButtonPin, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  noTone(buzzer);
  lcd.begin(lcdCols, lcdRows);
}

void showStatusScreen(float oxygenLevel, float flow, Status s) {
  char out[lcdCols];
  const char errorMessage[] = "err";
  const char statusActive[] = "ACTIVE";
  const char statusOff[] = "OFF";

  if (fabs(oxygenLevel) > 9999) {
    strncpy(out, errorMessage, sizeof(errorMessage) + 1);
  } else {
    dtostrf(oxygenLevel, 0, 2, out);
  }
  lcd.setCursor(0, 0);
  lcd.print("Level ");
  lcd.print(out);

  if (fabs(flow) > 9999) {
    strncpy(out, errorMessage, sizeof(errorMessage) + 1);
  } else {
    dtostrf(flow, 0, 2, out);
  }
  lcd.setCursor(0, 1);
  lcd.print("Flow ");
  lcd.print(out);

  switch (s) {
    case Status::ACTIVE:
      strncpy(out, statusActive, sizeof(statusActive) + 1);
      break;
    case Status::OFF:
      strncpy(out, statusOff, sizeof(statusOff) + 1);
      break;
  }
  lcd.setCursor(0, 2);
  lcd.print("Status ");
  lcd.print(out);
}

void showCalibrateScreen(float value) {
  char out[lcdCols];
  const char errorMessage[] = "err";
  
  lcd.setCursor(0, 0);
  lcd.print("Calibrate");

  if (fabs(value) > 9999) {
    strncpy(out, errorMessage, sizeof(errorMessage) + 1);
  } else {
    dtostrf(value, 0, 2, out);
  }
  lcd.setCursor(0, 1);
  lcd.print("Value ");
  lcd.print(out);
}

void loop() {
  encoder.tick();
  int encoderPosition = encoder.getPosition();
  switch (screen) {
    case Screen::STATUS:
      showStatusScreen(1.2, 0.2, Status::ACTIVE);
      break;
    case Screen::CALIBRATE:
      calibrateValue = calibrateEncoderAdjustment - encoderPosition;
      showCalibrateScreen(calibrateValue);
      break;
  }

  unsigned long current = millis();
  int reading = digitalRead(encoderButtonPin);
  if (reading != lastEncoderButtonState) {
    lastEncoderButtonSwitchTime = current;
    lastEncoderButtonState = reading;
  }
  if (current - lastEncoderButtonSwitchTime > 50) {
    if (reading == LOW && lastEncoderButtonStateStable == HIGH) {
      lcd.clear();
      switch (screen) {
        case Screen::STATUS:
          calibrateEncoderAdjustment = calibrateValue + encoderPosition;
          screen = Screen::CALIBRATE;
          break;
        case Screen::CALIBRATE:
          screen = Screen::STATUS;
          break;
      }
    }
    lastEncoderButtonStateStable = reading;
  }
}
