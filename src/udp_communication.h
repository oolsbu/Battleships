#ifndef UDP_COMMUNICATION_H
#define UDP_COMMUNICATION_H

#include <WiFiUdp.h>
#include <string.h>
#include "config.h"

extern WiFiUDP udp;

inline void startUDP(unsigned int localPort = LOCAL_PORT) {
  udp.begin(localPort);
}

inline void sendMessageTo(const IPAddress& targetIp, unsigned int targetPort, const char* message) {
  udp.beginPacket(targetIp, targetPort);
  udp.write((const uint8_t*)message, strlen(message));
  udp.endPacket();
}

inline void sendMessage(const char* message) {
  sendMessageTo(OTHER_IP, OTHER_PORT, message);
}

inline String receiveMessage() {
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incoming[256];
    int len = udp.read(incoming, sizeof(incoming) - 1);
    if (len > 0) incoming[len] = 0;
    else incoming[0] = 0;
    return String(incoming);
  }
  return String("");
}

#endif
