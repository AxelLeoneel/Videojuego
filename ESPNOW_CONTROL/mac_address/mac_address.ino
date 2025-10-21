#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin();
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
 
}