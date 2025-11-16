#include <Arduino.h>
#include <WiFiUdp.h>
#include "config.h"
#include "wifi_setup.h"
#include "udp_communication.h"
#include "joystick.h"
#include "led_matrix.h"
#include "game_logic.h"
#include "player_logic.h"
#include "credentials.h"

// UDP object (left in main so network helpers keep working)
WiFiUDP udp;
bool finished = false;

CRGB leds[NUM_LEDS]; // define once here
CRGB frame[WIDTH][HEIGHT];

void setup() {
    Serial.begin(9600);
    while (!Serial) { delay(10); }

    // LED matrix
    ledSetup();

    if (!beginPlacement(sizes, counts, types)) {
        Serial.println("Too many boats configured (MAX_BOATS exceeded)");
    }

    // Connect WiFi (optional for placement, kept from original project)
    const char* ssid = WIFI_SSID;
    const char* password = WIFI_PASSWORD;
    
    if (!connectWiFi(ssid, password, 20000)) {
        Serial.println("Failed to connect to WiFi - continuing without network");
    } else {
        startUDP(LOCAL_PORT);
    }
}

void loop() {
    // Read joystick input (-1, 0, 1)
    int xInput = 0, yInput = 0, button = 0;
    readJoystick(xInput, yInput, button);

    // Prepare frame and let game logic draw into it

    if(!finished){
        placementStep(xInput, yInput, button, frame, finished);
        // Handle state transition when placement finishes
        static bool placementWasNotFinished = true;
        if (placementWasNotFinished && finished) {
            Serial.println("\n========================================");
            Serial.println("[PLACEMENT] Boat placement COMPLETE!");
            Serial.println("[PLACEMENT] Waiting for opponent...");
            Serial.println("========================================\n");
            notifyReadyToOpponent();
            readyState = READY_WAITING_FOR_OPPONENT;
            placementFinishedTime = millis();
            placementWasNotFinished = false;
        }
    }
    else {
        // Game phase: handle ready handshake and then gameplay
        handleReadyHandshake();
        
        // Only allow aim/shooting if both players are synced
        if (readyState == READY_SYNCED) {
            // Both players ready - run normal gameplay
            aim(xInput, yInput, button, frame);
        } else {
            // Still waiting for opponent to finish placement - show waiting screen
            for (int y = 0; y < HEIGHT; y++)
                for (int x = 0; x < WIDTH; x++)
                    frame[x][y] = CRGB::Black;
            frame[0][0] = CRGB::Blue;  // Show a blue pixel indicating waiting
            // Periodic debug output
            static unsigned long lastPrint = 0;
            if ((millis() - lastPrint) > 1000) {
                Serial.println("[SYNC] Waiting for opponent to finish placement...");
                lastPrint = millis();
            }
        }
    }

    // Show the frame
    showFrame(frame);

    // Small delay controls responsiveness
    delay(75);
}
