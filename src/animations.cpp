#include "animations.h"
#include "piskel.h"
#include "led_matrix.h"

#include <FastLED.h>
#include <Arduino.h>

static volatile bool g_running = false;
static AnimationType g_current = ANIM_NONE;
static uint32_t g_frameDelay = 200;
static uint32_t g_lastMillis = 0;
static int g_currentFrame = 0;

// Frame buffer arranged as frame[x][y] to match showFrame expectations
static CRGB g_frameBuf[WIDTH][HEIGHT];

void animationsInit() {
  // nothing for now, but keep hook if needed
}

static void drawWifiFrame(int frameIndex) {
  if (frameIndex < 0) return;
  const uint32_t *frame = wifi_piskel_data[frameIndex % NEW_PISKEL_FRAME_COUNT];
  for (int y = 0; y < HEIGHT; ++y) {
    for (int x = 0; x < WIDTH; ++x) {
      int idx = y * WIDTH + x;
      uint32_t px = frame[idx];
      CRGB c;
      // Piskel exported pixels are either 0x00000000 (transparent) or 0xff000000
      // Treat any non-zero entry as white for this simple monochrome animation.
      if (px == 0) {
        c = CRGB::Black;
      } else {
        c = CRGB::White;
      }
      g_frameBuf[x][y] = c;
    }
  }
  showFrame(g_frameBuf);
}

static void drawCurrentFrame() {
  switch (g_current) {
    case ANIM_WIFI:
      drawWifiFrame(g_currentFrame);
      break;
    default:
      break;
  }
}

void startAnimation(AnimationType type, uint32_t frameDelayMs) {
  g_current = type;
  g_frameDelay = frameDelayMs;
  g_currentFrame = 0;
  g_lastMillis = millis();
  g_running = true;
  drawCurrentFrame();
}

void stopAnimation() {
  g_running = false;
  g_current = ANIM_NONE;
}

void animationsUpdate() {
  if (!g_running) return;
  uint32_t now = millis();
  if ((now - g_lastMillis) >= g_frameDelay) {
    g_lastMillis = now;
    g_currentFrame++;
    if (g_currentFrame >= NEW_PISKEL_FRAME_COUNT) g_currentFrame = 0;
    drawCurrentFrame();
  }
}

bool isAnimationRunning() {
  return g_running;
}

void showAnimationFrame(AnimationType type, int frameIndex) {
  switch (type) {
    case ANIM_WIFI:
      drawWifiFrame(frameIndex % NEW_PISKEL_FRAME_COUNT);
      break;
    default:
      break;
  }
}

void clearAnimationDisplay() {
  for (int y = 0; y < HEIGHT; ++y)
    for (int x = 0; x < WIDTH; ++x)
      g_frameBuf[x][y] = CRGB::Black;
  showFrame(g_frameBuf);
}
