#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_system.h>
#include <IRremote.h>
#include <RCSwitch.h>
#include <ArduinoJson.h>

#include "secrets.h"
#include "config.h"
#include "debug_utils.h"

#define LED_PIN D10
#define IR_RECEIVER_PIN D2
#define RF_TX_PIN 2 // D0
#define RF_RX_PIN 5 // D3

WiFiClient espClient;
PubSubClient mqtt(espClient);
RCSwitch txSwitch = RCSwitch();
RCSwitch rxSwitch = RCSwitch();

struct LampState
{
    bool power = false;
    uint8_t brightness = 10; // Whole numbers between 0 and 10 inclusive

    // void setPowerOn()
    // {
    //     power = true;
    // }
    // void setPowerOff()
    // {
    //     power = false;
    // }

    // get json representation of the lamp state
    String toJson()
    {
        JsonDocument doc;
        doc["power"] = power ? "on" : "off";
        doc["brightness"] = brightness;
        String json;
        serializeJson(doc, json);
        return json;
    }

    // If makeChange is true, we need to make the change in real life by sending the correct remote action via RF
    void updatePowerState(bool newPowerState, PubSubClient &mqttClient, bool makeChange = false, RCSwitch &RF_TX_Client = txSwitch)
    {

        debugPrint("calling update power state", mqttClient);

        // debug print the current state and the given args
        debugPrint((String("Current power state: ") + String(power)).c_str(), mqttClient);
        debugPrint((String("New power state: ") + String(newPowerState)).c_str(), mqttClient);


        if (makeChange && (power != newPowerState))
        {
            Serial.println("Sending RF signal to change power state");
            
            power = newPowerState; 
            
            RF_TX_Client.send(BUTTON_POWER, RF_BIT_LENGTH); // Example RF code for testing
            // lastTxTime = millis();
            Serial.println("RF signal sent");
        }

        mqttClient.publish(TOPIC_LAMP_STATE, toJson().c_str());
    }

    void updateBrightness(uint8_t newBrightness, PubSubClient &mqttClient, bool makeChange = false, RCSwitch &RF_TX_Client = txSwitch)
    {
        brightness = newBrightness;
        if (makeChange)
        {
            Serial.println("Will send RF signal to change brightness");
            debugPrint("Sending RF signal to change brightness NOT IMPLEMENTED YET", mqttClient);
        }
        mqttClient.publish(TOPIC_LAMP_STATE, toJson().c_str());
    }
};



LampState lampState;

unsigned long lastPublish = 0;
const long publishInterval = 10000000; // Publish every 60 seconds

// Ignore our own transmissions
volatile bool isTransmitting = false;
unsigned long lastTxTime = 0;
#define TX_IGNORE_MS 150 // Ignore RX for 100ms after TX

// Lamp service methods. Methods that use the RF transmitter and receiver to update the lamp and its state go here
// TODO add here

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

void handleMqttLampMessages(String message)
{
    // Handles messages sent to TOPIC_LAMP_CONTROL

    Serial.println("Handling lamp control message:");

    // deserialize json message
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, message);
    if (error)
    {
        Serial.print("Failed to parse JSON: ");
        Serial.println(error.c_str());
        return;
    }

    // serial print incoming json 
    Serial.println("Received JSON:");
    serializeJsonPretty(doc, Serial);
    Serial.println();

    bool newPower;
    uint8_t newBrightness;

    // if json contains power field update power state, if it contains brightness field update brightness. If a field is not provided, keep the same state for that field
    if (doc.containsKey("power")) {
        bool newPower = doc["power"] == "on" ? true : false;
        lampState.updatePowerState(newPower, mqtt, true, txSwitch);
    }
    
    if (doc.containsKey("brightness")) {
        uint8_t newBrightness = doc["brightness"];
        lampState.updateBrightness(newBrightness, mqtt, true, txSwitch);
    }



    // bool newPowerState = doc["power"] == "on" || lampState.power;     // Use existing power state if not provided
    // uint8_t newBrightness = doc["brightness"] | lampState.brightness; // Use existing brightness if not provided
    // Serial.println("new states:");
    // Serial.print("Power: ");
    // Serial.println(newPowerState ? "ON" : "OFF");
    // Serial.print("Brightness: ");
    // Serial.println(newBrightness);

    // Update lamp state and publish new state
    // lampState.updatePowerState(newPowerState, mqtt);
    // lampState.updateBrightness(newBrightness, mqtt);
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
            // Serial.println("Turning LED ON");
            // digitalWrite(LED_PIN, HIGH);

            if (lampState.power)
            {
                Serial.println("Lamp is already ON, ignoring command");
                return;
            }

            Serial.println("Sending RF signal");
            txSwitch.send(BUTTON_POWER, RF_BIT_LENGTH); // Example RF code for testing
            lastTxTime = millis();
            Serial.println("RF signal sent");

            lampState.updatePowerState(true, mqtt);
        }
        else if (message == "off")
        {
            // Serial.println("Turning LED OFF");
            // digitalWrite(LED_PIN, LOW);

            if (!lampState.power)
            {
                Serial.println("Lamp is already OFF, ignoring command");
                return;
            }

            Serial.println("Sending RF signal");
            txSwitch.send(BUTTON_POWER, RF_BIT_LENGTH); // Example RF code for testing
            lastTxTime = millis();
            Serial.println("RF signal sent");

            lampState.updatePowerState(false, mqtt);
        }
        else if (message == "flicker")
        {
            Serial.println("Flickering LED");
            flickerLed();
        }
    }
    else if (topicStr == TOPIC_LAMP_CONTROL)
    {
        handleMqttLampMessages(message);
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

void handleRFButtonPress(unsigned long code)
// Process the received RF code and update lamp state accordingly.
// This only updates the internal state and not the actual lamp since
// we assume the physical remote will hit the lamp directly.
{
    Serial.print("Received RF code: ");
    Serial.println(code, HEX);

    switch (code)
    {
    case BUTTON_POWER:
        Serial.println("Power button pressed");
        lampState.updatePowerState(!lampState.power, mqtt);
        break;
    case BUTTON_B_UP:
        Serial.println("Brightness Up button pressed");
        if (lampState.brightness < 10)
        {
            lampState.updateBrightness(lampState.brightness + 1, mqtt);
        }
        break;
    case BUTTON_B_DOWN:
        Serial.println("Brightness Down button pressed");
        if (lampState.brightness > 0)
        {
            lampState.updateBrightness(lampState.brightness - 1, mqtt);
        }
        break;
    default:
        Serial.println("Unknown RF code received");
        break;
    }
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
    handleRFButtonPress(code);
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

void checkIRReceiver()
{
    if (IrReceiver.decode())
    {
        uint64_t code = IrReceiver.decodedIRData.decodedRawData;

        if (code != 0)
        {
            handleRemoteButton(code);
        }

        IrReceiver.resume();
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
        debugPrint("Publishing status update", mqtt);
        publishStatus();
        lastPublish = millis();
    }

    checkIRReceiver();
    checkRFReceiver();
}
