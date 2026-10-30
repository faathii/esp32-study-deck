#ifndef LED_INDICATOR_H
#define LED_INDICATOR_H

#include <Arduino.h>
#include "HardwareConfig.h"

enum LedMode {
    LED_MODE_OFF,
    LED_MODE_IDLE,    // Azul suave
    LED_MODE_FOCUS,   // Rojo constante (No Molestar)
    LED_MODE_BREAK,   // Verde constante (Descanso)
    LED_MODE_PAUSED,  // Amarillo parpadeante
    LED_MODE_CUSTOM   // Control manual directo
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

    // Control físico directo del LED RGB (Lógica activa en BAJO)
    void setRaw(bool r, bool g, bool b) {
        _mode = LED_MODE_CUSTOM;
        digitalWrite(PIN_LED_RED, !r);
        digitalWrite(PIN_LED_GREEN, !g);
        digitalWrite(PIN_LED_BLUE, !b);
    }

    void update() {
        // En modo pausa, parpadea en amarillo cada 500 ms de forma no bloqueante
        if (_mode == LED_MODE_PAUSED) {
            if (millis() - _lastBlinkTime >= 500) {
                _lastBlinkTime = millis();
                _blinkState = !_blinkState;
                if (_blinkState) {
                    digitalWrite(PIN_LED_RED, LOW);   // Encendido
                    digitalWrite(PIN_LED_GREEN, LOW); // Encendido
                    digitalWrite(PIN_LED_BLUE, HIGH); // Apagado
                } else {
                    digitalWrite(PIN_LED_RED, HIGH);
                    digitalWrite(PIN_LED_GREEN, HIGH);
                    digitalWrite(PIN_LED_BLUE, HIGH);
                }
            }
        }
    }

private:
    LedMode _mode = LED_MODE_OFF;
    unsigned long _lastBlinkTime = 0;
    bool _blinkState = true;

    void applyHardware() {
        switch (_mode) {
            case LED_MODE_FOCUS:
                digitalWrite(PIN_LED_RED, LOW);
                digitalWrite(PIN_LED_GREEN, HIGH);
                digitalWrite(PIN_LED_BLUE, HIGH);
                break;
            case LED_MODE_BREAK:
                digitalWrite(PIN_LED_RED, HIGH);
                digitalWrite(PIN_LED_GREEN, LOW);
                digitalWrite(PIN_LED_BLUE, HIGH);
                break;
            case LED_MODE_PAUSED:
                digitalWrite(PIN_LED_RED, LOW);
                digitalWrite(PIN_LED_GREEN, LOW);
                digitalWrite(PIN_LED_BLUE, HIGH);
                break;
            case LED_MODE_IDLE:
                digitalWrite(PIN_LED_RED, HIGH);
                digitalWrite(PIN_LED_GREEN, HIGH);
                digitalWrite(PIN_LED_BLUE, LOW);
                break;
            case LED_MODE_OFF:
            default:
                digitalWrite(PIN_LED_RED, HIGH);
                digitalWrite(PIN_LED_GREEN, HIGH);
                digitalWrite(PIN_LED_BLUE, HIGH);
                break;
        }
    }
};

#endif // LED_INDICATOR_H
