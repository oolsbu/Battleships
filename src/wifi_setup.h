#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

#include <WiFiNINA.h>
#include "animations.h"

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
    animationsUpdate();
    delay(200);
  }

  Serial.println("\nConnected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

#endif
