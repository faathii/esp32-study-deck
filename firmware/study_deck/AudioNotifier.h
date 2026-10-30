#ifndef AUDIO_NOTIFIER_H
#define AUDIO_NOTIFIER_H

#include <Arduino.h>
#include "HardwareConfig.h"

class AudioNotifier {
public:
    void begin() {
        pinMode(PIN_BUZZER, OUTPUT);
        digitalWrite(PIN_BUZZER, LOW);
    }

    void playClick() {
        tone(PIN_BUZZER, 2400, 15);
    }

    void playStart() {
        tone(PIN_BUZZER, 1760, 60);
        delay(70);
        tone(PIN_BUZZER, 2349, 100);
    }

    void playPause() {
        tone(PIN_BUZZER, 2000, 60);
        delay(70);
        tone(PIN_BUZZER, 1500, 80);
    }

    void playWorkFinished() {
        // Melodía de éxito al completar bloque de estudio
        tone(PIN_BUZZER, 1568, 90);
        delay(110);
        tone(PIN_BUZZER, 1960, 90);
        delay(110);
        tone(PIN_BUZZER, 2349, 90);
        delay(110);
        tone(PIN_BUZZER, 3136, 220);
    }

    void playBreakFinished() {
        // Alerta de retorno al trabajo
        tone(PIN_BUZZER, 2093, 100);
        delay(130);
        tone(PIN_BUZZER, 1760, 100);
        delay(130);
        tone(PIN_BUZZER, 2093, 180);
    }

    void playSkip() {
        tone(PIN_BUZZER, 1800, 40);
        delay(50);
        tone(PIN_BUZZER, 2200, 40);
    }
};

#endif // AUDIO_NOTIFIER_H

