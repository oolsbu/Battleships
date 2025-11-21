#pragma once
#include <FastLED.h>
#include "config.h"

// ===== Game Constants =====
#define MAX_BOAT_TYPES 6
#define MAX_BOATS 10

// ===== Colors =====
static const CRGB COLOR_PLACING = CRGB::Blue;
static const CRGB COLOR_PLACED = CRGB::Green;
static const CRGB COLOR_INVALID = CRGB::Red;
static const CRGB COLOR_MISS = CRGB::Blue;
static const CRGB COLOR_HIT = CRGB::Red;
static const CRGB COLOR_SUNK = CRGB::Purple;
static const CRGB COLOR_AIM = CRGB::Yellow;
static const CRGB COLOR_WAITING = CRGB::Orange;
static const CRGB COLOR_BORDER = CRGB::White;

// ===== Timing Constants =====
static const unsigned long LONG_PRESS_MS = 500;
static const unsigned long AIM_SEND_INTERVAL_MS = 150;
static const unsigned long OPP_AIM_TIMEOUT_MS = 1500;
static const unsigned long READY_HANDSHAKE_TIMEOUT_MS = 10000;
static const unsigned long RESULT_DISPLAY_TIME_MS = 1000;

// ===== Boat Structure =====
struct Boat {
    uint8_t size;
    int x, y;           // Coordinates within the play board
    bool vertical;
    bool placed;
};

// ===== Game Phase Enum =====
enum GamePhase {
    PHASE_MY_TURN,
    PHASE_OPPONENT_SHOT,
    PHASE_SHOW_RESULT,
    PHASE_WAIT_FOR_OPPONENT,
    PHASE_GAME_WON,
    PHASE_GAME_LOST
};

// ===== Ready State Enum =====
enum ReadyState {
    READY_PLACEMENT,
    READY_WAITING_FOR_OPPONENT,
    READY_SYNCED
};

// ===== Placement State =====
static Boat boats[MAX_BOATS];
static uint8_t boatsCount = 0;
static int currentIndex = 0;
static bool occupied[BOARD_SIZE][BOARD_SIZE];
static int prevButtonState = 0;
static unsigned long buttonPressTime = 0;

// ===== Game State =====
static bool hitMap[BOARD_SIZE][BOARD_SIZE];
static uint8_t opponentMap[BOARD_SIZE][BOARD_SIZE];
static int aimX = BOARD_SIZE / 2;
static int aimY = BOARD_SIZE / 2;
static unsigned long lastAimSendTime = 0;
static int oppAimX = -1, oppAimY = -1;
static unsigned long oppAimTime = 0;

// ===== Phase Management =====
static GamePhase gamePhase = PHASE_MY_TURN;
static unsigned long phaseStartTime = 0;

// ===== Ready Handshake =====
static ReadyState readyState = READY_PLACEMENT;
static unsigned long placementFinishedTime = 0;
static bool opponentReady = false;
static unsigned long readyStateStartTime = 0;
static unsigned long opponentPlacementTime = 0;
static bool opponentPlacementTimeReceived = false;
// Temporary buffer for a received non-READY message that was consumed
// during the ready handshake. This ensures we don't lose important
// game messages (SHOT/AIM/RESULT) if they arrive while processing READY.
static String pendingMessage = "";

// ===== Win/Loss Tracking =====
static bool gameEnded = false;
static bool playerWon = false;
