// Simple Battleships placement logic
// All boat placement state and helpers live in this header so `main.cpp`
// only needs to call the small public functions below.

#pragma once

#include <FastLED.h>
#include "led_matrix.h"
#include "joystick.h"

// Configuration limits
#define MAX_BOAT_TYPES 6
#define MAX_BOATS 10

// Color palette used while placing
static const CRGB COLOR_PLACING = CRGB::Blue;
static const CRGB COLOR_PLACED = CRGB::Green;
static const CRGB COLOR_INVALID = CRGB::Red;

// Boat representation: size (length), orientation, position and placed flag
struct Boat {
    uint8_t size;    // length in tiles
    int x;           // top-left x (when horizontal) or top cell when vertical
    int y;           // top-left y
    bool vertical;   // orientation: false = horizontal, true = vertical
    bool placed;     // true when confirmed
};

// Internal state
static Boat boats[MAX_BOATS];
static uint8_t boatsCount = 0;      // number of boats in the boats[] array
static int currentIndex = 0;        // index of boat currently being placed
static bool occupied[WIDTH][HEIGHT];

// Button press tracking for short vs long press
static int prevButtonState = 0;
static unsigned long buttonPressTime = 0;
static const unsigned long LONG_PRESS_MS = 500; // >= 500ms to confirm placement

// Public: configure the boats to place
// sizes[]: array of sizes
// counts[]: how many boats for each size
// types: number of distinct sizes
// returns true on success, false if too many boats
inline bool beginPlacement(const uint8_t sizes[], const uint8_t counts[], int types) {
    boatsCount = 0;
    currentIndex = 0;

    // reset occupancy
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            occupied[x][y] = false;

    // expand sizes*counts into boats[]
    for (int t = 0; t < types; t++) {
        for (int c = 0; c < counts[t]; c++) {
            if (boatsCount >= MAX_BOATS) return false;
            boats[boatsCount].size = sizes[t];
            boats[boatsCount].x = 0;
            boats[boatsCount].y = 0;
            boats[boatsCount].vertical = false;
            boats[boatsCount].placed = false;
            boatsCount++;
        }
    }

    // initialize starting position for the first boat (centered-ish)
    if (boatsCount > 0) {
        boats[0].x = max(0, (WIDTH - boats[0].size) / 2);
        boats[0].y = HEIGHT / 2;
    }

    return true;
}

// Helper: check whether a boat (with given pos/orientation) fits in bounds
inline bool fitsInBounds(const Boat &b) {
    if (!b.vertical) {
        if (b.x < 0) return false;
        if (b.y < 0) return false;
        if (b.x + b.size > WIDTH) return false;
        if (b.y >= HEIGHT) return false;
        return true;
    } else {
        if (b.x < 0) return false;
        if (b.y < 0) return false;
        if (b.x >= WIDTH) return false;
        if (b.y + b.size > HEIGHT) return false;
        return true;
    }
}

// Helper: check collision with already placed boats
inline bool collidesWithPlaced(const Boat &b) {
    if (!fitsInBounds(b)) return true;
    if (!b.vertical) {
        for (int i = 0; i < b.size; i++) {
            if (occupied[b.x + i][b.y]) return true;
        }
    } else {
        for (int i = 0; i < b.size; i++) {
            if (occupied[b.x][b.y + i]) return true;
        }
    }
    return false;
}

// Confirm placement: mark occupied cells and advance to next boat
inline void confirmPlacement(bool finished) {
    if (currentIndex >= boatsCount) return;
    Boat &b = boats[currentIndex];
    if (collidesWithPlaced(b)) {
        // invalid placement, ignore confirm
        return;
    }

    // mark occupied
    if (!b.vertical) {
        for (int i = 0; i < b.size; i++) occupied[b.x + i][b.y] = true;
    } else {
        for (int i = 0; i < b.size; i++) occupied[b.x][b.y + i] = true;
    }

    b.placed = true;

    // advance
    currentIndex++;
    if (currentIndex < boatsCount) {
        // position next boat near center
        Boat &next = boats[currentIndex];
        next.vertical = false;
        next.x = max(0, (WIDTH - next.size) / 2);
        next.y = HEIGHT / 2;
    }
    else{
        finished = true;
    }
}

// Draw all placed boats and the currently placing boat into frame
inline void drawPlacementFrame(CRGB frame[WIDTH][HEIGHT]) {
    // Clear frame
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            frame[x][y] = CRGB::Black;

    // Draw placed boats
    for (int b = 0; b < boatsCount; b++) {
        if (!boats[b].placed) continue;
        Boat &bo = boats[b];
        if (!bo.vertical) {
            for (int i = 0; i < bo.size; i++) frame[bo.x + i][bo.y] = COLOR_PLACED;
        } else {
            for (int i = 0; i < bo.size; i++) frame[bo.x][bo.y + i] = COLOR_PLACED;
        }
    }

    // Draw current boat (if any left to place)
    if (currentIndex < boatsCount) {
        Boat &cb = boats[currentIndex];
        // choose color depending on validity
        CRGB col = collidesWithPlaced(cb) || !fitsInBounds(cb) ? COLOR_INVALID : COLOR_PLACING;
        if (!cb.vertical) {
            for (int i = 0; i < cb.size; i++) {
                int xx = cb.x + i;
                int yy = cb.y;
                if (xx >= 0 && xx < WIDTH && yy >= 0 && yy < HEIGHT) frame[xx][yy] = col;
            }
        } else {
            for (int i = 0; i < cb.size; i++) {
                int xx = cb.x;
                int yy = cb.y + i;
                if (xx >= 0 && xx < WIDTH && yy >= 0 && yy < HEIGHT) frame[xx][yy] = col;
            }
        }
    }
}

// Move the currently placing boat by dx/dy
inline void moveCurrentBoat(int dx, int dy, int button) {
    if (currentIndex >= boatsCount) return;
    if(button != 0) return;
    Boat &b = boats[currentIndex];
    b.x += dx;
    b.y += dy;
    // Clamp
    if (!b.vertical) {
        if (b.x < 0) b.x = 0;
        if (b.y < 0) b.y = 0;
        if (b.x + b.size > WIDTH) b.x = WIDTH - b.size;
        if (b.y >= HEIGHT) b.y = HEIGHT - 1;
    } else {
        if (b.x < 0) b.x = 0;
        if (b.y < 0) b.y = 0;
        if (b.x >= WIDTH) b.x = WIDTH - 1;
        if (b.y + b.size > HEIGHT) b.y = HEIGHT - b.size;
    }
}

// Toggle orientation of current boat
inline void rotateCurrentBoat() {
    if (currentIndex >= boatsCount) return;
    Boat &b = boats[currentIndex];
    b.vertical = !b.vertical;
    // After rotate, clamp position so boat remains inside
    if (!b.vertical) {
        if (b.x + b.size > WIDTH) b.x = WIDTH - b.size;
        if (b.y >= HEIGHT) b.y = HEIGHT - 1;
    } else {
        if (b.y + b.size > HEIGHT) b.y = HEIGHT - b.size;
        if (b.x >= WIDTH) b.x = WIDTH - 1;
    }
}

// Return true if all boats have been placed
inline bool allBoatsPlaced() {
    return (currentIndex >= boatsCount);
}

// Main per-loop placement handler
// dx,dy: joystick movement (-1/0/1)
// button: current button state (1 when pressed)
// frame: output frame to draw
inline void placementStep(int dx, int dy, int button, CRGB frame[WIDTH][HEIGHT], bool finished) {
    // Movement
    if (dx != 0 || dy != 0) {
        moveCurrentBoat(dx, dy, button); // joystick Y is not inverted in readJoystick (up=1)
    }

    // Button edge detection
    if (button && !prevButtonState) {
        // pressed now
        buttonPressTime = millis();
    }

    if (!button && prevButtonState) {
        // released now -> measure duration
        unsigned long dt = millis() - buttonPressTime;
        if (dt >= LONG_PRESS_MS) {
            // long press: confirm placement
            confirmPlacement(finished);
        } else {
            // short press: rotate
            rotateCurrentBoat();
        }
    }

    prevButtonState = button;

    // draw
    drawPlacementFrame(frame);
}

inline void aim(int dx, int dy, int button, CRGB frame[WIDTH][HEIGHT]){

}
