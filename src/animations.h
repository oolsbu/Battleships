#ifndef ANIMATIONS_H
#define ANIMATIONS_H

#include <stdint.h>

typedef enum {
  ANIM_NONE = -1,
  ANIM_WIFI = 0,
} AnimationType;

void animationsInit();

// Start the given animation with optional frame delay in milliseconds
void startAnimation(AnimationType type, uint32_t frameDelayMs = 200);

// Stop any running animation
void stopAnimation();

// Call this from main loop to update the animation (non-blocking)
void animationsUpdate();

// Returns true if an animation is currently running
bool isAnimationRunning();

// Draw a single frame of an animation (immediate)
void showAnimationFrame(AnimationType type, int frameIndex);

// Clear the animation/display (black)
void clearAnimationDisplay();

#endif
