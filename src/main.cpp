#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_system.h>
#include <IRremote.h>
#include <RCSwitch.h>

#include "secrets.h"
#include "config.h"

#define LED_PIN D10
#define IR_RECEIVER_PIN D2
#define RF_TX_PIN 2 // D0
#define RF_RX_PIN 5 // D3

WiFiClient espClient;
PubSubClient mqtt(espClient);
RCSwitch txSwitch = RCSwitch();
RCSwitch rxSwitch = RCSwitch();

unsigned long lastPublish = 0;
const long publishInterval = 60000; // Publish every 60 seconds

// Ignore our own transmissions
volatile bool isTransmitting = false;
unsigned long lastTxTime = 0;
#define TX_IGNORE_MS 150 // Ignore RX for 100ms after TX

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

            Serial.println("Sending RF signal");
            txSwitch.send(BUTTON_POWER, RF_BIT_LENGTH); // Example RF code for testing
            Serial.println("RF signal sent");
            lastTxTime = millis();
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

void initIRReceiver()
{
    IrReceiver.begin(IR_RECEIVER_PIN, DISABLE_LED_FEEDBACK);
    Serial.println("IR Receiver initialized");
}

void initRFTransmitter()
{
    txSwitch.enableTransmit(RF_TX_PIN);
    txSwitch.setPulseLength(RF_PULSE_LEN);
    txSwitch.setProtocol(RF_PROTOCOL);
    txSwitch.setRepeatTransmit(20);
}

void initRFReceiver()
{
    rxSwitch.enableReceive(RF_RX_PIN);
}

void checkRFReceiver()
{
    if (!rxSwitch.available())
        return;

    unsigned long code = rxSwitch.getReceivedValue();
    rxSwitch.resetAvailable();

    // Ignore if we just transmitted (would pick up our own signal)
    if (millis() - lastTxTime < TX_IGNORE_MS)
        return;

    // Ignore invalid codes
    if (code == 0)
        return;

    // Process the received code
    Serial.print("Received RF code: ");
    Serial.println(code, HEX);
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    connectWifi();
    initPins();
    initMQTT();
    initIRReceiver();
    initRFTransmitter();
    initRFReceiver();
}

String getResetReason()
{
    switch (esp_reset_reason())
    {
    case ESP_RST_POWERON:
        return "power_on";
    case ESP_RST_SW:
        return "software";
    case ESP_RST_PANIC:
        return "panic";
    case ESP_RST_INT_WDT:
        return "watchdog_int";
    case ESP_RST_TASK_WDT:
        return "watchdog_task";
    case ESP_RST_WDT:
        return "watchdog";
    case ESP_RST_DEEPSLEEP:
        return "deep_sleep";
    case ESP_RST_BROWNOUT:
        return "brownout";
    default:
        return "unknown";
    }
}
void publishStatus()
{
    String json = "{";
    json += "\"uptime\":" + String(millis() / 1000) + ",";
    json += "\"heap_free\":" + String(ESP.getFreeHeap()) + ",";
    json += "\"heap_min\":" + String(ESP.getMinFreeHeap()) + ",";
    json += "\"rssi\":" + String(WiFi.RSSI()) + ",";
    json += "\"reset_reason\":\"" + getResetReason() + "\"";
    json += "}";

    mqtt.publish(TOPIC_UPTIME, json.c_str());
}

void handleRemoteButton(uint64_t code)
{
    // publish ir button press
    mqtt.publish(TOPIC_IR_BUTTON_PRESS, String(code, HEX).c_str());

    switch (code)
    {
    case BUTTON_1:
        Serial.println("Fan speed pressed");
        // TODO: Toggle fan power
        break;

    case BUTTON_2:
        Serial.println("Power pressed");
        // TODO: Toggle fan power
        break;

    case BUTTON_3:
        Serial.println("Blank pressed");
        // TODO: Handle blank button
        break;

    case BUTTON_4:
        Serial.println("Timer pressed");
        // TODO: Cycle timer setting
        break;

    case BUTTON_5:
        Serial.println("Blank pressed");
        // TODO: Handle blank button
        break;

    case BUTTON_6:
        Serial.println("Blank pressed");
        // TODO: Handle blank button
        break;

    default:
        Serial.print("Unknown code: 0x");
        Serial.println((uint32_t)(code & 0xFFFFFFFF), HEX);
        break;
    }
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
        publishStatus();
        lastPublish = millis();
    }

    if (IrReceiver.decode())
    {
        uint64_t code = IrReceiver.decodedIRData.decodedRawData;

        if (code != 0)
        {
            handleRemoteButton(code);
        }

        IrReceiver.resume();
    }

    // Check for RF signals from physical remote
    checkRFReceiver();
}
