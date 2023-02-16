#include <Arduino.h>

#define signal_pin 13

bool state = false;

void setup() {
  pinMode(signal_pin, OUTPUT);
  digitalWrite(signal_pin, HIGH);
}

void loop() {
  state = !state;
  digitalWrite(signal_pin, state);
  delay(2000);
}