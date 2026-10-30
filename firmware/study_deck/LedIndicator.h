#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

#include <Arduino.h>
#include "HardwareConfig.h"

enum LedMode {
    LED_MODE_OFF,
    LED_MODE_IDLE,    // Azul suave
    LED_MODE_FOCUS,   // Rojo constante (No Molestar)
    LED_MODE_BREAK,   // Verde constante (Descanso)
    LED_MODE_PAUSED   // Amarillo parpadeante
};

class LedIndicator {
public:
    void begin() {
        pinMode(PIN_LED_RED, OUTPUT);
        pinMode(PIN_LED_GREEN, OUTPUT);
        pinMode(PIN_LED_BLUE, OUTPUT);
        setMode(LED_MODE_IDLE);
    }

    void setMode(LedMode mode) {
        _mode = mode;
        _lastBlinkTime = millis();
        _blinkState = true;
        applyHardware();
    }

    LedMode getMode() const {
        return _mode;
    }

    void update() {
        // En modo pausa, parpadea en amarillo cada 500 ms de forma no bloqueante
        if (_mode == LED_MODE_PAUSED) {
            if (millis() - _lastBlinkTime >= 500) {
                _lastBlinkTime = millis();
                _blinkState = !_blinkState;
                if (_blinkState) {
                    setRaw(true, true, false); // Amarillo (Rojo + Verde)
                } else {
                    setRaw(false, false, false); // Apagado
                }
            }
        }
    }

private:
    LedMode _mode = LED_MODE_OFF;
    unsigned long _lastBlinkTime = 0;
    bool _blinkState = true;

    // Control físico del LED RGB (Cátodo común / Lógica activa en BAJO)
    void setRaw(bool r, bool g, bool b) {
        digitalWrite(PIN_LED_RED, !r);
        digitalWrite(PIN_LED_GREEN, !g);
        digitalWrite(PIN_LED_BLUE, !b);
    }

    void applyHardware() {
        switch (_mode) {
            case LED_MODE_FOCUS:
                setRaw(true, false, false); // Rojo fijo
                break;
            case LED_MODE_BREAK:
                setRaw(false, true, false); // Verde fijo
                break;
            case LED_MODE_PAUSED:
                setRaw(true, true, false);  // Amarillo inicial
                break;
            case LED_MODE_IDLE:
                setRaw(false, false, true); // Azul de espera
                break;
            case LED_MODE_OFF:
            default:
                setRaw(false, false, false);
                break;
        }
    }
};

#endif // LED_INDICATOR_H

