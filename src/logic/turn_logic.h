#pragma once
#include "game_state.h"
#include "../udp_communication.h"

// ===== Ready Handshake Management =====

inline void handleReadyHandshake() {
    unsigned long now = millis();
    
    if (readyState != READY_SYNCED) {
        String msg = receiveMessage();
        if (msg.length() > 0) {
            if (msg.startsWith("READY:")) {
                unsigned long otherTime = (unsigned long) msg.substring(6).toInt();
                Serial.print("[READY] Received opponent READY timestamp: ");
                Serial.println(otherTime);
                opponentReady = true;
                opponentPlacementTime = otherTime;
                opponentPlacementTimeReceived = true;
                readyStateStartTime = now;
            } else if (msg.startsWith("READY")) {
                Serial.println("[READY] Received opponent READY (no timestamp)");
                opponentReady = true;
                opponentPlacementTime = 0;
                opponentPlacementTimeReceived = true;
                readyStateStartTime = now;
            } else {
                Serial.print("[READY] Deferring non-READY message during handshake: ");
                Serial.println(msg);
                pendingMessage = msg;
            }
        }
    }

    if (readyState == READY_PLACEMENT) {
        return;
    } else if (readyState == READY_WAITING_FOR_OPPONENT) {
        if (opponentPlacementTimeReceived) {
            Serial.println("[READY] Both timestamps available - deciding who shoots first...");
            const unsigned long MAX_UL = 0xFFFFFFFFUL;
            unsigned long myTime = placementFinishedTime ? placementFinishedTime : MAX_UL;
            unsigned long theirTime = opponentPlacementTime ? opponentPlacementTime : MAX_UL;
            Serial.print("[READY] myTime="); Serial.print(myTime);
            Serial.print(" theirTime="); Serial.println(theirTime);
            
#if RANDOM_FIRST_SHOOTER
            randomSeed(myTime ^ theirTime);
            bool iShootFirst = (random(0, 2) == 0);
            Serial.print("[READY] Random selection: ");
            Serial.println(iShootFirst ? "YOU SHOOT FIRST" : "OPPONENT SHOOTS FIRST");
#else
            bool iShootFirst = (myTime <= theirTime);
            Serial.print("[READY] Deterministic selection (timestamp-based): ");
            Serial.println(iShootFirst ? "YOU FINISHED FIRST - YOU SHOOT FIRST" : "OPPONENT FINISHED FIRST - YOU WAIT FIRST");
#endif
            
            if (iShootFirst) {
                gamePhase = PHASE_MY_TURN;
                Serial.println("[READY] >>> YOU SHOOT FIRST! <<<");
            } else {
                gamePhase = PHASE_WAIT_FOR_OPPONENT;
                Serial.println("[READY] >>> OPPONENT SHOOTS FIRST - YOU WAIT FIRST <<<");
            }
            readyState = READY_SYNCED;
            readyStateStartTime = now;
        } else if ((now - placementFinishedTime) > READY_HANDSHAKE_TIMEOUT_MS) {
            Serial.println("[READY] Opponent timeout! Starting anyway... you shoot first");
            readyState = READY_SYNCED;
            readyStateStartTime = now;
            gamePhase = PHASE_MY_TURN;
        }
    } else if (readyState == READY_SYNCED) {
    }
}

inline void notifyReadyToOpponent() {
    Serial.println("[READY] Notifying opponent that placement is complete (with timestamp)...");
    placementFinishedTime = millis();
    char buf[32];
    snprintf(buf, sizeof(buf), "READY:%lu", placementFinishedTime);
    Serial.print("[READY] Sending: ");
    Serial.println(buf);
    sendMessage(buf);
}

// ===== Phase Transitions =====

inline void updatePhaseTransitions() {
    unsigned long now = millis();
    if (gamePhase == PHASE_SHOW_RESULT && (now - phaseStartTime) >= RESULT_DISPLAY_TIME_MS) {
        Serial.println("[PHASE] PHASE_SHOW_RESULT -> PHASE_WAIT_FOR_OPPONENT");
        gamePhase = PHASE_WAIT_FOR_OPPONENT;
    }
    if (gamePhase == PHASE_OPPONENT_SHOT && (now - phaseStartTime) >= RESULT_DISPLAY_TIME_MS) {
        Serial.println("[PHASE] PHASE_OPPONENT_SHOT -> PHASE_MY_TURN");
        gamePhase = PHASE_MY_TURN;
    }
}
