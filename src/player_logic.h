// Small wrapper to access turn state from game logic
#include "game_logic.h"

inline bool isMyTurn() {
    return getMyTurn();
}