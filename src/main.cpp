#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

#define LED_PIN D10
#define DELAY 1000


void connectWifi() {
  Serial.print("Connecting to ");
  Serial.print(WIFI_SSID);
  Serial.print(" with password ");
  Serial.println(WIFI_PASSWORD);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
	delay(500);
	Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void initPins() {
  pinMode(LED_PIN, OUTPUT);
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWifi();
  initPins();
}

void blink_led() {
  // Put your main code here, to run repeatedly:
  digitalWrite(LED_PIN, HIGH);
  delay(DELAY);
  digitalWrite(LED_PIN, LOW);
  delay(DELAY);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    blink_led();
  }
  else {
	digitalWrite(LED_PIN, HIGH);
  }
}

