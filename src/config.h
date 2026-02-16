#define MQTT_BROKER "10.0.0.37"
#define MQTT_PORT 1883
#define CLIENT_ID "ESP32Client"

// TOPICS
#define TOPIC_ALL "esp_iot/#"
#define TOPIC_UPTIME "esp_iot/uptime"
#define TOPIC_LED_CONTROL "esp_iot/led"
#define TOPIC_IR_BUTTON_PRESS "esp_iot/controls/ir_remote/press"


// IR Remote Buttons
// Remote codes for my fan remote
// The buttons are numbered moving left to right, top to bottom
#define BUTTON_1 0xF807F807FE01 // Fan speed (cycle between 1 2 3)
#define BUTTON_2 0xFC03FC03FE01 // Power
#define BUTTON_3 0xEC13EC13FE01 // Blank
#define BUTTON_4 0xF40BF40BFE01 // Timer (cycle)
#define BUTTON_5 0xAF50AF50FE01 // Blank
#define BUTTON_6 0xF00FF00FFE01 // Blank
