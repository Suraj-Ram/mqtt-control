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

void flickerLed()
{
    digitalWrite(LED_PIN, HIGH);
    delay(60);
    digitalWrite(LED_PIN, LOW);
    delay(80);
    digitalWrite(LED_PIN, HIGH);
    delay(60);
    digitalWrite(LED_PIN, LOW);
}

void connectWifi()
{
    Serial.print("Connecting to ");
    Serial.print(WIFI_SSID);
    Serial.print(" with password ");
    Serial.println(WIFI_PASSWORD);

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
}

void mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String message;
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }
    String topicStr(topic);

    Serial.print("Message arrived [");
    Serial.print(topic);
    Serial.print("] ");
    Serial.print(message);
    Serial.println();

    if (topicStr == TOPIC_LED_CONTROL)
    {
        if (message == "on")
        {
            Serial.println("Turning LED ON");
            digitalWrite(LED_PIN, HIGH);
        }
        else if (message == "off")
        {
            Serial.println("Turning LED OFF");
            digitalWrite(LED_PIN, LOW);
        }
        else if (message == "flicker")
        {
            Serial.println("Flickering LED");
            flickerLed();
        }
    }
}

void connectMQTT()
{
    while (!mqtt.connected())
    {
        Serial.print("Connecting to MQTT...");
        if (mqtt.connect(CLIENT_ID))
        {
            Serial.println("connected");
            if (mqtt.subscribe(TOPIC_ALL))
            {
                Serial.println("subscribed to topic");
            }
            else
            {
                Serial.println("failed to subscribe to topic");
            }
            mqtt.publish(TOPIC_UPTIME, "i am online");
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(mqtt.state());
            Serial.println(" retrying in 2s");
            delay(2000);
        }
    }
}

void initMQTT()
{
    mqtt.setServer(MQTT_BROKER, MQTT_PORT);
    mqtt.setCallback(mqttCallback);
}

void initPins()
{
    pinMode(LED_PIN, OUTPUT);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    connectWifi();
    initPins();
    initMQTT();
}

void loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        connectWifi();
    }
    if (!mqtt.connected())
    {
        connectMQTT();
    }

    mqtt.loop();

    // Publish periodically
    if (millis() - lastPublish > publishInterval)
    {
        lastPublish = millis();
        String payload = "uptime: " + String(millis() / 1000) + "s";
        mqtt.publish(TOPIC_UPTIME, payload.c_str());
        Serial.println("Published: " + payload);
    }
}
