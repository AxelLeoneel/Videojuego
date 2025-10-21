#include <WiFi.h>
#include <esp_now.h>

// Pines UART2 (ajústalos si es necesario)
#define RXD2 16
#define TXD2 17

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, RXD2, TXD2);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error iniciando ESP-NOW");
    return;
  }
  esp_now_register_recv_cb([](const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len > 0) {
      uint8_t comando = data[0];
      Serial2.write(comando);
      Serial2.write('\n'); 
      Serial.print("Comando recibido y enviado por UART2: 0x");
      Serial.println(comando, HEX);
    }
  });

  Serial.println("Receptor listo");
}

void loop() {
}
