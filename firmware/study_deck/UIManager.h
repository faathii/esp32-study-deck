#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "PomodoroEngine.h"
#include "HardwareConfig.h"

enum UIButtonAction {
    UI_ACTION_NONE,
    UI_ACTION_PLAY_PAUSE,
    UI_ACTION_RESET,
    UI_ACTION_SKIP,
    UI_ACTION_SELECT_25,
    UI_ACTION_SELECT_50
};

class UIManager {
public:
    UIManager(TFT_eSPI &tft);

    void begin();
    void fullRedraw(const PomodoroEngine &engine, int ldrValue);
    void updateTimerDisplay(const PomodoroEngine &engine, int ldrValue);

    // Detección de toque en los botones
    UIButtonAction handleTouch(int x, int y);

private:
    TFT_eSPI &_tft;

    // Estados previos para redibujar únicamente lo que cambia (evitar parpadeos)
    uint32_t _lastRemainingSec = 0xFFFFFFFF;
    PomodoroState _lastState = (PomodoroState)0xFF;
    uint8_t _lastCycle = 0xFF;
    uint16_t _lastFocusMin = 0xFF;
    int _lastLdr = -1;
    float _lastProgress = -1.0f;

    void drawTopBar(const PomodoroEngine &engine, int ldrValue);
    void drawModeButtons(const PomodoroEngine &engine);
    void drawCountdown(const PomodoroEngine &engine);
    void drawProgressBar(const PomodoroEngine &engine);
    void drawActionButtons(const PomodoroEngine &engine);
};

#endif // UI_MANAGER_H

