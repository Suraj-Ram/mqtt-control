

MQTT topics:
esp_iot/uptime - publishes device info every 5 seconds
esp_iot/led/set - used to set the LED state (payload: "on" or "off")
esp_iot/led/state - publishes the current LED state

esp_iot/ir_remote/pressed - publishes hex code of IR remote button press
esp_iot/ir_remote/send - used to send an IR signal (payload: hex code of the signal to send)

esp_iot






Create a secrets.h file to store WiFi credentials

```cpp
#define WIFI_SSID "your_wifi_ssid"
#define WIFI_PASSWORD "your_wifi_password"
```


Scan wifi networks

```cpp
void setup() {
  Serial.begin(115200);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
}

void blink_led() {
  // Put your main code here, to run repeatedly:
  digitalWrite(LED_PIN, HIGH);
  delay(DELAY);
  digitalWrite(LED_PIN, LOW);
  delay(DELAY);
}



void scan_wifi_blocking() {
  Serial.println("scan start");

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0) {
	Serial.println("no networks found");
  } else {
	Serial.print(n);
	Serial.println(" networks found");
	for (int i = 0; i < n; ++i) {
	  // Print SSID and RSSI for each network found
	  Serial.print(i + 1);
	  Serial.print(": ");
	  Serial.print(WiFi.SSID(i));
	  Serial.print(" (");
	  Serial.print(WiFi.RSSI(i));
	  Serial.print(")");
	  Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
	  delay(10);
	}
  }
  Serial.println("");

  // Wait a bit before scanning again
  delay(5000);
}
```


Connect to wifi
```cpp
void setup() {
  Serial.begin(115200);
  delay(10);

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
```


Sniff IR signal pulses

```cpp
#include <IRremote.h>

#define IR_PIN D2


bool ledState = false;

void setup() {

  Serial.begin(115200);
  delay(1000);  // Give serial monitor time to connect
  Serial.println("Starting...");
  // pinMode(LED_PIN, OUTPUT);
  IrReceiver.begin(IR_PIN, DISABLE_LED_FEEDBACK);
  Serial.println("IR Receiver Ready");
}

void loop() {
  if (IrReceiver.decode()) {
    // Print everything the library captured
    IrReceiver.printIRResultShort(&Serial);
    Serial.println();
    
    // Also print raw data for debugging
    IrReceiver.printIRResultRawFormatted(&Serial, true);
    Serial.println();
    
    IrReceiver.resume();
  }
}
```


Sniff RF remote signals
```cpp
#include <RCSwitch.h>

#define MIDDLE_BUTTON 10325253

RCSwitch mySwitch = RCSwitch();

void setup() {
  Serial.begin(115200);
  Serial.println("\n433 MHz RF Sniffer Started");
  Serial.println("Waiting for signals...\n");
  
  // Receiver on GPIO4 (D2 on NodeMCU/Wemos)
  mySwitch.enableReceive(4);  // Use GPIO number, not D2
}

void loop() {
  if (mySwitch.available()) {
    Serial.println("========== Signal Received ==========");
    
    // Decimal value
    Serial.print("Decimal: ");
    Serial.println(mySwitch.getReceivedValue());
    
    // Bit length
    Serial.print("Bit Length: ");
    Serial.print(mySwitch.getReceivedBitlength());
    Serial.println(" bits");
    
    // Protocol
    Serial.print("Protocol: ");
    Serial.println(mySwitch.getReceivedProtocol());
    
    // Pulse length
    Serial.print("Pulse Length: ");
    Serial.print(mySwitch.getReceivedDelay());
    Serial.println(" µs");
    
    // Binary representation
    Serial.print("Binary: ");
    Serial.println(toBinary(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength()));
    
    // Raw data pointer (for advanced use)
    unsigned int* raw = mySwitch.getReceivedRawdata();
    Serial.print("Raw Timings: ");
    for (int i = 0; i < mySwitch.getReceivedBitlength() * 2; i++) {
      Serial.print(raw[i]);
      Serial.print(" ");
    }
    Serial.println("\n");
    
    mySwitch.resetAvailable();
  }
}

// Helper function to convert to binary string
String toBinary(unsigned long value, unsigned int bitLength) {
  String binary = "";
  for (int i = bitLength - 1; i >= 0; i--) {
    binary += ((value >> i) & 1) ? "1" : "0";
  }
  return binary;
}
```

Send remote values via Serial

```cpp
#include <RCSwitch.h>

// RF codes from your remote
#define BUTTON_TIMER_1H  10325249
#define BUTTON_POWER     10325250
#define BUTTON_B_UP      10325251
#define BUTTON_CT_UP     10325254
#define BUTTON_B_DOWN    10325255
#define BUTTON_CT_DOWN   10325252
#define BUTTON_WHITE     10325253
#define BUTTON_K         10325256
#define BUTTON_NIGHT     10325257
#define BUTTON_BOTTLE    10325258
#define BUTTON_COMPUTER  10325259
#define BUTTON_READING   10325260

// Adjust these based on your sniffed values
#define RF_PROTOCOL    1
#define RF_PULSE_LEN   380
#define RF_BIT_LENGTH  24
#define TX_PIN         5  // GPIO5 (D1 on NodeMCU)

RCSwitch mySwitch = RCSwitch();


void setup() {
  Serial.begin(115200);
  
  mySwitch.enableTransmit(TX_PIN);
  mySwitch.setProtocol(RF_PROTOCOL);
  mySwitch.setPulseLength(RF_PULSE_LEN);
  
  printMenu();
}

void loop() {
  if (Serial.available()) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toLowerCase();
    
    unsigned long code = parseCommand(input);
    
    if (code > 0) {
      Serial.print("Transmitting: ");
      Serial.println(code);
      mySwitch.send(code, RF_BIT_LENGTH);
      Serial.println("Sent!\n");
    } else {
      Serial.println("Unknown command. Type 'help' for options.\n");
    }
  }
}

unsigned long parseCommand(String cmd) {
  // Named commands
  if (cmd == "power" || cmd == "p")        return BUTTON_POWER;
  if (cmd == "timer" || cmd == "t")        return BUTTON_TIMER_1H;
  if (cmd == "bup" || cmd == "b+")         return BUTTON_B_UP;
  if (cmd == "bdown" || cmd == "b-")       return BUTTON_B_DOWN;
  if (cmd == "ctup" || cmd == "ct+")       return BUTTON_CT_UP;
  if (cmd == "ctdown" || cmd == "ct-")     return BUTTON_CT_DOWN;
  if (cmd == "white" || cmd == "w")        return BUTTON_WHITE;
  if (cmd == "k")                          return BUTTON_K;
  if (cmd == "night" || cmd == "n")        return BUTTON_NIGHT;
  if (cmd == "bottle")                     return BUTTON_BOTTLE;
  if (cmd == "computer" || cmd == "c")     return BUTTON_COMPUTER;
  if (cmd == "reading" || cmd == "r")      return BUTTON_READING;
  
  if (cmd == "help" || cmd == "?") {
    printMenu();
    return 0;
  }
  
  // Allow raw numeric input
  if (cmd.length() > 0 && isDigit(cmd[0])) {
    return cmd.toInt();
  }
  
  return 0;
}

void printMenu() {
  Serial.println("\n===== 433 MHz Light Remote =====");
  Serial.println("Commands:");
  Serial.println("  power, p     - Power on/off");
  Serial.println("  timer, t     - 1 hour timer");
  Serial.println("  bup, b+      - Brightness up");
  Serial.println("  bdown, b-    - Brightness down");
  Serial.println("  ctup, ct+    - Color temp up");
  Serial.println("  ctdown, ct-  - Color temp down");
  Serial.println("  white, w     - White mode");
  Serial.println("  k            - K mode");
  Serial.println("  night, n     - Night light");
  Serial.println("  bottle       - Bottle mode");
  Serial.println("  computer, c  - Computer mode");
  Serial.println("  reading, r   - Reading mode");
  Serial.println("  <number>     - Send raw code");
  Serial.println("  help, ?      - Show this menu");
  Serial.println("================================\n");
}


// Helper function to convert to binary string
String toBinary(unsigned long value, unsigned int bitLength) {
  String binary = "";
  for (int i = bitLength - 1; i >= 0; i--) {
    binary += ((value >> i) & 1) ? "1" : "0";
  }
  return binary;
}
```
