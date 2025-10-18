#include <Arduino.h>
#include <WiFiUdp.h>
#include "config.h"
#include "wifi_setup.h"
#include "udp_communication.h"
#include "joystick.h"
#include "led_matrix.h"
#include "game_logic.h"

// UDP object
WiFiUDP udp;

CRGB leds[NUM_LEDS]; // define once here

// Boat definition
const uint8_t boatShape[1][3] = {{1, 1, 1}}; // 1x3 boat
int boatX = 0;
int boatY = 0;

void setup() {
    Serial.begin(9600);
    while (!Serial) { delay(10); }

    // LED matrix setup
    ledSetup();

    // Initialize a blank frame and draw the boat at start
    CRGB frame[WIDTH][HEIGHT];
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            frame[x][y] = CRGB::Black;

    moveBoat(frame, boatX, boatY, boatShape, 3, 1, 0, 0, CRGB::Blue);
    showFrame(frame);

    // Connect WiFi
    const char* ssid = WIFI_SSID;
    const char* password = WIFI_PASSWORD;

    if (!connectWiFi(ssid, password, 20000)) {
        Serial.println("Failed to connect to WiFi - aborting UDP send");
        return;
    }

    // Start UDP and send a test message
    startUDP(LOCAL_PORT);
    sendMessage("hello");
    Serial.println("Message sent.");
}

void loop() {
    // Read joystick input (-1, 0, 1)
    int xInput, yInput, button;
    readJoystick(xInput, yInput, button);

    Serial.print("X: "); Serial.print(xInput);
    Serial.print("  Y: "); Serial.print(yInput);
    Serial.print("  Button: "); Serial.println(button);

    // Clear frame
    CRGB frame[WIDTH][HEIGHT];
    for (int y = 0; y < HEIGHT; y++)
        for (int x = 0; x < WIDTH; x++)
            frame[x][y] = CRGB::Black;

    // Move boat based on joystick input
    moveBoat(frame, boatX, boatY, boatShape, 3, 1, xInput, yInput, CRGB::Blue);

    // Display the frame
    showFrame(frame);

    delay(100); // adjust movement speed
}
