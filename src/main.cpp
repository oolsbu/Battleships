#include <Arduino.h>
#include <WiFiUdp.h>
#include "config.h"
#include "wifi_setup.h"
#include "udp_communication.h"
#include "joystick.h"
#include "led_matrix.h"
#include "game_logic.h"

// UDP object (left in main so network helpers keep working)
WiFiUDP udp;

CRGB leds[NUM_LEDS]; // define once here

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
        sendMessage("hello");
        Serial.println("UDP test message sent.");
    }
}

void loop() {
    // Read joystick input (-1, 0, 1)
    int xInput = 0, yInput = 0, button = 0;
    readJoystick(xInput, yInput, button);

    // Prepare frame and let game logic draw into it
    CRGB frame[WIDTH][HEIGHT];
    placementStep(xInput, yInput, button, frame);

    // Show the frame
    showFrame(frame);

    // Small delay controls responsiveness
    delay(75);
}
