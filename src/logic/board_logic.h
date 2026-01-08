#pragma once
#include "game_state.h"

// ===== Boat Bounds & Collision Checking =====

inline bool boatFitsInBounds(const Boat &b) {
    // Check if boat fits within the board
    if (!b.vertical)
        return (b.x >= 0 && b.y >= 0 && b.x + b.size <= BOARD_SIZE && b.y < BOARD_SIZE);
    else
        return (b.x >= 0 && b.y >= 0 && b.x < BOARD_SIZE && b.y + b.size <= BOARD_SIZE);
}

inline bool boatCollidesWithPlaced(const Boat &b) {
    if (!boatFitsInBounds(b)) return true;
    if (!b.vertical) {
        for (int i = 0; i < b.size; i++)
            if (occupied[b.x + i][b.y]) return true;
    } else {
        for (int i = 0; i < b.size; i++)
            if (occupied[b.x][b.y + i]) return true;
    }
    return false;
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
    if (x < 0 || y < 0 || x >= BOARD_SIZE || y >= BOARD_SIZE) return;
    opponentMap[x][y] = 3;
    int lx = x, rx = x;
    while (lx - 1 >= 0 && opponentMap[lx - 1][y] == 2) lx--;
    while (rx + 1 < BOARD_SIZE && opponentMap[rx + 1][y] == 2) rx++;
    if (rx > lx) {
        for (int xi = lx; xi <= rx; xi++) opponentMap[xi][y] = 3;
        return;
    }
    int ty = y, by = y;
    while (ty - 1 >= 0 && opponentMap[x][ty - 1] == 2) ty--;
    while (by + 1 < BOARD_SIZE && opponentMap[x][by + 1] == 2) by++;
    if (by > ty) {
        for (int yi = ty; yi <= by; yi++) opponentMap[x][yi] = 3;
    }
}

// ===== Aiming =====

inline bool isAimWithinBounds(int x, int y) {
    return (x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE);
}

inline void clampAim(int &x, int &y) {
    if (x < 0) x = 0;
    if (x >= BOARD_SIZE) x = BOARD_SIZE - 1;
    if (y < 0) y = 0;
    if (y >= BOARD_SIZE) y = BOARD_SIZE - 1;
}

inline bool allOpponentBoatsSunk() {

    int totalBoatCells = 0;
    for (int i = 0; i < types; i++) {
        totalBoatCells += sizes[i] * counts[i];
    }
    
    int sunkCellCount = 0;
    for (int y = 0; y < BOARD_SIZE; y++) {
        for (int x = 0; x < BOARD_SIZE; x++) {
            if (opponentMap[x][y] == 3) {
                sunkCellCount++;
            }
        }
    }
    
    return (sunkCellCount == totalBoatCells);
}

inline bool allyBoatsAllSunk() {
    for (int i = 0; i < boatsCount; i++) {
        if (!boats[i].placed) continue;
        if (!boatSunk(i)) return false;
    }
    bool hasBoats = false;
    for (int i = 0; i < boatsCount; i++) {
        if (boats[i].placed) {
            hasBoats = true;
            break;
        }
    }
    return hasBoats;
}

inline bool shotAlreadyFired(int x, int y) {
    return (opponentMap[x][y] != 0);
}
