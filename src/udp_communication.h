#ifndef UDP_COMMUNICATION_H
#define UDP_COMMUNICATION_H

#include <WiFiUdp.h>
#include "config.h"

// Create a UDP object in the implementation translation unit
extern WiFiUDP udp;

// Initialize UDP on the given local port.
inline void startUDP(unsigned int localPort = LOCAL_PORT) {
  udp.begin(localPort);
}

// Send a null-terminated message to a specific target IP and port.
inline void sendMessageTo(const IPAddress& targetIp, unsigned int targetPort, const char* message) {
  udp.beginPacket(targetIp, targetPort);
  udp.write((const uint8_t*)message, strlen(message));
  udp.endPacket();
}

// Convenience: send to the default OTHER_IP/OTHER_PORT from config.h
inline void sendMessage(const char* message) {
  sendMessageTo(OTHER_IP, OTHER_PORT, message);
}

// Receive a single UDP packet as a String (empty if none)
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
