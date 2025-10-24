#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <IPAddress.h>

const uint8_t sizes[] = {4, 3, 2};
const uint8_t counts[] = {1, 2, 3};
const int types = 3;

// This board listens on port 8888
const unsigned int LOCAL_PORT = 8888;

// Change this to the IP/port of the other board
const unsigned int OTHER_PORT = 8888;
const IPAddress OTHER_IP(192,168,0,181);

#endif
