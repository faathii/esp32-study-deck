#include "UIManager.h"
#include <time.h>

// Paleta de colores estilo Android / Material Design
#define COLOR_BG          0x0821 // Fondo negro azulado profundo
#define COLOR_STATUS_BAR  0x0000 // Negro puro barra de estado
#define COLOR_CARD        0x18E3 // Tarjeta grafito
#define COLOR_CARD_HL     0x2966 // Tarjeta activa

// Colores de Apps
#define APP_COLOR_POMO    0xD164 // Rojo Coral
#define APP_COLOR_TODO    0x2A9C // Azul Índigo
#define APP_COLOR_FLASH   0xE4A2 // Naranja Ámbar
#define APP_COLOR_AGENDA  0x1CA7 // Verde Esmeralda
#define APP_COLOR_CONFIG  0x6A96 // Púrpura Ajustes
#define APP_COLOR_STATS   0x10A2 // Grafito neutro

#define COLOR_FOCUS_RED   0xF986
#define COLOR_BREAK_GREEN 0x05E6
#define COLOR_PAUSE_AMBER 0xFDA0
#define COLOR_IDLE_BLUE   0x3BFF
#define COLOR_TEXT_MUTED  0x9CD3
#define COLOR_WIFI_OK     0x2E6A
#define COLOR_WIFI_OFF    0x4208

UIManager::UIManager(TFT_eSPI &tft) : _tft(tft) {}

void UIManager::begin() {
    _tft.begin();
    _tft.setRotation(SCREEN_ROTATION);
    _tft.fillScreen(COLOR_BG);
}

// ============================================================================
// 1. PANTALLA DE INICIO (ANDROID LAUNCHER CON CAJAS DE APP)
// ============================================================================

void UIManager::drawAppCard(int x, int y, int w, int h, uint16_t bgColor, const char *icon, const char *title, const char *subtitle) {
    // Tarjeta exterior redondeada
    _tft.fillRoundRect(x, y, w, h, 8, bgColor);
    _tft.drawRoundRect(x, y, w, h, 8, 0x4A69);

    // Icono centrado superior
    _tft.setTextColor(TFT_WHITE, bgColor);
    _tft.drawCentreString(icon, x + w / 2, y + 10, 4);

    // Título de la App
    _tft.drawCentreString(title, x + w / 2, y + 42, 2);

    // Subtítulo
    _tft.setTextColor(0xD6BA, bgColor);
    _tft.drawCentreString(subtitle, x + w / 2, y + 62, 1);
}

void UIManager::drawHomeScreen(const PomodoroEngine &engine, const TaskManager &tasks, bool wifiConnected, int ldrValue) {
    _tft.fillScreen(COLOR_BG);

    // 1. Barra de estado superior estilo Android (y: 0..24)
    _tft.fillRect(0, 0, 320, 24, COLOR_STATUS_BAR);

    // Hora o título a la izquierda
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    char timeStr[16];
    if (t->tm_year > 120) {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d", t->tm_hour, t->tm_min);
    } else {
        snprintf(timeStr, sizeof(timeStr), "Study Deck");
    }
    _tft.setTextColor(TFT_WHITE, COLOR_STATUS_BAR);
    _tft.drawString(timeStr, 12, 5, 2);

    // Racha Pomodoro en el centro
    char streakStr[16];
    snprintf(streakStr, sizeof(streakStr), "* %d Pomos", engine.getCompletedSessionsToday());
    _tft.setTextColor(TFT_GOLD, COLOR_STATUS_BAR);
    _tft.drawCentreString(streakStr, 160, 5, 2);

    // Wi-Fi en la esquina derecha
    _tft.fillRoundRect(248, 3, 62, 18, 4, wifiConnected ? COLOR_WIFI_OK : COLOR_WIFI_OFF);
    _tft.setTextColor(TFT_WHITE, wifiConnected ? COLOR_WIFI_OK : COLOR_WIFI_OFF);
    _tft.drawCentreString(wifiConnected ? "WiFi ON" : "WiFi OFF", 279, 5, 1);

    _tft.drawFastHLine(0, 24, 320, 0x2124);

    // 2. Título de bienvenida (y: 26..46)
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawString("APLICACIONES", 12, 30, 2);

    // 3. Cuadrícula de Apps estilo Android (3 columnas x 2 filas)
    // Subtítulo dinámico para Pomodoro
    char pomoSub[16];
    if (engine.isRunning()) {
        uint32_t s = engine.getRemainingSeconds();
        snprintf(pomoSub, sizeof(pomoSub), "%02d:%02d", s / 60, s % 60);
    } else if (engine.isPaused()) {
        snprintf(pomoSub, sizeof(pomoSub), "En pausa");
    } else {
        snprintf(pomoSub, sizeof(pomoSub), "%d min", engine.getFocusMinutes());
    }

    // Subtítulo dinámico para Tareas
    char taskSub[16];
    snprintf(taskSub, sizeof(taskSub), "%d/%d hechas", tasks.getCompletedCount(), tasks.getTaskCount());

    // FILA 1 (y: 48, h: 84)
    drawAppCard(10, 48, 94, 84, APP_COLOR_POMO, "[T]", "Pomodoro", pomoSub);
    drawAppCard(113, 48, 94, 84, APP_COLOR_TODO, "[V]", "Tareas", taskSub);
    drawAppCard(216, 48, 94, 84, APP_COLOR_FLASH, "[?]", "Fichas", "Repaso");

    // FILA 2 (y: 140, h: 84)
    drawAppCard(10, 140, 94, 84, APP_COLOR_AGENDA, "[#]", "Agenda", "Reloj/NTP");
    drawAppCard(113, 140, 94, 84, APP_COLOR_CONFIG, "[*]", "Ajustes", "WiFi/Luz");
    drawAppCard(216, 140, 94, 84, APP_COLOR_STATS, "[+]", "Info", "Deck OS");
}

int UIManager::handleHomeTouch(int x, int y) {
    // FILA 1 (y: 45..135)
    if (y >= 45 && y <= 135) {
        if (x >= 8 && x <= 106) return 1;   // POMODORO
        if (x >= 111 && x <= 209) return 2; // TAREAS
        if (x >= 214 && x <= 312) return 3; // FLASHCARDS
    }

    // FILA 2 (y: 138..228)
    if (y >= 138 && y <= 228) {
        if (x >= 8 && x <= 106) return 4;   // AGENDA
        if (x >= 111 && x <= 209) return 5; // AJUSTES
        if (x >= 214 && x <= 312) return 5; // INFO / AJUSTES
    }

    // Toque en el botón Wi-Fi de la barra de estado superior
    if (x >= 240 && x <= 315 && y <= 24) {
        return 5; // Lleva a Ajustes
    }

    return 0; // Ninguna app
}

// Cabecera común de aplicación con botón de volver al Home estilo Android
void UIManager::drawHeaderWithBack(const char *title, uint16_t barColor) {
    _tft.fillRect(0, 0, 320, 30, barColor);

    // Botón [< INICIO] estilo Android
    _tft.fillRoundRect(6, 4, 78, 22, 4, 0x0821);
    _tft.drawRoundRect(6, 4, 78, 22, 4, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, 0x0821);
    _tft.drawCentreString("< INICIO", 45, 7, 2);

    // Título de la App centrado
    _tft.setTextColor(TFT_WHITE, barColor);
    _tft.drawCentreString(title, 190, 7, 2);

    _tft.drawFastHLine(0, 30, 320, 0x3186);
}

// ============================================================================
// 2. APLICACIÓN POMODORO
// ============================================================================

void UIManager::drawPomodoroView(const PomodoroEngine &engine, int ldrValue, bool wifiConnected) {
    _tft.fillScreen(COLOR_BG);

    _lastRemainingSec = 0xFFFFFFFF;
    _lastState = (PomodoroState)0xFF;
    _lastCycle = 0xFF;
    _lastFocusMin = 0xFF;
    _lastLdr = -1;
    _lastWifiStatus = !wifiConnected;

    drawTopBar(engine, ldrValue, wifiConnected);
    drawModeButtons(engine);
    drawCountdown(engine);
    drawProgressBar(engine);
    drawActionButtons(engine);
}

void UIManager::updatePomodoroTimer(const PomodoroEngine &engine, int ldrValue, bool wifiConnected) {
    if (engine.getState() != _lastState || engine.getCurrentCycle() != _lastCycle ||
        engine.getFocusMinutes() != _lastFocusMin || abs(ldrValue - _lastLdr) > 50 ||
        wifiConnected != _lastWifiStatus) {
        drawTopBar(engine, ldrValue, wifiConnected);
        drawModeButtons(engine);
        drawActionButtons(engine);
        _lastState = engine.getState();
        _lastCycle = engine.getCurrentCycle();
        _lastFocusMin = engine.getFocusMinutes();
        _lastLdr = ldrValue;
        _lastWifiStatus = wifiConnected;
    }

    if (engine.getRemainingSeconds() != _lastRemainingSec) {
        drawCountdown(engine);
        drawProgressBar(engine);
        _lastRemainingSec = engine.getRemainingSeconds();
    }
}

void UIManager::drawTopBar(const PomodoroEngine &engine, int ldrValue, bool wifiConnected) {
    _tft.fillRect(0, 0, 320, 32, COLOR_BG);

    // Botón [< INICIO]
    _tft.fillRoundRect(6, 5, 75, 22, 4, 0x18E3);
    _tft.drawRoundRect(6, 5, 75, 22, 4, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, 0x18E3);
    _tft.drawCentreString("< INICIO", 43, 8, 2);

    // Pastilla de estado
    uint16_t pillColor = COLOR_IDLE_BLUE;
    const char *stateLabel = "LISTO";
    if (engine.getState() == POMO_STATE_FOCUS) {
        pillColor = COLOR_FOCUS_RED;
        stateLabel = "ENFOQUE";
    } else if (engine.isPaused()) {
        pillColor = COLOR_PAUSE_AMBER;
        stateLabel = "PAUSA";
    } else if (engine.isBreak()) {
        pillColor = COLOR_BREAK_GREEN;
        stateLabel = (engine.getState() == POMO_STATE_LONG_BREAK) ? "D. LARGO" : "DESCANSO";
    }

    _tft.fillRoundRect(86, 5, 76, 22, 4, pillColor);
    _tft.setTextColor(TFT_WHITE, pillColor);
    _tft.drawCentreString(stateLabel, 124, 8, 2);

    // Ciclo
    _tft.setTextColor(TFT_WHITE, COLOR_BG);
    char cycleStr[16];
    snprintf(cycleStr, sizeof(cycleStr), "C: %d/%d", engine.getCurrentCycle(), engine.getTotalCycles());
    _tft.drawString(cycleStr, 170, 8, 2);

    // Racha
    _tft.setTextColor(TFT_GOLD, COLOR_BG);
    char streakStr[16];
    snprintf(streakStr, sizeof(streakStr), "* %d", engine.getCompletedSessionsToday());
    _tft.drawString(streakStr, 235, 8, 2);

    _tft.drawFastHLine(6, 31, 308, 0x2124);
}

void UIManager::drawModeButtons(const PomodoroEngine &engine) {
    bool is25 = (engine.getFocusMinutes() == 25);
    bool is50 = (engine.getFocusMinutes() == 50);

    uint16_t bg25 = is25 ? 0x21E7 : COLOR_CARD;
    uint16_t border25 = is25 ? COLOR_IDLE_BLUE : 0x3186;
    _tft.fillRoundRect(50, 36, 100, 26, 5, bg25);
    _tft.drawRoundRect(50, 36, 100, 26, 5, border25);
    _tft.setTextColor(is25 ? TFT_WHITE : COLOR_TEXT_MUTED, bg25);
    _tft.drawCentreString("25 min", 100, 41, 2);

    uint16_t bg50 = is50 ? 0x21E7 : COLOR_CARD;
    uint16_t border50 = is50 ? COLOR_IDLE_BLUE : 0x3186;
    _tft.fillRoundRect(170, 36, 100, 26, 5, bg50);
    _tft.drawRoundRect(170, 36, 100, 26, 5, border50);
    _tft.setTextColor(is50 ? TFT_WHITE : COLOR_TEXT_MUTED, bg50);
    _tft.drawCentreString("50 min", 220, 41, 2);
}

void UIManager::drawCountdown(const PomodoroEngine &engine) {
    uint32_t totalSec = engine.getRemainingSeconds();
    uint16_t mins = totalSec / 60;
    uint16_t secs = totalSec % 60;

    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", mins, secs);

    uint16_t timerColor = TFT_WHITE;
    if (engine.getState() == POMO_STATE_FOCUS) timerColor = TFT_WHITE;
    else if (engine.isPaused()) timerColor = COLOR_PAUSE_AMBER;
    else if (engine.isBreak()) timerColor = COLOR_BREAK_GREEN;

    _tft.fillRect(20, 68, 280, 56, COLOR_BG);
    _tft.setTextColor(timerColor, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 70, 7);
}

void UIManager::drawProgressBar(const PomodoroEngine &engine) {
    float progress = engine.getProgress();
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    const int barX = 25;
    const int barY = 132;
    const int barW = 270;
    const int barH = 8;

    uint16_t barFillColor = COLOR_IDLE_BLUE;
    if (engine.getState() == POMO_STATE_FOCUS) barFillColor = COLOR_FOCUS_RED;
    else if (engine.isBreak()) barFillColor = COLOR_BREAK_GREEN;
    else if (engine.isPaused()) barFillColor = COLOR_PAUSE_AMBER;

    _tft.fillRoundRect(barX, barY, barW, barH, 4, 0x2124);

    int fillW = (int)(progress * barW);
    if (fillW > 4) {
        _tft.fillRoundRect(barX, barY, fillW, barH, 4, barFillColor);
    }

    _tft.fillRect(20, 146, 280, 16, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);

    const char *subtext = "Toca INICIAR para comenzar";
    if (engine.getState() == POMO_STATE_FOCUS) subtext = "Modo Enfoque - No Molestar";
    else if (engine.getState() == POMO_STATE_SHORT_BREAK) subtext = "Descanso Corto - Estira la vista";
    else if (engine.getState() == POMO_STATE_LONG_BREAK) subtext = "Descanso Largo - Gran trabajo";
    else if (engine.isPaused()) subtext = "Sesion en Pausa";

    _tft.drawCentreString(subtext, 160, 147, 2);
}

void UIManager::drawActionButtons(const PomodoroEngine &engine) {
    _tft.fillRoundRect(15, 174, 85, 48, 8, COLOR_CARD);
    _tft.drawRoundRect(15, 174, 85, 48, 8, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("REINICIAR", 57, 190, 2);

    uint16_t playBtnColor = COLOR_BREAK_GREEN;
    const char *playLabel = "INICIAR";
    if (engine.isRunning()) {
        playBtnColor = COLOR_PAUSE_AMBER;
        playLabel = "PAUSAR";
    } else if (engine.isPaused()) {
        playBtnColor = COLOR_IDLE_BLUE;
        playLabel = "SEGUIR";
    }

    _tft.fillRoundRect(110, 172, 100, 52, 10, playBtnColor);
    _tft.drawRoundRect(110, 172, 100, 52, 10, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, playBtnColor);
    _tft.drawCentreString(playLabel, 160, 188, 2);

    _tft.fillRoundRect(220, 174, 85, 48, 8, COLOR_CARD);
    _tft.drawRoundRect(220, 174, 85, 48, 8, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("SALTAR", 262, 190, 2);
}

UIButtonAction UIManager::handlePomodoroTouch(int x, int y) {
    // 1. Botón [< INICIO] (x: 4..85, y: 0..32)
    if (x >= 4 && x <= 85 && y >= 0 && y <= 32) {
        return UI_ACTION_BACK_HOME;
    }

    // 2. Selectores 25m / 50m (y: 34..64)
    if (x >= 45 && x <= 155 && y >= 34 && y <= 64) return UI_ACTION_SELECT_25;
    if (x >= 165 && x <= 275 && y >= 34 && y <= 64) return UI_ACTION_SELECT_50;

    // 3. Botonera inferior (y: 168..230)
    if (x >= 10 && x <= 104 && y >= 168 && y <= 230) return UI_ACTION_RESET;
    if (x >= 106 && x <= 214 && y >= 168 && y <= 230) return UI_ACTION_PLAY_PAUSE;
    if (x >= 216 && x <= 310 && y >= 168 && y <= 230) return UI_ACTION_SKIP;

    return UI_ACTION_NONE;
}

// ============================================================================
// 3. APLICACIÓN TAREAS TO-DO
// ============================================================================

void UIManager::drawTodoView(const TaskManager &taskMgr) {
    _tft.fillScreen(COLOR_BG);
    char headStr[32];
    snprintf(headStr, sizeof(headStr), "TAREAS (%d/%d)", taskMgr.getCompletedCount(), taskMgr.getTaskCount());
    drawHeaderWithBack(headStr, APP_COLOR_TODO);

    int count = taskMgr.getTaskCount();
    for (int i = 0; i < count; i++) {
        StudyTask task = taskMgr.getTask(i);
        int cardY = 38 + i * 36;

        _tft.fillRoundRect(12, cardY, 296, 32, 6, COLOR_CARD);
        _tft.drawRoundRect(12, cardY, 296, 32, 6, task.completed ? COLOR_BREAK_GREEN : 0x3186);

        _tft.fillRoundRect(20, cardY + 5, 22, 22, 4, task.completed ? COLOR_BREAK_GREEN : 0x10A2);
        _tft.drawRoundRect(20, cardY + 5, 22, 22, 4, TFT_WHITE);
        if (task.completed) {
            _tft.setTextColor(TFT_WHITE, COLOR_BREAK_GREEN);
            _tft.drawCentreString("V", 31, cardY + 8, 2);
        }

        String txt = task.text;
        if (txt.length() > 27) txt = txt.substring(0, 25) + "...";
        _tft.setTextColor(task.completed ? COLOR_TEXT_MUTED : TFT_WHITE, COLOR_CARD);
        _tft.drawString(txt, 50, cardY + 8, 2);

        if (task.completed) {
            _tft.drawFastHLine(50, cardY + 16, txt.length() * 7, COLOR_TEXT_MUTED);
        }
    }

    // Botón reiniciar tareas
    _tft.fillRoundRect(85, 192, 150, 32, 6, 0x21E7);
    _tft.drawRoundRect(85, 192, 150, 32, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, 0x21E7);
    _tft.drawCentreString("Reiniciar Lista", 160, 200, 2);
}

int UIManager::handleTodoTouch(int x, int y, bool *outReset, bool *outBack) {
    *outReset = false;
    *outBack = false;

    // Botón [< INICIO] (x: 4..85, y: 0..30)
    if (x >= 4 && x <= 85 && y >= 0 && y <= 30) {
        *outBack = true;
        return -1;
    }

    // Toque en tarea (y: 38..185)
    for (int i = 0; i < MAX_TASKS; i++) {
        int cardY = 38 + i * 36;
        if (x >= 12 && x <= 308 && y >= cardY && y <= cardY + 32) {
            return i;
        }
    }

    // Botón reiniciar lista (y: 190..230)
    if (x >= 80 && x <= 240 && y >= 190 && y <= 230) {
        *outReset = true;
        return -1;
    }

    return -1;
}

// ============================================================================
// 4. APLICACIÓN FLASHCARDS
// ============================================================================

void UIManager::drawFlashcardsView(const FlashcardManager &flashcards) {
    _tft.fillScreen(COLOR_BG);
    Flashcard card = flashcards.getCurrentCard();

    char headStr[48];
    snprintf(headStr, sizeof(headStr), "FICHAS (%d/%d) %s", 
             flashcards.getCurrentIndex() + 1, flashcards.getTotalCards(), card.category.c_str());
    drawHeaderWithBack(headStr, APP_COLOR_FLASH);

    bool flipped = flashcards.isFlipped();
    uint16_t cardBg = flipped ? 0x0A85 : COLOR_CARD;
    uint16_t cardBorder = flipped ? COLOR_BREAK_GREEN : COLOR_IDLE_BLUE;

    _tft.fillRoundRect(15, 38, 290, 120, 8, cardBg);
    _tft.drawRoundRect(15, 38, 290, 120, 8, cardBorder);

    if (!flipped) {
        _tft.setTextColor(COLOR_IDLE_BLUE, cardBg);
        _tft.drawCentreString("PREGUNTA", 160, 52, 2);
        _tft.setTextColor(TFT_WHITE, cardBg);
        _tft.drawCentreString(card.question, 160, 80, 4);
        _tft.setTextColor(COLOR_TEXT_MUTED, cardBg);
        _tft.drawCentreString("[ Toca para voltear la tarjeta ]", 160, 128, 2);
    } else {
        _tft.setTextColor(COLOR_BREAK_GREEN, cardBg);
        _tft.drawCentreString("RESPUESTA", 160, 48, 2);
        _tft.setTextColor(TFT_WHITE, cardBg);

        int lineY = 74;
        int lastPos = 0, nextPos = 0;
        while ((nextPos = card.answer.indexOf('\n', lastPos)) != -1) {
            String line = card.answer.substring(lastPos, nextPos);
            _tft.drawCentreString(line, 160, lineY, 2);
            lineY += 20;
            lastPos = nextPos + 1;
        }
        if (lastPos < (int)card.answer.length()) {
            _tft.drawCentreString(card.answer.substring(lastPos), 160, lineY, 2);
        }
    }

    // Botones de respuesta
    _tft.fillRoundRect(25, 172, 125, 48, 8, COLOR_FOCUS_RED);
    _tft.drawRoundRect(25, 172, 125, 48, 8, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, COLOR_FOCUS_RED);
    _tft.drawCentreString("DIFICIL", 87, 188, 2);

    _tft.fillRoundRect(170, 172, 125, 48, 8, COLOR_BREAK_GREEN);
    _tft.drawRoundRect(170, 172, 125, 48, 8, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, COLOR_BREAK_GREEN);
    _tft.drawCentreString("FACIL", 232, 188, 2);
}

bool UIManager::handleFlashcardTouch(int x, int y, bool *outFlipped, bool *outEasy, bool *outHard, bool *outBack) {
    *outFlipped = false;
    *outEasy = false;
    *outHard = false;
    *outBack = false;

    // Botón [< INICIO]
    if (x >= 4 && x <= 85 && y >= 0 && y <= 30) {
        *outBack = true;
        return true;
    }

    // Tarjeta central voltear (y: 38..160)
    if (x >= 15 && x <= 305 && y >= 38 && y <= 160) {
        *outFlipped = true;
        return true;
    }

    // Botón DIFÍCIL (y: 168..228)
    if (x >= 20 && x <= 152 && y >= 168 && y <= 228) {
        *outHard = true;
        return true;
    }

    // Botón FÁCIL (y: 168..228)
    if (x >= 168 && x <= 298 && y >= 168 && y <= 228) {
        *outEasy = true;
        return true;
    }

    return false;
}

// ============================================================================
// 5. APLICACIÓN AGENDA & RELOJ
// ============================================================================

void UIManager::drawAgendaView(const WiFiConfig &wifi) {
    _tft.fillScreen(COLOR_BG);
    drawHeaderWithBack("AGENDA & RELOJ", APP_COLOR_AGENDA);

    updateAgendaClock();

    _tft.fillRoundRect(15, 120, 290, 44, 6, COLOR_CARD);
    _tft.drawRoundRect(15, 120, 290, 44, 6, COLOR_IDLE_BLUE);
    _tft.setTextColor(COLOR_IDLE_BLUE, COLOR_CARD);
    _tft.drawString("Proximo evento de estudio:", 25, 126, 2);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Bloque de Enfoque y Estudio", 25, 144, 2);

    _tft.fillRoundRect(15, 174, 290, 48, 6, COLOR_CARD);
    _tft.drawRoundRect(15, 174, 290, 48, 6, 0x3186);
    if (wifi.isConnected()) {
        _tft.setTextColor(COLOR_BREAK_GREEN, COLOR_CARD);
        _tft.drawString("Wi-Fi Conectado (NTP Activo)", 25, 182, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        char netInfo[48];
        snprintf(netInfo, sizeof(netInfo), "IP: %s (%d dBm)", wifi.getLocalIP().c_str(), wifi.getRSSI());
        _tft.drawString(netInfo, 25, 200, 2);
    } else {
        _tft.setTextColor(COLOR_PAUSE_AMBER, COLOR_CARD);
        _tft.drawString("Wi-Fi Desconectado (Modo Offline)", 25, 182, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawString("Configura la red en la App Ajustes.", 25, 200, 2);
    }
}

void UIManager::updateAgendaClock() {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    char timeStr[16];
    char dateStr[32];

    if (t->tm_year > 120) {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
        const char *dias[7] = { "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado" };
        const char *meses[12] = { "Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic" };
        snprintf(dateStr, sizeof(dateStr), "%s, %d de %s", dias[t->tm_wday], t->tm_mday, meses[t->tm_mon]);
    } else {
        snprintf(timeStr, sizeof(timeStr), "--:--:--");
        snprintf(dateStr, sizeof(dateStr), "Conecta a Wi-Fi para hora real");
    }

    _tft.fillRect(20, 38, 280, 52, COLOR_BG);
    _tft.setTextColor(TFT_WHITE, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 42, 7);

    _tft.fillRect(20, 94, 280, 20, COLOR_BG);
    _tft.setTextColor(TFT_GOLD, COLOR_BG);
    _tft.drawCentreString(dateStr, 160, 96, 2);
}

bool UIManager::handleAgendaTouch(int x, int y) {
    if (x >= 4 && x <= 85 && y >= 0 && y <= 30) {
        return true; // Volver al Home
    }
    return false;
}

// ============================================================================
// 6. APLICACIÓN AJUSTES DEL SISTEMA
// ============================================================================

void UIManager::drawSettingsView(const WiFiConfig &wifi, int brightness, bool autoBrightness, bool soundMuted) {
    _tft.fillScreen(COLOR_BG);
    drawHeaderWithBack("AJUSTES", APP_COLOR_CONFIG);

    // 1. Tarjeta Brillo de Pantalla (y: 38..88)
    _tft.fillRoundRect(12, 38, 296, 48, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 38, 296, 48, 6, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Brillo Pantalla", 22, 46, 2);

    char briStr[16];
    snprintf(briStr, sizeof(briStr), "%d%%", (brightness * 100) / 255);
    _tft.setTextColor(COLOR_IDLE_BLUE, COLOR_CARD);
    _tft.drawString(briStr, 22, 64, 2);

    // Botones [-] y [+]
    _tft.fillRoundRect(120, 46, 36, 32, 4, 0x2A6B);
    _tft.setTextColor(TFT_WHITE, 0x2A6B);
    _tft.drawCentreString("-", 138, 52, 4);

    _tft.fillRoundRect(164, 46, 36, 32, 4, 0x2A6B);
    _tft.setTextColor(TFT_WHITE, 0x2A6B);
    _tft.drawCentreString("+", 182, 52, 4);

    // Botón Auto LDR
    uint16_t ldrBg = autoBrightness ? COLOR_BREAK_GREEN : 0x10A2;
    _tft.fillRoundRect(210, 46, 90, 32, 4, ldrBg);
    _tft.setTextColor(TFT_WHITE, ldrBg);
    _tft.drawCentreString(autoBrightness ? "Auto ON" : "Auto OFF", 255, 54, 2);

    // 2. Tarjeta Sonido (y: 92..138)
    _tft.fillRoundRect(12, 92, 296, 44, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 92, 296, 44, 6, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Alertas Sonoras", 22, 104, 2);

    uint16_t soundBg = soundMuted ? COLOR_FOCUS_RED : COLOR_BREAK_GREEN;
    _tft.fillRoundRect(190, 98, 110, 32, 4, soundBg);
    _tft.setTextColor(TFT_WHITE, soundBg);
    _tft.drawCentreString(soundMuted ? "SILENCIO" : "SONIDO ON", 245, 106, 2);

    // 3. Tarjeta Wi-Fi (y: 142..192)
    _tft.fillRoundRect(12, 142, 296, 48, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 142, 296, 48, 6, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Red Wi-Fi", 22, 148, 2);

    String netInfo = wifi.isConnected() ? ("Conectado a: " + wifi.getSSID()) : "Desconectado";
    _tft.setTextColor(wifi.isConnected() ? COLOR_BREAK_GREEN : COLOR_PAUSE_AMBER, COLOR_CARD);
    _tft.drawString(netInfo, 22, 168, 1);

    // Botón Configurar Wi-Fi
    _tft.fillRoundRect(200, 150, 100, 32, 4, COLOR_IDLE_BLUE);
    _tft.setTextColor(TFT_WHITE, COLOR_IDLE_BLUE);
    _tft.drawCentreString("CONFIG", 250, 158, 2);

    // 4. Versión y firma (y: 202)
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawCentreString("Study Deck OS v0.2.0 - Codigo Abierto", 160, 210, 2);
}

int UIManager::handleSettingsTouch(int x, int y, bool *outToggleAuto, bool *outBriDown, bool *outBriUp, 
                                   bool *outToggleMute, bool *outOpenWifi, bool *outClearWifi, bool *outBack) {
    *outToggleAuto = false;
    *outBriDown = false;
    *outBriUp = false;
    *outToggleMute = false;
    *outOpenWifi = false;
    *outClearWifi = false;
    *outBack = false;

    // Botón [< INICIO]
    if (x >= 4 && x <= 85 && y >= 0 && y <= 30) {
        *outBack = true;
        return 0;
    }

    // Control de brillo (y: 42..82)
    if (y >= 42 && y <= 82) {
        if (x >= 115 && x <= 158) { *outBriDown = true; return 1; }
        if (x >= 160 && x <= 204) { *outBriUp = true; return 1; }
        if (x >= 208 && x <= 305) { *outToggleAuto = true; return 1; }
    }

    // Toggle Sonido (x: 185..305, y: 94..134)
    if (x >= 185 && x <= 305 && y >= 94 && y <= 134) {
        *outToggleMute = true;
        return 2;
    }

    // Botón Configurar Wi-Fi (x: 195..305, y: 145..188)
    if (x >= 195 && x <= 305 && y >= 145 && y <= 188) {
        *outOpenWifi = true;
        return 3;
    }

    return 0;
}

// ============================================================================
// SUBVISTAS DE CONFIGURACIÓN WI-FI
// ============================================================================

void UIManager::drawWiFiScanView(const WiFiConfig &wifi, bool scanning, int page) {
    _tft.fillScreen(COLOR_BG);
    drawHeaderWithBack("REDES WI-FI", COLOR_IDLE_BLUE);

    if (scanning) {
        _tft.setTextColor(COLOR_IDLE_BLUE, COLOR_BG);
        _tft.drawCentreString("Buscando redes en el aire...", 160, 100, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
        _tft.drawCentreString("Espera un momento", 160, 125, 2);
        return;
    }

    int totalNetworks = wifi.getNetworkCount();
    if (totalNetworks == 0) {
        _tft.setTextColor(COLOR_PAUSE_AMBER, COLOR_BG);
        _tft.drawCentreString("No se han encontrado redes.", 160, 100, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
        _tft.drawCentreString("Toca 'ESCANEAR' para reintentar.", 160, 125, 2);
    } else {
        int startIndex = page * 4;
        for (int i = 0; i < 4; i++) {
            int netIdx = startIndex + i;
            if (netIdx >= totalNetworks) break;

            DiscoveredNetwork net = wifi.getNetwork(netIdx);
            int cardY = 36 + i * 36;

            _tft.fillRoundRect(12, cardY, 296, 32, 6, COLOR_CARD);
            _tft.drawRoundRect(12, cardY, 296, 32, 6, 0x3186);

            String name = net.ssid;
            if (name.length() > 20) {
                name = name.substring(0, 18) + "...";
            }
            _tft.setTextColor(TFT_WHITE, COLOR_CARD);
            _tft.drawString(name, 22, cardY + 8, 2);

            String info = (net.isEncrypted ? "[*] " : "[Abierta] ");
            info += String(net.rssi) + " dBm";
            _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
            _tft.drawRightString(info, 298, cardY + 8, 2);
        }
    }

    _tft.fillRoundRect(12, 194, 85, 36, 6, 0xC8A2);
    _tft.drawRoundRect(12, 194, 85, 36, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, 0xC8A2);
    _tft.drawCentreString("VOLVER", 54, 204, 2);

    if (totalNetworks > 4) {
        _tft.fillRoundRect(105, 194, 95, 36, 6, COLOR_CARD);
        _tft.drawRoundRect(105, 194, 95, 36, 6, 0x3186);
        _tft.setTextColor(TFT_WHITE, COLOR_CARD);
        char pagStr[16];
        snprintf(pagStr, sizeof(pagStr), "Pag %d/%d", page + 1, (totalNetworks + 3) / 4);
        _tft.drawCentreString(pagStr, 152, 204, 2);
    }

    _tft.fillRoundRect(208, 194, 100, 36, 6, COLOR_IDLE_BLUE);
    _tft.drawRoundRect(208, 194, 100, 36, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, COLOR_IDLE_BLUE);
    _tft.drawCentreString("ESCANEAR", 258, 204, 2);
}

int UIManager::handleWiFiScanTouch(int x, int y, int netCount, int page, bool *outRescan, bool *outBack, bool *outNextPage) {
    *outRescan = false;
    *outBack = false;
    *outNextPage = false;

    if (x >= 4 && x <= 85 && y >= 0 && y <= 30) {
        *outBack = true;
        return -1;
    }

    for (int i = 0; i < 4; i++) {
        int cardY = 36 + i * 36;
        if (x >= 12 && x <= 308 && y >= cardY && y <= cardY + 32) {
            int selectedIndex = page * 4 + i;
            if (selectedIndex < netCount) {
                return selectedIndex;
            }
        }
    }

    if (x >= 10 && x <= 98 && y >= 190 && y <= 236) {
        *outBack = true;
        return -1;
    }

    if (x >= 102 && x <= 202 && y >= 190 && y <= 236) {
        *outNextPage = true;
        return -1;
    }

    if (x >= 205 && x <= 312 && y >= 190 && y <= 236) {
        *outRescan = true;
        return -1;
    }

    return -1;
}

void UIManager::drawWiFiConnecting(const String &ssid) {
    _tft.fillScreen(COLOR_BG);

    _tft.fillRoundRect(30, 45, 260, 150, 10, COLOR_CARD);
    _tft.drawRoundRect(30, 45, 260, 150, 10, COLOR_IDLE_BLUE);

    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("Conectando a la red...", 160, 65, 2);

    _tft.setTextColor(COLOR_IDLE_BLUE, COLOR_CARD);
    _tft.drawCentreString(ssid, 160, 95, 4);

    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawCentreString("Comprobando contrasena...", 160, 140, 2);
}

void UIManager::drawWiFiResult(bool success, const String &info) {
    _tft.fillScreen(COLOR_BG);

    uint16_t borderColor = success ? COLOR_BREAK_GREEN : COLOR_FOCUS_RED;
    _tft.fillRoundRect(30, 40, 260, 160, 10, COLOR_CARD);
    _tft.drawRoundRect(30, 40, 260, 160, 10, borderColor);

    if (success) {
        _tft.setTextColor(COLOR_BREAK_GREEN, COLOR_CARD);
        _tft.drawCentreString("CONEXION EXITOSA", 160, 58, 4);

        _tft.setTextColor(TFT_WHITE, COLOR_CARD);
        _tft.drawCentreString("IP asignada:", 160, 100, 2);
        _tft.setTextColor(COLOR_IDLE_BLUE, COLOR_CARD);
        _tft.drawCentreString(info, 160, 122, 2);

        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawCentreString("Guardada en NVS. Volviendo...", 160, 160, 2);
    } else {
        _tft.setTextColor(COLOR_FOCUS_RED, COLOR_CARD);
        _tft.drawCentreString("FALLO DE CONEXION", 160, 58, 4);

        _tft.setTextColor(TFT_WHITE, COLOR_CARD);
        _tft.drawCentreString("No se pudo conectar.", 160, 105, 2);

        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawCentreString(info, 160, 130, 2);

        _tft.fillRoundRect(85, 155, 150, 32, 6, COLOR_IDLE_BLUE);
        _tft.setTextColor(TFT_WHITE, COLOR_IDLE_BLUE);
        _tft.drawCentreString("REINTENTAR", 160, 162, 2);
    }
}
