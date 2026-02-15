#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

#include "secrets.h"
#include "config.h"

#define LED_PIN D10
#define DELAY 1000

WiFiClient espClient;
PubSubClient mqtt(espClient);

unsigned long lastPublish = 0;
const long publishInterval = 5000; // Publish every 5 seconds


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

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
	Serial.print((char)payload[i]);
  }
  Serial.println();
}


void connectMQTT() {
    while (!mqtt.connected()) {
        Serial.print("Connecting to MQTT...");
        if (mqtt.connect(CLIENT_ID)) {
            Serial.println("connected");
            mqtt.subscribe(TEST_TOPIC);
            mqtt.publish(TEST_TOPIC, "i am online");
        } else {
            Serial.print("failed, rc=");
            Serial.print(mqtt.state());
            Serial.println(" retrying in 2s");
            delay(2000);
        }
    }
}


void initMQTT() {
	mqtt.setServer(MQTT_BROKER, MQTT_PORT);
	mqtt.setCallback(mqttCallback);
}

void initPins() {
  pinMode(LED_PIN, OUTPUT);
}


void setup() {
  Serial.begin(115200);
  delay(1000);

  connectWifi();
  initPins();
  initMQTT();

  mqtt.publish(TEST_TOPIC, "ESP32 booted up!");
  mqtt.subscribe(TEST_TOPIC);
}


void blink_led() {
  // Put your main code here, to run repeatedly:
  digitalWrite(LED_PIN, HIGH);
  mqtt.publish(TEST_TOPIC, "ESP32 not connected to WiFi!");
  delay(DELAY);
  digitalWrite(LED_PIN, LOW);
  delay(DELAY);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWifi();
  }
  if (!mqtt.connected()) {
	connectMQTT();
  }

      // Publish periodically
    if (millis() - lastPublish > publishInterval) {
        lastPublish = millis();
        String payload = "uptime: " + String(millis() / 1000) + "s";
        mqtt.publish(TEST_TOPIC, payload.c_str());
        Serial.println("Published: " + payload);
    }
}

