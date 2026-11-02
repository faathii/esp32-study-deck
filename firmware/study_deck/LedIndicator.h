#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

#include <Arduino.h>
#include "HardwareConfig.h"

enum LedMode {
    LED_MODE_OFF,
    LED_MODE_IDLE,
    LED_MODE_FOCUS,
    LED_MODE_BREAK,
    LED_MODE_PAUSED,
    LED_MODE_CUSTOM
};

class LedIndicator {
public:
    void begin() {
        pinMode(PIN_LED_RED, OUTPUT);
        pinMode(PIN_LED_GREEN, OUTPUT);
        pinMode(PIN_LED_BLUE, OUTPUT);
        turnOff();
    }

    // Apaga completamente el LED RGB a nivel de hardware
    void turnOff() {
        _alerting = false;
        _mode = LED_MODE_OFF;
        digitalWrite(PIN_LED_RED, HIGH);
        digitalWrite(PIN_LED_GREEN, HIGH);
        digitalWrite(PIN_LED_BLUE, HIGH);
    }

    // Dispara un parpadeo de alarma durante durationMs (exclusivo para avisos y alarmas)
    void flashAlert(uint32_t durationMs = 3000, bool r = false, bool g = true, bool b = true) {
        _alerting = true;
        _alertEndTime = millis() + durationMs;
        _alertR = r;
        _alertG = g;
        _alertB = b;
        _lastFlashToggle = millis();
        _flashState = true;
        applyColor(_alertR, _alertG, _alertB);
    }

    bool isAlerting() const {
        return _alerting;
    }

    // Compatibilidad: solo se activa si hay alarma; de lo contrario se apaga
    void setMode(LedMode mode) {
        if (mode == LED_MODE_OFF) {
            turnOff();
        }
        // El resto de modos no dejan encendido el LED para evitar contaminación lumínica
    }

    LedMode getMode() const {
        return _alerting ? LED_MODE_CUSTOM : LED_MODE_OFF;
    }

    void setRaw(bool r, bool g, bool b) {
        if (!r && !g && !b) {
            turnOff();
        } else {
            // Si se llama explícitamente, aplicar pero con límite temporal o de depuración
            applyColor(r, g, b);
        }
    }

    void update() {
        if (_alerting) {
            if (millis() >= _alertEndTime) {
                turnOff();
            } else {
                if (millis() - _lastFlashToggle >= 180) {
                    _lastFlashToggle = millis();
                    _flashState = !_flashState;
                    if (_flashState) {
                        applyColor(_alertR, _alertG, _alertB);
                    } else {
                        digitalWrite(PIN_LED_RED, HIGH);
                        digitalWrite(PIN_LED_GREEN, HIGH);
                        digitalWrite(PIN_LED_BLUE, HIGH);
                    }
                }
            }
        }
    }

private:
    LedMode _mode = LED_MODE_OFF;
    bool _alerting = false;
    unsigned long _alertEndTime = 0;
    unsigned long _lastFlashToggle = 0;
    bool _flashState = false;
    bool _alertR = false;
    bool _alertG = true;
    bool _alertB = true;

    void applyColor(bool r, bool g, bool b) {
        // Lógica activa en BAJO en hardware Guition ESP32
        digitalWrite(PIN_LED_RED, !r);
        digitalWrite(PIN_LED_GREEN, !g);
        digitalWrite(PIN_LED_BLUE, !b);
    }
};

#endif // LED_INDICATOR_H
