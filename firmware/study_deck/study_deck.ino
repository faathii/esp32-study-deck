#include <Arduino.h>
#include <TFT_eSPI.h>
#include "HardwareConfig.h"
#include "CST820.h"
#include "AudioNotifier.h"
#include "LedIndicator.h"
#include "PomodoroEngine.h"
#include "UIManager.h"

// Instancias de periféricos y controladores
TFT_eSPI tft = TFT_eSPI();
CST820 touch(I2C_SDA_PIN, I2C_SCL_PIN, TP_RST_PIN, TP_INT_PIN);
AudioNotifier audio;
LedIndicator led;
PomodoroEngine pomodoro;
UIManager ui(tft);

// Variables de control temporal
unsigned long lastLdrCheck = 0;
unsigned long lastTouchTime = 0;
int currentLdr = 2000;
int currentBrightness = 220;

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n==============================================");
    Serial.println("  ESTACION DE ESTUDIO INTELIGENTE - STUDY DECK");
    Serial.println("  Hardware: Guition ESP32-2432S024C (2.4\")");
    Serial.println("==============================================\n");

    // 1. Inicializar retroiluminación con nivel confortable
    pinMode(PIN_BACKLIGHT, OUTPUT);
    analogWrite(PIN_BACKLIGHT, currentBrightness);

    // 2. Inicializar sensor LDR
    pinMode(PIN_LDR, INPUT);
    currentLdr = analogRead(PIN_LDR);

    // 3. Inicializar audio y LED RGB
    audio.begin();
    led.begin();
    led.setMode(LED_MODE_IDLE);

    // 4. Inicializar motor Pomodoro
    pomodoro.begin();

    // 5. Inicializar pantalla táctil y dibujar interfaz completa
    ui.begin();
    touch.begin();
    ui.fullRedraw(pomodoro, currentLdr);

    // Sonido de bienvenida
    audio.playStart();
    Serial.println("[Sistema] Study Deck listo y a la espera de interacción.");
}

void loop() {
    // 1. Actualización no bloqueante del LED RGB (parpadeo en pausa)
    led.update();

    // 2. Actualización de la máquina de estados Pomodoro
    PomodoroEvent event = pomodoro.update();
    if (event == POMO_EVENT_FOCUS_COMPLETE) {
        Serial.println("[Pomodoro] ¡Bloque de estudio completado! Iniciando descanso.");
        audio.playWorkFinished();
        led.setMode(LED_MODE_BREAK);
        ui.fullRedraw(pomodoro, currentLdr);
    } else if (event == POMO_EVENT_BREAK_COMPLETE) {
        Serial.println("[Pomodoro] ¡Descanso finalizado! Volviendo al modo enfoque.");
        audio.playBreakFinished();
        led.setMode(LED_MODE_FOCUS);
        ui.fullRedraw(pomodoro, currentLdr);
    }

    // 3. Lectura táctil con debounce de 250 ms
    int touchX = -1, touchY = -1;
    if (touch.getTouch(&touchX, &touchY)) {
        if (millis() - lastTouchTime >= 250) {
            lastTouchTime = millis();
            Serial.printf("[Touch] Pulsado en X=%d, Y=%d\n", touchX, touchY);

            UIButtonAction action = ui.handleTouch(touchX, touchY);
            switch (action) {
                case UI_ACTION_PLAY_PAUSE:
                    pomodoro.togglePlayPause();
                    if (pomodoro.isRunning()) {
                        audio.playStart();
                        led.setMode(pomodoro.isFocus() ? LED_MODE_FOCUS : LED_MODE_BREAK);
                    } else if (pomodoro.isPaused()) {
                        audio.playPause();
                        led.setMode(LED_MODE_PAUSED);
                    }
                    break;

                case UI_ACTION_RESET:
                    audio.playClick();
                    pomodoro.reset();
                    led.setMode(LED_MODE_IDLE);
                    break;

                case UI_ACTION_SKIP:
                    audio.playSkip();
                    pomodoro.skip();
                    led.setMode(pomodoro.isFocus() ? LED_MODE_FOCUS : LED_MODE_BREAK);
                    break;

                case UI_ACTION_SELECT_25:
                    if (!pomodoro.isRunning()) {
                        audio.playClick();
                        pomodoro.setFocusMinutes(25);
                    }
                    break;

                case UI_ACTION_SELECT_50:
                    if (!pomodoro.isRunning()) {
                        audio.playClick();
                        pomodoro.setFocusMinutes(50);
                    }
                    break;

                case UI_ACTION_NONE:
                default:
                    break;
            }
        }
    }

    // 4. Actualización del reloj digital y barra de progreso en pantalla
    ui.updateTimerDisplay(pomodoro, currentLdr);

    // 5. Ajuste de brillo suave y lectura de LDR cada 400 ms
    if (millis() - lastLdrCheck >= 400) {
        lastLdrCheck = millis();
        currentLdr = analogRead(PIN_LDR);

        // Mapeo suave de luminosidad: 4000 (oscuridad) -> 60 PWM, 500 (luz fuerte) -> 255 PWM
        int targetBrightness = map(currentLdr, 4000, 500, 70, 255);
        targetBrightness = constrain(targetBrightness, 70, 255);

        // Transición suave de brillo
        if (abs(targetBrightness - currentBrightness) > 8) {
            currentBrightness += (targetBrightness > currentBrightness) ? 5 : -5;
            analogWrite(PIN_BACKLIGHT, currentBrightness);
        }
    }
}

