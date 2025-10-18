#include <FastLED.h>
#include "led_matrix.h"

// Move a boat across the screen
// frame: 2D array representing the LED matrix
// startX, startY: current top-left position of the boat
// boat: 2D array containing the boat shape (1 = filled, 0 = empty)
// boatWidth, boatHeight: dimensions of the boat array
// deltaX, deltaY: movement (-1, 0, 1)
// color: color of the boat
void moveBoat(CRGB frame[WIDTH][HEIGHT], int &startX, int &startY,
              const uint8_t boat[][3], int boatWidth, int boatHeight,
              int deltaX, int deltaY, CRGB color) {

    // Compute new position
    int newX = startX + deltaX;
    int newY = startY + deltaY;

    // Clamp position to screen boundaries
    if (newX < 0) newX = 0;
    if (newY < 0) newY = 0;
    if (newX + boatWidth > WIDTH) newX = WIDTH - boatWidth;
    if (newY + boatHeight > HEIGHT) newY = HEIGHT - boatHeight;

    // Draw boat at new position
    for (int y = 0; y < boatHeight; y++) {
        for (int x = 0; x < boatWidth; x++) {
            if (boat[y][x]) {
                frame[newX + x][newY + y] = color;
            }
        }
    }

    // Update boat coordinates
    startX = newX;
    startY = newY;
}
