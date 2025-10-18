#ifndef LED_MATRIX_H
#define LED_MATRIX_H

#include <FastLED.h>

#define LED_PIN     6
#define WIDTH       16
#define HEIGHT      16
#define NUM_LEDS    (WIDTH * HEIGHT)
#define COLOR_ORDER GRB

extern CRGB leds[NUM_LEDS];  // just declare, define in main.cpp

// Setup LEDs
void ledSetup() {
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  FastLED.setBrightness(1);
  FastLED.clear();
}

// Map 2D coordinates to 1D (serpentine layout)
int XY(int x, int y) {
  if (y % 2 == 0) {
    return y * WIDTH + x;
  } else {
    return y * WIDTH + (WIDTH - 1 - x);
  }
}

// Display any 2D CRGB array on the LED matrix
void showFrame(CRGB frame[WIDTH][HEIGHT]) {
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      leds[XY(x, y)] = frame[x][y];
    }
  }
  FastLED.show();
}

#endif // LED_MATRIX_H
