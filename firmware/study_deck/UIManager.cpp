#include "UIManager.h"

// Colores personalizados de interfaz (RGB565)
#define COLOR_BG          0x0841 // Gris oscuro / pizarra profundo
#define COLOR_CARD        0x18E3 // Superficie de tarjeta elevada
#define COLOR_FOCUS_RED   0xF986 // Rojo brillante
#define COLOR_BREAK_GREEN 0x05E6 // Verde esmeralda
#define COLOR_PAUSE_AMBER 0xFDA0 // Ámbar / Naranja
#define COLOR_IDLE_BLUE   0x3BFF // Azul cian moderno
#define COLOR_TEXT_MUTED  0x9CD3 // Gris suave para texto secundario

UIManager::UIManager(TFT_eSPI &tft) : _tft(tft) {}

void UIManager::begin() {
    _tft.begin();
    _tft.setRotation(SCREEN_ROTATION);
    _tft.fillScreen(COLOR_BG);
}

void UIManager::fullRedraw(const PomodoroEngine &engine, int ldrValue) {
    _tft.fillScreen(COLOR_BG);

    // Forzar redibujado de todas las secciones
    _lastRemainingSec = 0xFFFFFFFF;
    _lastState = (PomodoroState)0xFF;
    _lastCycle = 0xFF;
    _lastFocusMin = 0xFF;
    _lastLdr = -1;
    _lastProgress = -1.0f;

    drawTopBar(engine, ldrValue);
    drawModeButtons(engine);
    drawCountdown(engine);
    drawProgressBar(engine);
    drawActionButtons(engine);
}

void UIManager::updateTimerDisplay(const PomodoroEngine &engine, int ldrValue) {
    // Si cambió el estado o el ciclo, redibujar elementos afectados
    if (engine.getState() != _lastState || engine.getCurrentCycle() != _lastCycle ||
        engine.getFocusMinutes() != _lastFocusMin || abs(ldrValue - _lastLdr) > 40) {
        drawTopBar(engine, ldrValue);
        drawModeButtons(engine);
        drawActionButtons(engine);
        _lastState = engine.getState();
        _lastCycle = engine.getCurrentCycle();
        _lastFocusMin = engine.getFocusMinutes();
        _lastLdr = ldrValue;
    }

    // Si cambió el segundo actual, redibujar reloj y barra
    if (engine.getRemainingSeconds() != _lastRemainingSec) {
        drawCountdown(engine);
        drawProgressBar(engine);
        _lastRemainingSec = engine.getRemainingSeconds();
    }
}

void UIManager::drawTopBar(const PomodoroEngine &engine, int ldrValue) {
    _tft.fillRect(0, 0, 320, 32, COLOR_BG);

    // Pastilla indicadora de estado
    uint16_t pillColor = COLOR_IDLE_BLUE;
    const char *stateLabel = "LISTO";

    if (engine.getState() == POMO_STATE_FOCUS) {
        pillColor = COLOR_FOCUS_RED;
        stateLabel = "ENFOQUE";
    } else if (engine.getState() == POMO_STATE_FOCUS_PAUSED) {
        pillColor = COLOR_PAUSE_AMBER;
        stateLabel = "PAUSA";
    } else if (engine.getState() == POMO_STATE_SHORT_BREAK) {
        pillColor = COLOR_BREAK_GREEN;
        stateLabel = "DESCANSO";
    } else if (engine.getState() == POMO_STATE_SHORT_BREAK_PAUSED) {
        pillColor = COLOR_PAUSE_AMBER;
        stateLabel = "PAUSA";
    } else if (engine.getState() == POMO_STATE_LONG_BREAK) {
        pillColor = COLOR_BREAK_GREEN;
        stateLabel = "D. LARGO";
    } else if (engine.getState() == POMO_STATE_LONG_BREAK_PAUSED) {
        pillColor = COLOR_PAUSE_AMBER;
        stateLabel = "PAUSA";
    }

    // Dibujar pastilla
    _tft.fillRoundRect(8, 5, 82, 22, 6, pillColor);
    _tft.setTextColor(TFT_WHITE, pillColor);
    _tft.drawCentreString(stateLabel, 49, 8, 2);

    // Ciclo actual
    _tft.setTextColor(TFT_WHITE, COLOR_BG);
    char cycleStr[16];
    snprintf(cycleStr, sizeof(cycleStr), "Ciclo %d/%d", engine.getCurrentCycle(), engine.getTotalCycles());
    _tft.drawString(cycleStr, 105, 8, 2);

    // Racha de Pomodoros completados hoy
    _tft.setTextColor(TFT_GOLD, COLOR_BG);
    char streakStr[16];
    snprintf(streakStr, sizeof(streakStr), "* %d", engine.getCompletedSessionsToday());
    _tft.drawString(streakStr, 205, 8, 2);

    // Sensor de Luz LDR
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    char ldrStr[16];
    snprintf(ldrStr, sizeof(ldrStr), "L: %d", ldrValue);
    _tft.drawRightString(ldrStr, 312, 8, 2);

    // Línea separadora sutil
    _tft.drawFastHLine(8, 31, 304, 0x2124);
}

void UIManager::drawModeButtons(const PomodoroEngine &engine) {
    bool is25 = (engine.getFocusMinutes() == 25);
    bool is50 = (engine.getFocusMinutes() == 50);

    // Botón 25 min (x: 50, y: 38, w: 100, h: 26)
    uint16_t bg25 = is25 ? 0x21E7 : COLOR_CARD;
    uint16_t border25 = is25 ? COLOR_IDLE_BLUE : 0x3186;
    _tft.fillRoundRect(50, 38, 100, 26, 6, bg25);
    _tft.drawRoundRect(50, 38, 100, 26, 6, border25);
    _tft.setTextColor(is25 ? TFT_WHITE : COLOR_TEXT_MUTED, bg25);
    _tft.drawCentreString("25 min", 100, 43, 2);

    // Botón 50 min (x: 170, y: 38, w: 100, h: 26)
    uint16_t bg50 = is50 ? 0x21E7 : COLOR_CARD;
    uint16_t border50 = is50 ? COLOR_IDLE_BLUE : 0x3186;
    _tft.fillRoundRect(170, 38, 100, 26, 6, bg50);
    _tft.drawRoundRect(170, 38, 100, 26, 6, border50);
    _tft.setTextColor(is50 ? TFT_WHITE : COLOR_TEXT_MUTED, bg50);
    _tft.drawCentreString("50 min", 220, 43, 2);
}

void UIManager::drawCountdown(const PomodoroEngine &engine) {
    uint32_t totalSec = engine.getRemainingSeconds();
    uint16_t mins = totalSec / 60;
    uint16_t secs = totalSec % 60;

    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", mins, secs);

    // Color del reloj según el modo
    uint16_t timerColor = TFT_WHITE;
    if (engine.getState() == POMO_STATE_FOCUS) {
        timerColor = TFT_WHITE;
    } else if (engine.isPaused()) {
        timerColor = COLOR_PAUSE_AMBER;
    } else if (engine.isBreak()) {
        timerColor = COLOR_BREAK_GREEN;
    }

    // Limpiar área del reloj
    _tft.fillRect(20, 72, 280, 60, COLOR_BG);

    // Dibujar reloj grande centrado
    _tft.setTextColor(timerColor, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 74, 7); // Fuente digital 7 grande
}

void UIManager::drawProgressBar(const PomodoroEngine &engine) {
    float progress = engine.getProgress();
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    const int barX = 30;
    const int barY = 148;
    const int barW = 260;
    const int barH = 10;

    uint16_t barFillColor = COLOR_IDLE_BLUE;
    if (engine.getState() == POMO_STATE_FOCUS) {
        barFillColor = COLOR_FOCUS_RED;
    } else if (engine.isBreak()) {
        barFillColor = COLOR_BREAK_GREEN;
    } else if (engine.isPaused()) {
        barFillColor = COLOR_PAUSE_AMBER;
    }

    // Fondo de la barra
    _tft.fillRoundRect(barX, barY, barW, barH, 5, 0x2124);

    // Relleno de progreso
    int fillW = (int)(progress * barW);
    if (fillW > 4) {
        _tft.fillRoundRect(barX, barY, fillW, barH, 5, barFillColor);
    }

    // Subtexto de estado motivacional
    _tft.fillRect(20, 164, 280, 18, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);

    const char *subtext = "Toca INICIAR para comenzar";
    if (engine.getState() == POMO_STATE_FOCUS) {
        subtext = "Modo Enfoque Profundo - No Molestar";
    } else if (engine.getState() == POMO_STATE_SHORT_BREAK) {
        subtext = "Descanso Corto - Estira y relaja la vista";
    } else if (engine.getState() == POMO_STATE_LONG_BREAK) {
        subtext = "Descanso Largo - Gran trabajo acumulado";
    } else if (engine.isPaused()) {
        subtext = "Sesion en Pausa";
    }

    _tft.drawCentreString(subtext, 160, 166, 2);
}

void UIManager::drawActionButtons(const PomodoroEngine &engine) {
    // 1. Botón REINICIAR (Izquierda: x: 15, y: 188, w: 80, h: 44)
    _tft.fillRoundRect(15, 188, 80, 44, 8, COLOR_CARD);
    _tft.drawRoundRect(15, 188, 80, 44, 8, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("REINICIAR", 55, 202, 2);

    // 2. Botón INICIAR / PAUSAR (Centro: x: 105, y: 186, w: 110, h: 48)
    uint16_t playBtnColor = COLOR_BREAK_GREEN;
    const char *playLabel = "INICIAR";

    if (engine.isRunning()) {
        playBtnColor = COLOR_PAUSE_AMBER;
        playLabel = "PAUSAR";
    } else if (engine.isPaused()) {
        playBtnColor = COLOR_IDLE_BLUE;
        playLabel = "CONTINUAR";
    }

    _tft.fillRoundRect(105, 186, 110, 48, 10, playBtnColor);
    _tft.drawRoundRect(105, 186, 110, 48, 10, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, playBtnColor);
    _tft.drawCentreString(playLabel, 160, 200, 2);

    // 3. Botón SALTAR (Derecha: x: 225, y: 188, w: 80, h: 44)
    _tft.fillRoundRect(225, 188, 80, 44, 8, COLOR_CARD);
    _tft.drawRoundRect(225, 188, 80, 44, 8, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("SALTAR", 265, 202, 2);
}

UIButtonAction UIManager::handleTouch(int x, int y) {
    // Selector 25 min (x: 45..155, y: 35..68)
    if (x >= 45 && x <= 155 && y >= 35 && y <= 68) {
        return UI_ACTION_SELECT_25;
    }

    // Selector 50 min (x: 165..275, y: 35..68)
    if (x >= 165 && x <= 275 && y >= 35 && y <= 68) {
        return UI_ACTION_SELECT_50;
    }

    // Botón REINICIAR (x: 10..98, y: 182..238)
    if (x >= 10 && x <= 98 && y >= 182 && y <= 238) {
        return UI_ACTION_RESET;
    }

    // Botón INICIAR / PAUSAR (x: 102..218, y: 182..238)
    if (x >= 102 && x <= 218 && y >= 182 && y <= 238) {
        return UI_ACTION_PLAY_PAUSE;
    }

    // Botón SALTAR (x: 222..310, y: 182..238)
    if (x >= 222 && x <= 310 && y >= 182 && y <= 238) {
        return UI_ACTION_SKIP;
    }

    return UI_ACTION_NONE;
}

