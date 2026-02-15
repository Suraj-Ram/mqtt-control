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
