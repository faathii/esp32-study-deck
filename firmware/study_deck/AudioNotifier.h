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

    bool isMuted() const { return _muted; }
    void setMuted(bool mute) { _muted = mute; }
    void toggleMute() { _muted = !_muted; }

    void playClick() {
        if (_muted) return;
        tone(PIN_BUZZER, 2400, 15);
    }

    void playStart() {
        if (_muted) return;
        tone(PIN_BUZZER, 1760, 60);
        delay(70);
        tone(PIN_BUZZER, 2349, 100);
    }

    void playPause() {
        if (_muted) return;
        tone(PIN_BUZZER, 2000, 60);
        delay(70);
        tone(PIN_BUZZER, 1500, 80);
    }

    void playWorkFinished() {
        if (_muted) return;
        tone(PIN_BUZZER, 1568, 90);
        delay(110);
        tone(PIN_BUZZER, 1960, 90);
        delay(110);
        tone(PIN_BUZZER, 2349, 90);
        delay(110);
        tone(PIN_BUZZER, 3136, 220);
    }

    void playBreakFinished() {
        if (_muted) return;
        tone(PIN_BUZZER, 2093, 100);
        delay(130);
        tone(PIN_BUZZER, 1760, 100);
        delay(130);
        tone(PIN_BUZZER, 2093, 180);
    }

    void playSkip() {
        if (_muted) return;
        tone(PIN_BUZZER, 1800, 40);
        delay(50);
        tone(PIN_BUZZER, 2200, 40);
    }

    // Alarma de 3 segundos para inicio y fin de franjas de estudio
    void playStudyAlarm3s() {
        if (_muted) return;
        for (int i = 0; i < 6; i++) {
            tone(PIN_BUZZER, 2093, 220); // Do6
            delay(250);
            tone(PIN_BUZZER, 2637, 220); // Mi6
            delay(250);
        }
        noTone(PIN_BUZZER);
    }

private:
    bool _muted = false;
};

#endif // AUDIO_NOTIFIER_H
