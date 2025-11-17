#pragma once
#include "board_logic.h"
#include "../udp_communication.h"

// ===== Placement Initialization =====

inline bool beginPlacement(const uint8_t sizes[], const uint8_t counts[], int types) {
    boatsCount = 0;
    currentIndex = 0;
    
    // Clear boards
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            occupied[x][y] = false;
    
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            hitMap[x][y] = false, opponentMap[x][y] = 0;
    
    // Create boats from config
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
    
    // Position first boat at center
    if (boatsCount > 0) {
        boats[0].x = max(0, (BOARD_SIZE - boats[0].size) / 2);
        boats[0].y = BOARD_SIZE / 2;
    }
    return true;
}

// ===== Boat Movement =====

inline void moveCurrentBoat(int dx, int dy, int button) {
    if (currentIndex >= boatsCount || button != 0) return;
    Boat &b = boats[currentIndex];
    b.x += dx;
    b.y += dy;
    
    // Clamp position to board
    if (!b.vertical) {
        if (b.x < 0) b.x = 0;
        if (b.y < 0) b.y = 0;
        if (b.x + b.size > BOARD_SIZE) b.x = BOARD_SIZE - b.size;
        if (b.y >= BOARD_SIZE) b.y = BOARD_SIZE - 1;
    } else {
        if (b.x < 0) b.x = 0;
        if (b.y < 0) b.y = 0;
        if (b.x >= BOARD_SIZE) b.x = BOARD_SIZE - 1;
        if (b.y + b.size > BOARD_SIZE) b.y = BOARD_SIZE - b.size;
    }
}

inline void rotateCurrentBoat() {
    if (currentIndex >= boatsCount) return;
    Boat &b = boats[currentIndex];
    b.vertical = !b.vertical;
    
    // Clamp after rotation
    if (!b.vertical) {
        if (b.x + b.size > BOARD_SIZE) b.x = BOARD_SIZE - b.size;
        if (b.y >= BOARD_SIZE) b.y = BOARD_SIZE - 1;
    } else {
        if (b.y + b.size > BOARD_SIZE) b.y = BOARD_SIZE - b.size;
        if (b.x >= BOARD_SIZE) b.x = BOARD_SIZE - 1;
    }
}

// ===== Placement Confirmation =====

inline void confirmPlacement(bool &finished) {
    if (currentIndex >= boatsCount) return;
    Boat &b = boats[currentIndex];
    
    // Check for collisions before confirming
    if (boatCollidesWithPlaced(b)) return;
    
    // Mark cells as occupied
    if (!b.vertical) {
        for (int i = 0; i < b.size; i++) occupied[b.x + i][b.y] = true;
    } else {
        for (int i = 0; i < b.size; i++) occupied[b.x][b.y + i] = true;
    }
    b.placed = true;
    currentIndex++;
    
    // Set up next boat or finish
    if (currentIndex < boatsCount) {
        Boat &next = boats[currentIndex];
        next.vertical = false;
        next.x = max(0, (BOARD_SIZE - next.size) / 2);
        next.y = BOARD_SIZE / 2;
    } else {
        finished = true;
        Serial.println("Placement complete!");
    }
}

// ===== Placement Step (Main Loop Hook) =====

inline void placementStep(int dx, int dy, int button, CRGB frame[16][16], bool &finished);
