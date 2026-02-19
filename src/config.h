#define MQTT_BROKER "10.0.0.37"
#define MQTT_PORT 1883
#define CLIENT_ID "ESP32Client"

// TOPICS
#define TOPIC_ALL "esp_iot/#"
#define TOPIC_UPTIME "esp_iot/uptime"
#define TOPIC_LED_CONTROL "esp_iot/led"
#define TOPIC_IR_BUTTON_PRESS "esp_iot/controls/ir_remote/press"

#define TOPIC_LAMP_STATE "esp_iot/lamp/state"


// IR Remote Buttons
// Remote codes for my fan remote
// The buttons are numbered moving left to right, top to bottom
#define BUTTON_1 0xF807F807FE01 // Fan speed (cycle between 1 2 3)
#define BUTTON_2 0xFC03FC03FE01 // Power
#define BUTTON_3 0xEC13EC13FE01 // Blank
#define BUTTON_4 0xF40BF40BFE01 // Timer (cycle)
#define BUTTON_5 0xAF50AF50FE01 // Blank
#define BUTTON_6 0xF00FF00FFE01 // Blank


// Lamp Remote Buttons
// Values are in decimal (base 10)

#define BUTTON_TIMER_1H 10325249
#define BUTTON_POWER 10325250
#define BUTTON_B_UP 10325251
#define BUTTON_CT_UP 10325254
#define BUTTON_B_DOWN 10325255
#define BUTTON_CT_DOWN 10325252
#define BUTTON_WHITE 10325253
#define BUTTON_K 10325256
#define BUTTON_NIGHT 10325257
#define BUTTON_BOTTLE 10325258
#define BUTTON_COMPUTER 10325259
#define BUTTON_READING 10325260

// Adjust these based on your sniffed values
#define RF_PROTOCOL    1
#define RF_PULSE_LEN   320
#define RF_BIT_LENGTH  24
#define TX_PIN         5  // GPIO5 (D1 on NodeMCU)
