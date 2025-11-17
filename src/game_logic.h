#pragma once
#include "logic/game_state.h"
#include "logic/board_logic.h"
#include "logic/placement_logic.h"
#include "logic/display_logic.h"
#include "logic/turn_logic.h"
#include "udp_communication.h"


// ===== Main Placement Loop =====

inline void placementStep(int dx, int dy, int button, CRGB frame[16][16], bool &finished) {
    if (dx != 0 || dy != 0) moveCurrentBoat(dx, dy, button);
    if (button && !prevButtonState) buttonPressTime = millis();
    if (!button && prevButtonState) {
        unsigned long pressDuration = millis() - buttonPressTime;
        if (pressDuration >= LONG_PRESS_MS) confirmPlacement(finished);
        else rotateCurrentBoat();
    }
    prevButtonState = button;
    drawPlacementFrame(frame);
}

// ===== Main Aiming Loop =====

inline void aim(int dx, int dy, int button, CRGB frame[16][16]) {
    // Process incoming messages
    String msg = receiveMessage();
    if (msg.length() > 0) {
        Serial.print("[AIM] Received message: ");
        Serial.println(msg);

        if (msg.startsWith("AIM:")) {
            int comma = msg.indexOf(',');
            if (comma > 4) {
                int x = msg.substring(4, comma).toInt();
                int y = msg.substring(comma + 1).toInt();
                if (isAimWithinBounds(x, y)) {
                    Serial.print("[AIM] Opponent aiming at: ");
                    Serial.print(x);
                    Serial.print(",");
                    Serial.println(y);
                    oppAimX = x;
                    oppAimY = y;
                    oppAimTime = millis();
                }
            }
        }
        else if (msg.startsWith("SHOT:")) {
            int comma = msg.indexOf(',');
            if (comma > 5) {
                int sx = msg.substring(5, comma).toInt();
                int sy = msg.substring(comma + 1).toInt();
                Serial.print("[AIM] Opponent shot at: ");
                Serial.print(sx);
                Serial.print(",");
                Serial.println(sy);

                if (isAimWithinBounds(sx, sy)) {
                    bool wasHit = occupied[sx][sy];
                    Serial.print("[AIM] Shot result: ");
                    Serial.println(wasHit ? "HIT" : "MISS");

                    if (wasHit) hitMap[sx][sy] = true;
                    int boatIdx = boatIndexAt(sx, sy);
                    bool sunk = (boatIdx >= 0) && boatSunk(boatIdx);
                    char reply[32];
                    if (wasHit) snprintf(reply, sizeof(reply), "RESULT:%s", sunk ? "SINK" : "HIT");
                    else snprintf(reply, sizeof(reply), "RESULT:MISS");
                    
                    Serial.println("[AIM] >>> Transitioning to PHASE_OPPONENT_SHOT (local)");
                    gamePhase = PHASE_OPPONENT_SHOT;
                    phaseStartTime = millis();
                    
                    Serial.print("[AIM] Sending reply: ");
                    Serial.println(reply);
                    sendMessage(reply);
                }
            }
        }
        else if (msg.startsWith("RESULT:")) {
            String result = msg.substring(7);
            Serial.print("[AIM] Received result: ");
            Serial.println(result);

            if (aimX >= 0 && aimY >= 0) {
                if (result.startsWith("HIT")) opponentMap[aimX][aimY] = 2;
                else if (result.startsWith("MISS")) opponentMap[aimX][aimY] = 1;
                else if (result.startsWith("SINK")) {
                    opponentMap[aimX][aimY] = 2;
                    markSunkOpponentBoat(aimX, aimY);
                }
            }
            Serial.println("[AIM] >>> Transitioning to PHASE_SHOW_RESULT");
            gamePhase = PHASE_SHOW_RESULT;
            phaseStartTime = millis();
        }
    }

    // Update phase transitions
    updatePhaseTransitions();

    // Handle aiming input and drawing
    if (gamePhase == PHASE_MY_TURN) {
        // Player can move cursor and aim
        bool moved = false;
        if (dx != 0 || dy != 0) {
            aimX += dx;
            aimY += dy;
            moved = true;
            clampAim(aimX, aimY);
        }
        
#if SHOW_OPPONENT_AIM
        if (moved && (millis() - lastAimSendTime) >= AIM_SEND_INTERVAL_MS) {
            char aimMsg[32];
            snprintf(aimMsg, sizeof(aimMsg), "AIM:%d,%d", aimX, aimY);
            sendMessage(aimMsg);
            lastAimSendTime = millis();
        }
#endif
        
        if (button == 1) {
            char shotMsg[32];
            snprintf(shotMsg, sizeof(shotMsg), "SHOT:%d,%d", aimX, aimY);
            Serial.print("[SHOOT] FIRING at ");
            Serial.println(shotMsg);
            sendMessage(shotMsg);
            Serial.println("[SHOOT] >>> Transitioning to PHASE_WAIT_FOR_OPPONENT");
            gamePhase = PHASE_WAIT_FOR_OPPONENT;
        }
        
        drawMyTurnFrame(frame);
    }
    else if (gamePhase == PHASE_OPPONENT_SHOT) {
        drawOpponentShotFrame(frame);
    }
    else if (gamePhase == PHASE_SHOW_RESULT) {
        drawShowResultFrame(frame);
    }
    else if (gamePhase == PHASE_WAIT_FOR_OPPONENT) {
        drawWaitForOpponentFrame(frame);
    }
}
