#pragma once
#include <Arduino.h>

// Inline means: defined in header, compiled into any file that includes it
inline void readJoystick(int& x, int& y, int& button) {
    x = 0;
    y = 0;
    button = 0;
    int valueX = analogRead(A0);  // Read X axis
    int valueY = analogRead(A1);  // Read Y axis
    if(valueX < 400) x = -1;
    if(valueX > 600) x = 1;
    if(valueY < 400) y = 1;
    if(valueY > 600 && valueY < 1000) y = -1;
    if(valueY > 1000) button = 1;

}
