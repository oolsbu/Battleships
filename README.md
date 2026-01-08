# Battleships (Arduino, WiFi, LED Matrix)

A simplified, production-formatted implementation of Battleships for two Arduinos. Each board connects to WiFi and exchanges UDP packets to play the game on a 16x16 LED matrix with a joystick.

## Features
- Boat placement with collision bounds and rotation
- Turn-based gameplay over UDP (AIM/SHOT/RESULT protocol)
- Clear visual feedback for hits, misses, sunk, win/loss states
- Minimal configuration; only the peer IP differs between boards

## Hardware Assumptions
- LED matrix: 16x16 WS2812B on pin `6`
- Joystick axes on `A0` (X) and `A1` (Y)
- Joystick button on digital pin `2` (HIGH when pressed). If not wired, a hard-right press on X will act as fallback button.

## Configuration
- Game board and boat configuration: see `src/config.h`
- WiFi credentials: set `WIFI_SSID` and `WIFI_PASSWORD` in `src/credentials.h`
- UDP ports and peer IP: set `LOCAL_PORT`, `OTHER_PORT`, and `OTHER_IP` in `src/config.h`

On the second Arduino, only change `OTHER_IP` to point to the first board.

## Build & Upload (PlatformIO)
1. Open this folder in VS Code.
2. Ensure the Arduino board with WiFi (e.g., MKR WiFi 1010 or compatible) is selected in `platformio.ini`.
3. Build and upload:

```sh
# Build
pio run
# Upload to the connected board
pio run -t upload
# (Optional) Open serial monitor
pio device monitor -b 9600
```

## Usage
1. Power both boards; they will connect to WiFi and start boat placement.
2. Use the joystick to move (`left/right/up/down`) and short-press button to rotate; long-press button to confirm boat placement.
3. After both players finish placement, the game will sync. The first shooter is determined by who finished first (configurable in `src/config.h`).
4. During your turn, move the aim and press the button to fire. Results are shown with transitions.

## Notes
- The code aims to be small and readable. Animations are limited to a simple WiFi icon during connection.
- If you prefer deterministic first shooter or to disable opponent aim overlay, toggle `RANDOM_FIRST_SHOOTER` and `SHOW_OPPONENT_AIM` in `src/config.h`.
