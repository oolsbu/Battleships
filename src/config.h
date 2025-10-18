#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <IPAddress.h>

// This board listens on port 8888
const unsigned int LOCAL_PORT = 8888;

// IP of the other device (change only this per board)
// Updated default target to 192.168.0.102 as requested.
const unsigned int OTHER_PORT = 8888;
const IPAddress OTHER_IP(192,168,0,181);

#endif
