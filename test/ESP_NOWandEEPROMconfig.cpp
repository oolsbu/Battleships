#include <ESP8266WiFi.h>
#include <espnow.h>
#include <EEPROM.h>

// MAC of the OTHER ESP8266
uint8_t peer[] = {0xC8, 0x2B, 0x96, 0x22, 0xCB, 0x00};

String msg = "ping";

unsigned int messages = 0;
unsigned int addr = 0;

void saveMessages() {
  EEPROM.put(addr, messages);
  EEPROM.commit();
}

void onRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  String incoming = "";
  for (int i = 0; i < len; i++) incoming += (char)data[i];

  Serial.println("Got: " + incoming);

  msg = (incoming == "ping") ? "pong" : "ping";

  // load once (EEPROM is never "empty", so just trust value)
  EEPROM.get(addr, messages);

  // simple safety check (prevents garbage values)
  if (messages == 0xFFFFFFFF || messages > 1000000) {
    messages = 0;
  }

  messages++;

  saveMessages();

  Serial.print("Message nr: ");
  Serial.println(messages);

  delay(500);
  esp_now_send(peer, (uint8_t*)msg.c_str(), msg.length());

  Serial.println("Sent: " + msg);
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  EEPROM.begin(512); // must be >= 4 bytes for int

  esp_now_init();
  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_add_peer(peer, ESP_NOW_ROLE_COMBO, 1, NULL, 0);
  esp_now_register_recv_cb(onRecv);

  EEPROM.get(addr, messages);
  if (messages == 0xFFFFFFFF) messages = 0;

  esp_now_send(peer, (uint8_t*)msg.c_str(), msg.length());
  Serial.println("Sent: ping");
}

void loop() {}
