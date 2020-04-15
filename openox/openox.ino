#include <LiquidCrystal.h>
#include <RotaryEncoder.h>
#include "configuration.h"
#include <Wire.h>
#include <Adafruit_ADS1015.h>

enum class Status {
  ACTIVE,
  OFF
};

enum class Screen {
  STATUS,
  CALIBRATE,
  DEBUG_TIMINGS
};

/* HW Configuration */
#ifdef OPENOX_SHIELD
RotaryEncoder encoder(33, 32);
LiquidCrystal lcd(27, 26, 28, 29, 30, 31);
const int encoderButtonPin = 25;
const int buzzer = 24;
#else
RotaryEncoder encoder(33, 31);
LiquidCrystal lcd(16, 17, 23, 25, 27, 29);
const int encoderButtonPin = 35;
const int buzzer = 37;
#endif

const int lcdCols = 20;
const int lcdRows = 4;

const int valve_1 = 17;
const int valve_2 = 16;
const int valve_3 = 15;
const int valve_4 = 14;

Adafruit_ADS1115 oxygen_ADC;

int lastEncoderButtonState = HIGH;
int lastEncoderButtonStateStable = HIGH;
unsigned long lastEncoderButtonSwitchTime = 0;

Screen screen = Screen::STATUS;

int calibrateValue = 100;
int calibrateEncoderAdjustment = 0;

int adsorptionCycleDuration = 2000;
int adsorptionEncoderAdjustment = 0;

unsigned long lastRelaySwitch = millis();
int relayState = 0;

float calibrationCoeffitient = 0.00223;

void setup() {
  pinMode(encoderButtonPin, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  pinMode(valve_1, OUTPUT);
  pinMode(valve_2, OUTPUT);
  pinMode(valve_3, OUTPUT);
  pinMode(valve_4, OUTPUT);
  lcd.begin(lcdCols, lcdRows);

  oxygen_ADC.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.0078125mV
  oxygen_ADC.begin();
}

float getOxygenConcentration(float calibrationCoeff)
{
  const float adc_mv_per_bit = 0.0078125;
  int32_t adc0;
  adc0 = oxygen_ADC.readADC_SingleEnded(0);
  return calibrationCoeff * 1000 * adc0 * adc_mv_per_bit;
}

void showStatusScreen(float oxygenLevel, float flow, Status s, int relayState) {
  char out[lcdCols];
  const char errorMessage[] = "err";
  const char statusActiveValve1[] = "ACTIVE <";
  const char statusActiveValve2[] = "ACTIVE >";
  const char statusOff[] = "OFF";

  if (fabs(oxygenLevel) > 9999) {
    strncpy(out, errorMessage, sizeof(errorMessage) + 1);
  } else {
    dtostrf(oxygenLevel, 0, 1, out);
  }
  lcd.setCursor(0, 0);
  lcd.print("Oxygen Level ");
  lcd.print(out);
  lcd.print(" %");
  
  if (fabs(flow) > 9999) {
    strncpy(out, errorMessage, sizeof(errorMessage) + 1);
  } else {
    dtostrf(flow, 0, 1, out);
  }
  lcd.setCursor(0, 1);
  lcd.print("Flow ");
  lcd.print(out);
  lcd.print(" lpm");
  switch (s) {
    case Status::ACTIVE:
      if (relayState)
        strncpy(out, statusActiveValve1, sizeof(statusActiveValve1) + 1);
      else
        strncpy(out, statusActiveValve2, sizeof(statusActiveValve2) + 1);
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
    value = 9999;
  } else if (fabs(value) < 500) {
    value = 500;
  } else {
    dtostrf(value, 0, 2, out);
  }
  lcd.setCursor(0, 1);
  lcd.print("Value ");
  lcd.print(out);
}

void showDebugTimingsScreen(int value) {
  char out[lcdCols];
  const char errorMessage[] = "err";
  
  lcd.setCursor(0, 0);
  lcd.print("Timings");

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
      showStatusScreen(getOxygenConcentration(calibrationCoeffitient), 0, Status::ACTIVE, relayState);
      break;
    case Screen::CALIBRATE:
      calibrateValue = calibrateEncoderAdjustment - encoderPosition;
      showCalibrateScreen(calibrateValue);
      lastRelaySwitch = millis();
      break;
    case Screen::DEBUG_TIMINGS:
      adsorptionCycleDuration = adsorptionEncoderAdjustment - 200*encoderPosition;
      showDebugTimingsScreen(adsorptionCycleDuration);
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
          screen = Screen::DEBUG_TIMINGS;
          adsorptionEncoderAdjustment = adsorptionCycleDuration + 200*encoderPosition;
          lastRelaySwitch = millis();
          relayState = 0;
          break;
        case Screen::DEBUG_TIMINGS:
          screen = Screen::STATUS;
          break;
      }
    }
    lastEncoderButtonStateStable = reading;
  }
  /* Handle valve state */
  if (abs(millis() - lastRelaySwitch) > adsorptionCycleDuration) {
    digitalWrite(valve_1, relayState);
    relayState = !relayState;
    lastRelaySwitch = millis();
  }
}
