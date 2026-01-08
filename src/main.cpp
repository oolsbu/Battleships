#include <Arduino.h>
#include <WiFiUdp.h>
#include "config.h"
#include "wifi_setup.h"
#include "udp_communication.h"
#include "joystick.h"
#include "led_matrix.h"
#include "game_logic.h"
#include "credentials.h"
#include "animations.h"

WiFiUDP udp;
bool finished = false;

CRGB leds[NUM_LEDS];
CRGB frame[WIDTH][HEIGHT];

void setup() {
    Serial.begin(9600);
    while (!Serial) { delay(10); }

    ledSetup();

    if (!beginPlacement(sizes, counts, types)) {
        Serial.println("Too many boats configured");
    }

    const char* ssid = WIFI_SSID;
    const char* password = WIFI_PASSWORD;
    
    animationsInit();
    startAnimation(ANIM_WIFI, 150);
    bool wifiOk = connectWiFi(ssid, password, 20000);
    stopAnimation();

    if (wifiOk) {
        showAnimationFrame(ANIM_WIFI, 3);
        delay(2000);
        clearAnimationDisplay();
        startUDP(LOCAL_PORT);
    } else {
        Serial.println("Failed to connect to WiFi - continuing without network");
        unsigned long t0 = millis();
            showAnimationFrame(ANIM_WIFI, 0);
            delay(300);
            clearAnimationDisplay();
            delay(300);
    }
}

void loop() {
    int xInput = 0, yInput = 0, button = 0;
    readJoystick(xInput, yInput, button);


    if(!finished){
        placementStep(xInput, yInput, button, frame, finished);
        static bool placementWasNotFinished = true;
        if (placementWasNotFinished && finished) {
            notifyReadyToOpponent();
            readyState = READY_WAITING_FOR_OPPONENT;
            placementFinishedTime = millis();
            placementWasNotFinished = false;
        }
    }
    else {
        handleReadyHandshake();
        if (readyState == READY_SYNCED) {
            aim(xInput, yInput, button, frame);
        } else {
            for (int y = 0; y < HEIGHT; y++)
                for (int x = 0; x < WIDTH; x++)
                    frame[x][y] = CRGB::Black;
            frame[0][0] = CRGB::Blue;
            static unsigned long lastPrint = 0;
            if ((millis() - lastPrint) > 1000) {
                Serial.println("[SYNC] Waiting for opponent to finish placement...");
                lastPrint = millis();
            }
        }
    }
    showFrame(frame);
    delay(75);
}
