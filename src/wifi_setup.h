#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <WiFiNINA.h>

// Connect to WiFi and wait up to timeoutMs for connection.
// Returns true if connected, false on timeout.
inline bool connectWiFi(const char* ssid, const char* password, unsigned long timeoutMs = 15000) {
  Serial.print("Connecting to WiFi: ");
  if(ssid == nullptr || strlen(ssid) == 0) {
    Serial.println("SSID is null or empty!");
    return false;
  }
  Serial.println(ssid);

  unsigned long start = millis();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > timeoutMs) {
      Serial.println("\nWiFi connect timeout");
      return false;
    }
    Serial.print('.');
    delay(500);
  }

  Serial.println("\nConnected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

#endif
