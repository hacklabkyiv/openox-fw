/*
 * Author: Volodymyr Shymanskyy
 * Dependencies:
 *   Adafruit MAX6675   https://github.com/adafruit/MAX6675-library/releases
 *   SimpleTimer        https://github.com/marcelloromani/Arduino-SimpleTimer/releases
 */

#include <max6675.h>
#include <SimpleTimer.h>

// ThermoCouple pins
int thermo_vcc_pin = 53;
int thermo_sck_pin = 51;
int thermo_cs_pin  = 49;
int thermo_so_pin  = 47;

// Heater pins
int heater_pin   = 8;
int heater_pin2  = 9;


SimpleTimer timer;
MAX6675 thermocouple(thermo_sck_pin, thermo_cs_pin, thermo_so_pin);

double thermoAccum = 0.0;
int    thermoCount = 0;

void readThermo() {
  thermoAccum += thermocouple.readCelsius();
  thermoCount++;

  if (thermoCount >= 4) {
    double temp = thermoAccum / thermoCount;
    Serial.print("Temp: ");
    Serial.println(temp);

    thermoAccum = 0.0;
    thermoCount = 0;
  }
}

int knobValue = 0;

void readKnob() {
  knobValue = map(analogRead(A0), 0, 1023, 5, 80);
}

bool heaterState = true;
uint32_t heaterChange = 0;

void setHeater(bool state) {
  if (heaterState != state) {
    heaterState = state;
    heaterChange = millis();

    digitalWrite(heater_pin,  !state); // RELAY IS NC
    digitalWrite(heater_pin2, !state); // RELAY IS NC

    Serial.print("Heater: ");
    Serial.print(state ? "ON " : "OFF");

    Serial.print(", Duty cycle: ");
    Serial.print(knobValue);
    Serial.println("%");
  }
}

void driveHeater() {
  // Calculate PWM parameters (in milliseconds)

  const uint32_t period = 10000;

  uint32_t time_on = (period * knobValue) / 100;
  uint32_t time_off = period - time_on;

  /*Serial.print("ON: ");
  Serial.print(time_on);
  Serial.print(" OFF: ");
  Serial.println(time_off);*/
  
  bool newState = heaterState;
  if (heaterState && (millis() - heaterChange > time_on)) {
    setHeater(false);
  } else if (!heaterState && (millis() - heaterChange > time_off)) {
    setHeater(true);
  }
}


void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(thermo_vcc_pin, OUTPUT);
  digitalWrite(thermo_vcc_pin, HIGH);

  pinMode(heater_pin, OUTPUT);
  pinMode(heater_pin2, OUTPUT);

  setHeater(false);
  readThermo();
  readKnob();

  timer.setInterval(250, readThermo);
  timer.setInterval(1000, readKnob);

  timer.setInterval(100, driveHeater);
}


void loop() {
  timer.run();
}
