#include <Arduino.h>

#define LED_PIN D10
#define DELAY 1000



void setup() {
  // Put your setup code here, to run once:

  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  // Put your main code here, to run repeatedly:
  digitalWrite(LED_PIN, HIGH);
  delay(DELAY);
  digitalWrite(LED_PIN, LOW);
  delay(DELAY);
}
