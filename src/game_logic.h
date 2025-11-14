#pragma once
#include <FastLED.h>
#include "led_matrix.h"
#include "joystick.h"
#include "udp_communication.h"
#include "config.h"

#define MAX_BOAT_TYPES 6
#define MAX_BOATS 10

static const CRGB COLOR_PLACING = CRGB::Blue;
static const CRGB COLOR_PLACED = CRGB::Green;
static const CRGB COLOR_INVALID = CRGB::Red;
static const CRGB COLOR_MISS = CRGB::Blue;
static const CRGB COLOR_HIT = CRGB::Red;
static const CRGB COLOR_SUNK = CRGB::Purple;
static const CRGB COLOR_AIM = CRGB::Yellow;
static const CRGB COLOR_WAITING = CRGB::Blue;

static const unsigned long LONG_PRESS_MS = 500;
static const unsigned long AIM_SEND_INTERVAL_MS = 150;
static const unsigned long OPP_AIM_TIMEOUT_MS = 1500;

static const unsigned long READY_HANDSHAKE_TIMEOUT_MS = 10000; // 10 second timeout for ready handshake
static const unsigned long PHASE_DISPLAY_TIME_MS = 1000; // 1 second to view opponent's shot

struct Boat {
    uint8_t size;
    int x, y;
    bool vertical;
    bool placed;
};

static Boat boats[MAX_BOATS];
static uint8_t boatsCount = 0;
static int currentIndex = 0;
static bool occupied[WIDTH][HEIGHT];
static int prevButtonState = 0;
static unsigned long buttonPressTime = 0;

static bool hitMap[WIDTH][HEIGHT];
static uint8_t opponentMap[WIDTH][HEIGHT];
static int aimX = WIDTH / 2;
static int aimY = HEIGHT / 2;
static bool myTurn = true;
static unsigned long lastAimSendTime = 0;
static int oppAimX = -1, oppAimY = -1;
static unsigned long oppAimTime = 0;

// Turn flow state machine
enum GamePhase { PHASE_MY_TURN, PHASE_OPPONENT_SHOT, PHASE_SHOW_RESULT, PHASE_WAIT_FOR_OPPONENT };
static GamePhase gamePhase = PHASE_MY_TURN;
static unsigned long phaseStartTime = 0;
static const unsigned long RESULT_DISPLAY_TIME_MS = 1000; // 1 second to view opponent's shot

// Ready handshake state
enum ReadyState { READY_PLACEMENT, READY_WAITING_FOR_OPPONENT, READY_SYNCED };
static ReadyState readyState = READY_PLACEMENT;
static unsigned long placementFinishedTime = 0;
static bool opponentReady = false;
static unsigned long readyStateStartTime = 0;
static unsigned long opponentPlacementTime = 0;
static bool opponentPlacementTimeReceived = false;

inline bool beginPlacement(const uint8_t sizes[], const uint8_t counts[], int types) {
    boatsCount = 0; currentIndex = 0;
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            occupied[x][y] = false;
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            hitMap[x][y] = false, opponentMap[x][y] = 0;
    for (int t = 0; t < types; t++)
        for (int c = 0; c < counts[t]; c++) {
            if (boatsCount >= MAX_BOATS) return false;
            boats[boatsCount].size = sizes[t];
            boats[boatsCount].x = 0;
            boats[boatsCount].y = 0;
            boats[boatsCount].vertical = false;
            boats[boatsCount].placed = false;
            boatsCount++;
        }
    if (boatsCount > 0) {
        boats[0].x = max(0, (WIDTH - boats[0].size) / 2);
        boats[0].y = HEIGHT / 2;
    }
    return true;
}

inline bool fitsInBounds(const Boat &b) {
    if (!b.vertical)
        return (b.x >= 0 && b.y >= 0 && b.x + b.size <= WIDTH && b.y < HEIGHT);
    else
        return (b.x >= 0 && b.y >= 0 && b.x < WIDTH && b.y + b.size <= HEIGHT);
}

inline bool collidesWithPlaced(const Boat &b) {
    if (!fitsInBounds(b)) return true;
    if (!b.vertical) {
        for (int i = 0; i < b.size; i++)
            if (occupied[b.x + i][b.y]) return true;
    } else {
        for (int i = 0; i < b.size; i++)
            if (occupied[b.x][b.y + i]) return true;
    }
    return false;
}

inline void moveCurrentBoat(int dx, int dy, int button) {
    if (currentIndex >= boatsCount || button != 0) return;
    Boat &b = boats[currentIndex];
    b.x += dx; b.y += dy;
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

inline void rotateCurrentBoat() {
    if (currentIndex >= boatsCount) return;
    Boat &b = boats[currentIndex];
    b.vertical = !b.vertical;
    if (!b.vertical) {
        if (b.x + b.size > WIDTH) b.x = WIDTH - b.size;
        if (b.y >= HEIGHT) b.y = HEIGHT - 1;
    } else {
        if (b.y + b.size > HEIGHT) b.y = HEIGHT - b.size;
        if (b.x >= WIDTH) b.x = WIDTH - 1;
    }
}

inline void confirmPlacement(bool &finished) {
    if (currentIndex >= boatsCount) return;
    Boat &b = boats[currentIndex];
    if (collidesWithPlaced(b)) return;
    if (!b.vertical) {
        for (int i = 0; i < b.size; i++) occupied[b.x + i][b.y] = true;
    } else {
        for (int i = 0; i < b.size; i++) occupied[b.x][b.y + i] = true;
    }
    b.placed = true;
    currentIndex++;
    if (currentIndex < boatsCount) {
        Boat &next = boats[currentIndex];
        next.vertical = false;
        next.x = max(0, (WIDTH - next.size) / 2);
        next.y = HEIGHT / 2;
    } else {
        finished = true;
        Serial.println("Placement complete!");
    }
}

inline void drawPlacementFrame(CRGB frame[WIDTH][HEIGHT]) {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            frame[x][y] = CRGB::Black;
    for (int i = 0; i < boatsCount; i++) {
        if (!boats[i].placed) continue;
        Boat &b = boats[i];
        if (!b.vertical) {
            for (int cell = 0; cell < b.size; cell++) frame[b.x + cell][b.y] = COLOR_PLACED;
        } else {
            for (int cell = 0; cell < b.size; cell++) frame[b.x][b.y + cell] = COLOR_PLACED;
        }
    }
    if (currentIndex < boatsCount) {
        Boat &cb = boats[currentIndex];
        CRGB color = (collidesWithPlaced(cb) || !fitsInBounds(cb)) ? COLOR_INVALID : COLOR_PLACING;
        if (!cb.vertical) {
            for (int cell = 0; cell < cb.size; cell++) {
                int px = cb.x + cell, py = cb.y;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) frame[px][py] = color;
            }
        } else {
            for (int cell = 0; cell < cb.size; cell++) {
                int px = cb.x, py = cb.y + cell;
                if (px >= 0 && px < WIDTH && py >= 0 && py < HEIGHT) frame[px][py] = color;
            }
        }
    }
}

inline void placementStep(int dx, int dy, int button, CRGB frame[WIDTH][HEIGHT], bool &finished) {
    if (dx != 0 || dy != 0) moveCurrentBoat(dx, dy, button);
    if (button && !prevButtonState) buttonPressTime = millis();
    if (!button && prevButtonState) {
        unsigned long pressDuration = millis() - buttonPressTime;
        if (pressDuration >= LONG_PRESS_MS) confirmPlacement(finished);
        else rotateCurrentBoat();
    }
    prevButtonState = button;
    drawPlacementFrame(frame);
}

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

inline bool boatSunk(int index) {
    if (index < 0 || index >= boatsCount) return false;
    Boat &b = boats[index];
    if (!b.placed) return false;
    if (!b.vertical) {
        for (int i = 0; i < b.size; i++)
            if (!hitMap[b.x + i][b.y]) return false;
    } else {
        for (int i = 0; i < b.size; i++)
            if (!hitMap[b.x][b.y + i]) return false;
    }
    return true;
}

inline void markSunkOpponentBoat(int x, int y) {
    if (x < 0 || y < 0) return;
    opponentMap[x][y] = 3;
    int lx = x, rx = x;
    while (lx - 1 >= 0 && opponentMap[lx - 1][y] == 2) lx--;
    while (rx + 1 < WIDTH && opponentMap[rx + 1][y] == 2) rx++;
    if (rx > lx) {
        for (int xi = lx; xi <= rx; xi++) opponentMap[xi][y] = 3;
        return;
    }
    int ty = y, by = y;
    while (ty - 1 >= 0 && opponentMap[x][ty - 1] == 2) ty--;
    while (by + 1 < HEIGHT && opponentMap[x][by + 1] == 2) by++;
    if (by > ty) {
        for (int yi = ty; yi <= by; yi++) opponentMap[x][yi] = 3;
    }
}

inline bool getMyTurn() { return myTurn; }
inline void setMyTurn(bool v) { myTurn = v; }

inline void handleReadyHandshake() {
    // Handle the ready state machine for synchronizing game start
    String msg = receiveMessage();
    unsigned long now = millis();
    if (msg.length() > 0) {
        if (msg.startsWith("READY:")) {
            // Expect format: READY:<placementMillis>
            unsigned long otherTime = (unsigned long) msg.substring(6).toInt();
            Serial.print("[READY] Received opponent READY timestamp: ");
            Serial.println(otherTime);
            opponentReady = true;
            opponentPlacementTime = otherTime;
            opponentPlacementTimeReceived = true;
            readyStateStartTime = now;
        } else if (msg.startsWith("READY")) {
            // Backwards-compatible: treat plain READY as timestamp 0
            Serial.println("[READY] Received opponent READY (no timestamp)");
            opponentReady = true;
            opponentPlacementTime = 0;
            opponentPlacementTimeReceived = true;
            readyStateStartTime = now;
        }
    }

    if (readyState == READY_PLACEMENT) {
        // Still placing boats locally; nothing to do until placement finishes
        return;
    } else if (readyState == READY_WAITING_FOR_OPPONENT) {
        // We're done placing, waiting for opponent
        if (opponentPlacementTimeReceived) {
            // Both sides have placement timestamps; compare to determine who finished first
            Serial.println("[READY] Both timestamps available - deciding who shoots first...");
            const unsigned long MAX_UL = 0xFFFFFFFFUL;
            unsigned long myTime = placementFinishedTime ? placementFinishedTime : MAX_UL;
            unsigned long theirTime = opponentPlacementTime ? opponentPlacementTime : MAX_UL;
            Serial.print("[READY] myTime="); Serial.print(myTime);
            Serial.print(" theirTime="); Serial.println(theirTime);
            if (myTime <= theirTime) {
                // We finished earlier (or tie) -> we shoot first
                gamePhase = PHASE_MY_TURN;
                Serial.println("[READY] >>> YOU FINISHED FIRST - YOU SHOOT FIRST! <<<");
            } else {
                // Opponent finished earlier -> they shoot first
                gamePhase = PHASE_WAIT_FOR_OPPONENT;
                Serial.println("[READY] >>> OPPONENT FINISHED FIRST - YOU WAIT FIRST <<<");
            }
            readyState = READY_SYNCED;
            readyStateStartTime = now;
        } else if ((now - placementFinishedTime) > READY_HANDSHAKE_TIMEOUT_MS) {
            // Timeout - assume opponent is down or very slow
            Serial.println("[READY] Opponent timeout! Starting anyway... you shoot first");
            readyState = READY_SYNCED;
            readyStateStartTime = now;
            gamePhase = PHASE_MY_TURN;
        }
    } else if (readyState == READY_SYNCED) {
        // Game has started
        if (!opponentReady && (now - readyStateStartTime) > 5000) {
            // Extra safety: if opponent becomes unready, note it
            Serial.println("[READY] WARNING: Opponent became unready during game");
        }
    }
}

inline void notifyReadyToOpponent() {
    // Called when this player finishes placement
    Serial.println("[READY] Notifying opponent that placement is complete (with timestamp)...");
    // Send placement finish timestamp so we can determine who finished first
    placementFinishedTime = millis();
    char buf[32];
    snprintf(buf, sizeof(buf), "READY:%lu", placementFinishedTime);
    Serial.print("[READY] Sending: "); Serial.println(buf);
    sendMessage(buf);
}

inline void aim(int dx, int dy, int button, CRGB frame[WIDTH][HEIGHT]) {
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            frame[x][y] = CRGB::Black;

    String msg = receiveMessage();
    if (msg.length() > 0) {
        Serial.print("[AIM] Received message: ");
        Serial.println(msg);

        if (msg.startsWith("AIM:")) {
            int comma = msg.indexOf(',');
            if (comma > 4) {
                int x = msg.substring(4, comma).toInt();
                int y = msg.substring(comma + 1).toInt();
                if (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT)
                {
                    Serial.print("[AIM] Opponent aiming at: ");
                    Serial.print(x);
                    Serial.print(",");
                    Serial.println(y);
                    oppAimX = x, oppAimY = y, oppAimTime = millis();
                }
            }
        }
        else if (msg.startsWith("SHOT:")) {
            int comma = msg.indexOf(',');
            if (comma > 5) {
                int sx = msg.substring(5, comma).toInt();
                int sy = msg.substring(comma + 1).toInt();
                Serial.print("[AIM] Opponent shot at: ");
                Serial.print(sx);
                Serial.print(",");
                Serial.println(sy);

                if (sx >= 0 && sx < WIDTH && sy >= 0 && sy < HEIGHT) {
                    bool wasHit = occupied[sx][sy];
                    Serial.print("[AIM] Shot result: ");
                    Serial.println(wasHit ? "HIT" : "MISS");

                    if (wasHit) hitMap[sx][sy] = true;
                    int boatIdx = boatIndexAt(sx, sy);
                    bool sunk = (boatIdx >= 0) && boatSunk(boatIdx);
                    char reply[32];
                    if (wasHit) snprintf(reply, sizeof(reply), "RESULT:%s", sunk ? "SINK" : "HIT");
                    else snprintf(reply, sizeof(reply), "RESULT:MISS");
                    // Transition locally first so we display the opponent's shot before the shooter receives the result
                    Serial.println("[AIM] >>> Transitioning to PHASE_OPPONENT_SHOT (local)");
                    gamePhase = PHASE_OPPONENT_SHOT;
                    phaseStartTime = millis();
                    // Now send the reply
                    Serial.print("[AIM] Sending reply: ");
                    Serial.println(reply);
                    sendMessage(reply);
                }
            }
        }
        else if (msg.startsWith("RESULT:")) {
            String result = msg.substring(7);
            Serial.print("[AIM] Received result: ");
            Serial.println(result);

            if (aimX >= 0 && aimY >= 0) {
                if (result.startsWith("HIT")) opponentMap[aimX][aimY] = 2;
                else if (result.startsWith("MISS")) opponentMap[aimX][aimY] = 1;
                else if (result.startsWith("SINK")) {
                    opponentMap[aimX][aimY] = 2;
                    markSunkOpponentBoat(aimX, aimY);
                }
            }
            // Transition to showing result
            Serial.println("[AIM] >>> Transitioning to PHASE_SHOW_RESULT");
            gamePhase = PHASE_SHOW_RESULT;
            phaseStartTime = millis();
        }
    }

    // Update phase timing
    unsigned long now = millis();
    if (gamePhase == PHASE_SHOW_RESULT && (now - phaseStartTime) >= RESULT_DISPLAY_TIME_MS) {
        // After the shooter has seen the result, it's the opponent's turn next.
        Serial.println("[PHASE] PHASE_SHOW_RESULT -> PHASE_WAIT_FOR_OPPONENT");
        gamePhase = PHASE_WAIT_FOR_OPPONENT;
    }
    if (gamePhase == PHASE_OPPONENT_SHOT && (now - phaseStartTime) >= RESULT_DISPLAY_TIME_MS) {
        // After showing the incoming shot, the receiver gets to aim/fire next.
        Serial.println("[PHASE] PHASE_OPPONENT_SHOT -> PHASE_MY_TURN");
        gamePhase = PHASE_MY_TURN;
    }

    // Draw based on current phase
    if (gamePhase == PHASE_MY_TURN) {
        // Display opponent's board with previous shots
        bool moved = false;
        if (dx != 0 || dy != 0) {
            aimX += dx; aimY += dy;
            moved = true;
            if (aimX < 0) aimX = 0;
            if (aimY < 0) aimY = 0;
            if (aimX >= WIDTH) aimX = WIDTH - 1;
            if (aimY >= HEIGHT) aimY = HEIGHT - 1;
        }
#if SHOW_OPPONENT_AIM
        if (moved && (millis() - lastAimSendTime) >= AIM_SEND_INTERVAL_MS) {
            char aimMsg[32];
            snprintf(aimMsg, sizeof(aimMsg), "AIM:%d,%d", aimX, aimY);
            sendMessage(aimMsg);
            lastAimSendTime = millis();
        }
#endif
        // Draw opponent map
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)
                if (opponentMap[x][y] == 1) frame[x][y] = COLOR_MISS;
                else if (opponentMap[x][y] == 2) frame[x][y] = COLOR_HIT;
                else if (opponentMap[x][y] == 3) frame[x][y] = COLOR_SUNK;
        frame[aimX][aimY] = COLOR_AIM;
        
        if (button == 1) {
            char shotMsg[32];
            snprintf(shotMsg, sizeof(shotMsg), "SHOT:%d,%d", aimX, aimY);
                Serial.print("[SHOOT] FIRING at ");
                Serial.println(shotMsg);
            sendMessage(shotMsg);
                Serial.println("[SHOOT] >>> Transitioning to PHASE_WAIT_FOR_OPPONENT");
            gamePhase = PHASE_WAIT_FOR_OPPONENT;
        }
    } 
    else if (gamePhase == PHASE_OPPONENT_SHOT) {
        // Display your board showing where opponent shot
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)
                if (hitMap[x][y]) {
                    int boatIdx = boatIndexAt(x, y);
                    frame[x][y] = (boatIdx >= 0 && boatSunk(boatIdx)) ? COLOR_SUNK : COLOR_HIT;
                }
    }
    else if (gamePhase == PHASE_SHOW_RESULT) {
        // Display opponent's board showing the result of their shot
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)
                if (opponentMap[x][y] == 1) frame[x][y] = COLOR_MISS;
                else if (opponentMap[x][y] == 2) frame[x][y] = COLOR_HIT;
                else if (opponentMap[x][y] == 3) frame[x][y] = COLOR_SUNK;
    }
    else if (gamePhase == PHASE_WAIT_FOR_OPPONENT) {
        // Display your board while waiting for opponent
        frame[0][0] = COLOR_WAITING;
#if SHOW_OPPONENT_AIM
        if (oppAimX >= 0 && oppAimY >= 0 && (millis() - oppAimTime) < OPP_AIM_TIMEOUT_MS)
            frame[oppAimX][oppAimY] = COLOR_AIM;
#endif
        for (int y = 0; y < HEIGHT; y++)
            for (int x = 0; x < WIDTH; x++)
                if (hitMap[x][y]) {
                    int boatIdx = boatIndexAt(x, y);
                    frame[x][y] = (boatIdx >= 0 && boatSunk(boatIdx)) ? COLOR_SUNK : COLOR_HIT;
                }
    }
}
