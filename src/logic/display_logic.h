#pragma once
#include "board_logic.h"

// ===== Helper: Draw Border =====

inline void drawBorder(CRGB frame[16][16]) {
    // Draw a 1-pixel wide border around the play area (leave interior/background untouched)
    int left = BOARD_OFFSET_X - 1;
    int right = BOARD_OFFSET_X + BOARD_SIZE;
    int top = BOARD_OFFSET_Y - 1;
    int bottom = BOARD_OFFSET_Y + BOARD_SIZE;

    // Top and bottom horizontal lines
    for (int x = left; x <= right; x++) {
        frame[x][top] = COLOR_BORDER;
        frame[x][bottom] = COLOR_BORDER;
    }

    // Left and right vertical lines
    for (int y = top; y <= bottom; y++) {
        frame[left][y] = COLOR_BORDER;
        frame[right][y] = COLOR_BORDER;
    }
}

// ===== Placement Display =====

inline void drawPlacementFrame(CRGB frame[16][16]) {
    // Clear and draw border
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            frame[x][y] = CRGB::Black;
    
    drawBorder(frame);
    
    // Draw placed boats
    for (int i = 0; i < boatsCount; i++) {
        if (!boats[i].placed) continue;
        Boat &b = boats[i];
        if (!b.vertical) {
            for (int cell = 0; cell < b.size; cell++) {
                int px = BOARD_OFFSET_X + b.x + cell;
                int py = BOARD_OFFSET_Y + b.y;
                frame[px][py] = COLOR_PLACED;
            }
        } else {
            for (int cell = 0; cell < b.size; cell++) {
                int px = BOARD_OFFSET_X + b.x;
                int py = BOARD_OFFSET_Y + b.y + cell;
                frame[px][py] = COLOR_PLACED;
            }
        }
    }
    
    // Draw current boat being placed
    if (currentIndex < boatsCount) {
        Boat &cb = boats[currentIndex];
        CRGB color = (boatCollidesWithPlaced(cb) || !boatFitsInBounds(cb)) ? COLOR_INVALID : COLOR_PLACING;
        if (!cb.vertical) {
            for (int cell = 0; cell < cb.size; cell++) {
                int px = BOARD_OFFSET_X + cb.x + cell;
                int py = BOARD_OFFSET_Y + cb.y;
                if (px >= 0 && px < 16 && py >= 0 && py < 16) frame[px][py] = color;
            }
        } else {
            for (int cell = 0; cell < cb.size; cell++) {
                int px = BOARD_OFFSET_X + cb.x;
                int py = BOARD_OFFSET_Y + cb.y + cell;
                if (px >= 0 && px < 16 && py >= 0 && py < 16) frame[px][py] = color;
            }
        }
    }
}

// ===== Game Phase Displays =====

inline void drawMyTurnFrame(CRGB frame[16][16]) {
    // Show opponent's board with aiming cursor
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            frame[x][y] = CRGB::Black;
    
    drawBorder(frame);
    
    // Draw opponent map (misses and hits)
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int px = BOARD_OFFSET_X + x;
            int py = BOARD_OFFSET_Y + y;
            if (opponentMap[x][y] == 1) frame[px][py] = COLOR_MISS;
            else if (opponentMap[x][y] == 2) frame[px][py] = COLOR_HIT;
            else if (opponentMap[x][y] == 3) frame[px][py] = COLOR_SUNK;
        }
    }
    
    // Draw aiming cursor
    frame[BOARD_OFFSET_X + aimX][BOARD_OFFSET_Y + aimY] = COLOR_AIM;
}

inline void drawOpponentShotFrame(CRGB frame[16][16]) {
    // Show your board with hits
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            frame[x][y] = CRGB::Black;
    
    drawBorder(frame);
    
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int px = BOARD_OFFSET_X + x;
            int py = BOARD_OFFSET_Y + y;
            if (hitMap[x][y]) {
                int boatIdx = boatIndexAt(x, y);
                frame[px][py] = (boatIdx >= 0 && boatSunk(boatIdx)) ? COLOR_SUNK : COLOR_HIT;
            }
        }
    }
}

inline void drawShowResultFrame(CRGB frame[16][16]) {
    // Show opponent's board with result highlighted
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            frame[x][y] = CRGB::Black;
    
    drawBorder(frame);
    
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            int px = BOARD_OFFSET_X + x;
            int py = BOARD_OFFSET_Y + y;
            if (opponentMap[x][y] == 1) frame[px][py] = COLOR_MISS;
            else if (opponentMap[x][y] == 2) frame[px][py] = COLOR_HIT;
            else if (opponentMap[x][y] == 3) frame[px][py] = COLOR_SUNK;
        }
    }
}

inline void drawWaitForOpponentFrame(CRGB frame[16][16]) {
    // Show your board with all boats visible
    for (int y = 0; y < 16; y++)
        for (int x = 0; x < 16; x++)
            frame[x][y] = CRGB::Black;
    
    drawBorder(frame);
    
    // Draw all placed boats
    for (int i = 0; i < boatsCount; i++) {
        if (!boats[i].placed) continue;
        Boat &b = boats[i];
        if (!b.vertical) {
            for (int cell = 0; cell < b.size; cell++) {
                int px = BOARD_OFFSET_X + b.x + cell;
                int py = BOARD_OFFSET_Y + b.y;
                // Show red if hit, green if unhit
                if (hitMap[b.x + cell][b.y]) {
                    int boatIdx = boatIndexAt(b.x + cell, b.y);
                    frame[px][py] = (boatIdx >= 0 && boatSunk(boatIdx)) ? COLOR_SUNK : COLOR_HIT;
                } else {
                    frame[px][py] = COLOR_PLACED;
                }
            }
        } else {
            for (int cell = 0; cell < b.size; cell++) {
                int px = BOARD_OFFSET_X + b.x;
                int py = BOARD_OFFSET_Y + b.y + cell;
                if (hitMap[b.x][b.y + cell]) {
                    int boatIdx = boatIndexAt(b.x, b.y + cell);
                    frame[px][py] = (boatIdx >= 0 && boatSunk(boatIdx)) ? COLOR_SUNK : COLOR_HIT;
                } else {
                    frame[px][py] = COLOR_PLACED;
                }
            }
        }
    }
    
    // Show opponent's aiming cursor if enabled
#if SHOW_OPPONENT_AIM
    if (oppAimX >= 0 && oppAimY >= 0 && (millis() - oppAimTime) < OPP_AIM_TIMEOUT_MS) {
        frame[BOARD_OFFSET_X + oppAimX][BOARD_OFFSET_Y + oppAimY] = COLOR_AIM;
    }
#endif
    
    // Indicator in corner
    frame[0][0] = COLOR_WAITING;
}
