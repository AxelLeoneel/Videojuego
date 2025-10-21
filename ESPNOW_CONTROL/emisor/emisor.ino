#include <WiFi.h>
#include <esp_now.h>

// Dirección MAC del receptor ESP32
// 44:1D:64:F3:0E:2C
uint8_t receptorAddress[] = {0x44, 0x1D, 0x64, 0xF3, 0x0E, 0x2C};

#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error al inicializar ESP-NOW");
    return;
  }

  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, receptorAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;

  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Error al agregar peer");
    return;
  }

  Serial.println("Emisor listo");
}

void loop() {
  if (Serial2.available() > 0) {
    uint8_t data = Serial2.read();
    if (data == '\n' || data == '\r') return;
    esp_err_t result = esp_now_send(receptorAddress, &data, sizeof(data));
    if (result == ESP_OK) {
      Serial.print("Comando enviado: 0x");
      Serial.println(data, HEX);
    } else {
      Serial.println("Error al enviar comando");
    }
  }
}
