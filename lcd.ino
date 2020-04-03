#include <LiquidCrystal.h>
#include <RotaryEncoder.h>

unsigned long tt = 0;

RotaryEncoder encoder(33, 31); // WORKS, EXP2, clockwise goes down, counter-clockwise up
LiquidCrystal lcd(16, 17, 23, 25, 27, 29); // WORKS, EXP1
const int ledPin =  13; // WORKS, Mega
const int buttonPin = 35; // WORKS, EXP1, needs pullup, because of that default state HIGH
const int buzzer = 37; // WORKS, EXP1

int buttonState = 0;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(buzzer, OUTPUT);
  lcd.begin(20, 4);
  lcd.print("HELLO");
  noTone(buzzer);
}

void loop() {
  encoder.tick();
  unsigned long ct = millis() / 1000;
  if (ct != tt) {
    lcd.setCursor(0, 1);
    lcd.print(ct);
    tt = ct;
  }
  int newPos = encoder.getPosition();
  lcd.setCursor(0, 2);
  lcd.print(newPos);
  buttonState = digitalRead(buttonPin);
  if (buttonState == HIGH) {
    digitalWrite(ledPin, HIGH);
  } else {
    digitalWrite(ledPin, LOW);
  }
}
