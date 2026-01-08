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
const IPAddress OTHER_IP(172,20,10,3);

#define SHOW_OPPONENT_AIM 1

#define RANDOM_FIRST_SHOOTER 1

#define BOARD_SIZE 10

#if BOARD_SIZE < 8 || BOARD_SIZE > 14
#error "BOARD_SIZE must be between 8 and 14"
#endif

#define BOARD_OFFSET_X ((16 - BOARD_SIZE) / 2)
#define BOARD_OFFSET_Y ((16 - BOARD_SIZE) / 2)

#endif
