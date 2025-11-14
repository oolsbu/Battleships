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
const IPAddress OTHER_IP(172,20,10,4);

// Toggle: when enabled (1) the waiting player will see where the opponent is
// currently aiming (sent as AIM:x,y packets). Set to 0 to disable.
#define SHOW_OPPONENT_AIM 1

#endif
