// Simple Battleships placement logic
// All boat placement state and helpers live in this header so `main.cpp`
// only needs to call the small public functions below.

#pragma once

#include <FastLED.h>
#include "led_matrix.h"
#include "joystick.h"
#include "udp_communication.h"
#include "config.h"

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
// Track hits on our board (true if opponent hit this cell)
static bool hitMap[WIDTH][HEIGHT];
// Track results of our shots: 0=unknown, 1=miss, 2=hit
static uint8_t opponentMap[WIDTH][HEIGHT];

// Button press tracking for short vs long press
static int prevButtonState = 0;
static unsigned long buttonPressTime = 0;
static const unsigned long LONG_PRESS_MS = 500; // >= 500ms to confirm placement
// Timeout for showing opponent aim (ms)
static const unsigned long OPP_AIM_TIMEOUT_MS = 1500;

// Opponent aiming position (visible when SHOW_OPPONENT_AIM enabled)
static int oppAimX = -1;
static int oppAimY = -1;
static unsigned long oppAimTime = 0;

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
    // reset hit and opponent maps
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            hitMap[x][y] = false;
            opponentMap[x][y] = 0;
        }
    }

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

// Find which boat (index) occupies a given cell, or -1
inline int boatIndexAt(int x, int y) {
    for (int i = 0; i < boatsCount; i++) {
        Boat &b = boats[i];
        if (!b.placed) continue;
        if (!b.vertical) {
            if (y == b.y && x >= b.x && x < b.x + b.size) return i;
        } else {
            if (x == b.x && y >= b.y && y < b.y + b.size) return i;
        }
    }
    return -1;
}

// Check whether a boat is fully sunk (all its cells are hit)
inline bool boatSunk(int index) {
    if (index < 0 || index >= boatsCount) return false;
    Boat &b = boats[index];
    if (!b.placed) return false;
    if (!b.vertical) {
        for (int i = 0; i < b.size; i++) if (!hitMap[b.x + i][b.y]) return false;
    } else {
        for (int i = 0; i < b.size; i++) if (!hitMap[b.x][b.y + i]) return false;
    }
    return true;
}

// Mark an opponent boat as sunk in our opponentMap by expanding from a hit cell
inline void markSunkOpponentBoat(int x, int y) {
    if (x < 0 || y < 0) return;
    // mark the central cell
    opponentMap[x][y] = 3;
    // check horizontal run of hits
    int lx = x, rx = x;
    while (lx - 1 >= 0 && opponentMap[lx - 1][y] == 2) lx--;
    while (rx + 1 < WIDTH && opponentMap[rx + 1][y] == 2) rx++;
    if (rx > lx) {
        for (int xi = lx; xi <= rx; xi++) opponentMap[xi][y] = 3;
        return;
    }
    // check vertical run
    int ty = y, by = y;
    while (ty - 1 >= 0 && opponentMap[x][ty - 1] == 2) ty--;
    while (by + 1 < HEIGHT && opponentMap[x][by + 1] == 2) by++;
    if (by > ty) {
        for (int yi = ty; yi <= by; yi++) opponentMap[x][yi] = 3;
        return;
    }
    // single cell boat - already marked
}

// Confirm placement: mark occupied cells and advance to next boat
inline void confirmPlacement(bool &finished) {
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
        Serial.println(finished);
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
inline void placementStep(int dx, int dy, int button, CRGB frame[WIDTH][HEIGHT], bool &finished) {
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

// Current aiming position and game state
static int aimX = WIDTH/2;
static int aimY = HEIGHT/2;
static bool myTurn = true;  // Start with our turn
// AIM send rate limiting
static unsigned long lastAimSendTime = 0;
static const unsigned long AIM_SEND_INTERVAL_MS = 150; // ms

// Accessors so other modules (e.g., player_logic) can read/modify turn state
inline bool getMyTurn() { return myTurn; }
inline void setMyTurn(bool v) { myTurn = v; }

inline void aim(int dx, int dy, int button, CRGB frame[WIDTH][HEIGHT]){
    // Clear frame
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            frame[x][y] = CRGB::Black;

    // Process a single incoming message (non-blocking)
    String msg = receiveMessage();
    if (msg.length() > 0) {
        // AIM messages: opponent sharing their current aim
        if (msg.startsWith("AIM:")) {
            int comma = msg.indexOf(',');
            if (comma > 4) {
                int x = msg.substring(4, comma).toInt();
                int y = msg.substring(comma + 1).toInt();
                if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT) {
                    oppAimX = x; oppAimY = y; oppAimTime = millis();
                }
            }
        }
        // SHOT messages: opponent fired at us
        else if (msg.startsWith("SHOT:")) {
            int comma = msg.indexOf(',');
            if (comma > 5) {
                int sx = msg.substring(5, comma).toInt();
                int sy = msg.substring(comma + 1).toInt();
                if (sx >= 0 && sx < WIDTH && sy >= 0 && sy < HEIGHT) {
                    bool wasHit = false;
                    if (occupied[sx][sy]) {
                        // mark as hit
                        hitMap[sx][sy] = true;
                        wasHit = true;
                    }

                    // Determine sink
                    int bidx = boatIndexAt(sx, sy);
                    bool sunk = (bidx >= 0) && boatSunk(bidx);

                    // Reply with result
                    char reply[32];
                    if (wasHit) {
                        if (sunk) snprintf(reply, sizeof(reply), "RESULT:SINK");
                        else snprintf(reply, sizeof(reply), "RESULT:HIT");
                    } else {
                        snprintf(reply, sizeof(reply), "RESULT:MISS");
                    }
                    sendMessage(reply);
                }
            }
        }
        // RESULT messages: reply to our shot
        else if (msg.startsWith("RESULT:")) {
            String tok = msg.substring(7);
            if (tok.startsWith("HIT")) {
                // record last aimed cell as hit
                if (aimX >= 0 && aimY >= 0) opponentMap[aimX][aimY] = 2;
                myTurn = true; // our shot resolved, back to our turn
            } else if (tok.startsWith("MISS")) {
                if (aimX >= 0 && aimY >= 0) opponentMap[aimX][aimY] = 1;
                myTurn = true;
            } else if (tok.startsWith("SINK")) {
                if (aimX >= 0 && aimY >= 0) {
                    opponentMap[aimX][aimY] = 2; // mark as hit first
                    // convert contiguous hit run to sunk markers
                    markSunkOpponentBoat(aimX, aimY);
                }
                myTurn = true;
            }
        }
    }

    if (myTurn) {
        // Move aim position with joystick if it's our turn
        bool moved = false;
        if (dx != 0 || dy != 0) {
            aimX += dx;
            aimY += dy;
            moved = true;
            // Clamp to valid coordinates
            if (aimX < 0) aimX = 0;
            if (aimY < 0) aimY = 0;
            if (aimX >= WIDTH) aimX = WIDTH - 1;
            if (aimY >= HEIGHT) aimY = HEIGHT - 1;
        }

        // If configured, share our aim with opponent when moving
#if SHOW_OPPONENT_AIM
        if (moved && (millis() - lastAimSendTime) >= AIM_SEND_INTERVAL_MS) {
            char am[32];
            snprintf(am, sizeof(am), "AIM:%d,%d", aimX, aimY);
            sendMessage(am);
            lastAimSendTime = millis();
        }
#endif

        // Draw previous shot results (miss/hit/sunk)
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if (opponentMap[x][y] == 1) frame[x][y] = CRGB::Blue; // miss
                else if (opponentMap[x][y] == 2) frame[x][y] = CRGB::Red; // hit
                else if (opponentMap[x][y] == 3) frame[x][y] = CRGB::Purple; // sunk
            }
        }

        // Draw aiming crosshair
        frame[aimX][aimY] = CRGB::Yellow;

        // When button is pressed (edge detection handled in joystick code's caller), shoot at current position
        if (button == 1) {
            // Send shot coordinates
            char message[32];
            snprintf(message, sizeof(message), "SHOT:%d,%d", aimX, aimY);
            Serial.println(message);
            sendMessage(message);

            // Set turn to false and wait for response
            myTurn = false;
        }
    }
    else {
        // If it's not our turn, show waiting indicator and optionally opponent aim
        frame[0][0] = CRGB::Blue;

#if SHOW_OPPONENT_AIM
        if (oppAimX >= 0 && oppAimY >= 0 && (millis() - oppAimTime) < OPP_AIM_TIMEOUT_MS) {
            frame[oppAimX][oppAimY] = CRGB::Yellow;
        }
#endif
        // Also draw hits that opponent made on our board (highlight sunk boats)
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if (hitMap[x][y]) {
                    int bidx = boatIndexAt(x, y);
                    if (bidx >= 0 && boatSunk(bidx)) frame[x][y] = CRGB::Purple;
                    else frame[x][y] = CRGB::Red;
                }
            }
        }
    }
}
