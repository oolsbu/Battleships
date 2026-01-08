#pragma once
#include <Arduino.h>

inline void readJoystick(int& x, int& y, int& button) {
    const int LOW_THRESHOLD = 400;
    const int HIGH_THRESHOLD = 600;

    int valueX = analogRead(A0);
    int valueY = analogRead(A1);

    x = 0;
    y = 0;
    if (valueX < LOW_THRESHOLD) x = 1;
    else if (valueX > HIGH_THRESHOLD) x = -1;

    if (valueY < LOW_THRESHOLD) y = 1;
    else if (valueY > HIGH_THRESHOLD) y = -1;

    pinMode(2, INPUT);
    int btn = digitalRead(2);
    if (btn == HIGH) button = 1;
    else button = (valueX > 850) ? 1 : 0;
}
