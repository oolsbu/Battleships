#pragma once
#include <Arduino.h>

// Read joystick with simple deadzones and a dedicated button input.
// Assumes axes on A0 (X), A1 (Y), and button on digital pin 2 (active HIGH).
inline void readJoystick(int& x, int& y, int& button) {
    const int LOW_THRESHOLD = 400;
    const int HIGH_THRESHOLD = 600;

    int valueX = analogRead(A0);
    int valueY = analogRead(A1);

    x = 0;
    y = 0;
    if (valueX < LOW_THRESHOLD) x = 1;          // right
    else if (valueX > HIGH_THRESHOLD) x = -1;    // left

    if (valueY < LOW_THRESHOLD) y = 1;           // down
    else if (valueY > HIGH_THRESHOLD) y = -1;    // up

    // If a dedicated button pin exists, read it; else fall back to hard press on X
    pinMode(2, INPUT);
    int btn = digitalRead(2);
    if (btn == HIGH) button = 1;
    else button = (valueX > 850) ? 1 : 0;  // fallback
}
