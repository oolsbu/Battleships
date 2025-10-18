
#include <Arduino.h>
#include <WiFiUdp.h>
#include "config.h"
#include "wifi_setup.h"
#include "udp_communication.h"
#include "joystick.h"

WiFiUDP udp;

void setup() {
	Serial.begin(9600);
	while (!Serial) { delay(10); }

	const char* ssid = WIFI_SSID;
	const char* password = WIFI_PASSWORD;

	if (!connectWiFi(ssid, password, 20000)) {
		Serial.println("Failed to connect to WiFi - aborting UDP send");
		return;
	}

	startUDP(LOCAL_PORT);

	sendMessage("hello");


	Serial.println("Message sent.");
}

void loop() {
	int x, y, button;
    readJoystick(x, y, button);

    Serial.print("X: ");
    Serial.print(x);
    Serial.print("  Y: ");
    Serial.print(y);
    Serial.print("  Button: ");
    Serial.println(button);

    delay(1000);
}