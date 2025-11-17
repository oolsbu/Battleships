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

// Toggle: when enabled (1), the first shooter is chosen randomly after both players
// are ready. When disabled (0), the player who finished placement first shoots first.
#define RANDOM_FIRST_SHOOTER 0

// Board size: playable area (must be between 8 and 14)
// The board will be centered on the 16x16 LED grid with a border
#define BOARD_SIZE 10

// Validate board size at compile time
#if BOARD_SIZE < 8 || BOARD_SIZE > 14
#error "BOARD_SIZE must be between 8 and 14"
#endif

// Calculate centered board offsets
#define BOARD_OFFSET_X ((16 - BOARD_SIZE) / 2)
#define BOARD_OFFSET_Y ((16 - BOARD_SIZE) / 2)

#endif
