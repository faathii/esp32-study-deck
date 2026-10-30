#include "UIManager.h"
#include <time.h>

// Paleta de colores estilo Android / Material Dark
#define COLOR_BG          0x0821 // Fondo negro azulado profundo
#define COLOR_STATUS_BAR  0x0000 // Negro puro para barra de estado
#define COLOR_CARD        0x18E3 // Tarjeta grafito
#define COLOR_CARD_HL     0x2966 // Tarjeta activa

// Colores temáticos de las 6 aplicaciones
#define APP_COLOR_POMO    0xD164 // Rojo Coral
#define APP_COLOR_TODO    0x2A9C // Azul Índigo
#define APP_COLOR_FLASH   0xE4A2 // Naranja Ámbar
#define APP_COLOR_AGENDA  0x1CA7 // Verde Esmeralda
#define APP_COLOR_CONFIG  0x6A96 // Púrpura Ajustes
#define APP_COLOR_INFO    0x21E7 // Grafito Azulado Info

#define COLOR_FOCUS_RED   0xF986
#define COLOR_BREAK_GREEN 0x05E6
#define COLOR_PAUSE_AMBER 0xFDA0
#define COLOR_IDLE_BLUE   0x3BFF
#define COLOR_TEXT_MUTED  0x9CD3
#define COLOR_WIFI_OK     0x05E6
#define COLOR_WIFI_OFF    0x4208

UIManager::UIManager(TFT_eSPI &tft) : _tft(tft) {}

void UIManager::begin() {
    _tft.begin();
    _tft.setRotation(SCREEN_ROTATION);
    _tft.fillScreen(COLOR_BG);
}

// ============================================================================
// BARRA SUPERIOR UNIFICADA ESTILO ANDROID (STATUS BAR)
// ============================================================================

void UIManager::drawUnifiedStatusBar(const BatteryManager &battery, bool wifiConnected, 
                                     bool audioMuted, bool autoBrightness, 
                                     int streakCount, const char *appTitle) {
    // Fondo de la barra de estado (y: 0..23)
    _tft.fillRect(0, 0, 320, 24, COLOR_STATUS_BAR);

    // 1. ZONA IZQUIERDA: Reloj o Botón Volver
    if (appTitle == nullptr) {
        // En el Launcher Home: Reloj en vivo
        time_t now = time(nullptr);
        struct tm *t = localtime(&now);
        char timeStr[16];
        if (t->tm_year > 120) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", t->tm_hour, t->tm_min);
        } else {
            snprintf(timeStr, sizeof(timeStr), "Study Deck");
        }
        _tft.setTextColor(TFT_WHITE, COLOR_STATUS_BAR);
        _tft.drawString(timeStr, 8, 4, 2);
    } else {
        // Dentro de una App: Botón "< INICIO" y Título
        _tft.fillRoundRect(4, 2, 60, 20, 3, 0x18E3);
        _tft.drawRoundRect(4, 2, 60, 20, 3, TFT_WHITE);
        _tft.setTextColor(TFT_WHITE, 0x18E3);
        _tft.drawCentreString("< INICIO", 34, 6, 1);

        _tft.setTextColor(TFT_WHITE, COLOR_STATUS_BAR);
        _tft.drawString(appTitle, 68, 4, 2);
    }

    // 2. ZONA CENTRO: Racha e indicadores de sonido/brillo
    char streakStr[16];
    snprintf(streakStr, sizeof(streakStr), "* %d", streakCount);
    _tft.setTextColor(TFT_GOLD, COLOR_STATUS_BAR);
    _tft.drawString(streakStr, 142, 4, 2);

    if (audioMuted) {
        _tft.fillRoundRect(172, 3, 26, 18, 3, COLOR_FOCUS_RED);
        _tft.setTextColor(TFT_WHITE, COLOR_FOCUS_RED);
        _tft.drawCentreString("MUTE", 185, 6, 1);
    }

    if (autoBrightness) {
        _tft.fillRoundRect(202, 3, 24, 18, 3, 0x19E8);
        _tft.setTextColor(TFT_WHITE, 0x19E8);
        _tft.drawCentreString("AUTO", 214, 6, 1);
    }

    // 3. ZONA DERECHA: Wi-Fi y Batería
    // Pastilla Wi-Fi
    _tft.fillRoundRect(230, 3, 34, 18, 3, wifiConnected ? COLOR_WIFI_OK : COLOR_WIFI_OFF);
    _tft.setTextColor(TFT_WHITE, wifiConnected ? COLOR_WIFI_OK : COLOR_WIFI_OFF);
    _tft.drawCentreString(wifiConnected ? "WiFi" : "OFF", 247, 6, 1);

    // Porcentaje de batería en texto
    int pct = battery.getPercentage();
    char pctStr[8];
    snprintf(pctStr, sizeof(pctStr), "%d%%", pct);
    _tft.setTextColor(battery.getStatusColor(), COLOR_STATUS_BAR);
    _tft.drawRightString(pctStr, 298, 6, 1);

    // Icono gráfico de batería
    _tft.drawRoundRect(301, 6, 15, 11, 2, TFT_WHITE);
    _tft.fillRect(316, 9, 2, 5, TFT_WHITE); // Terminal positivo (+)
    int fillW = map(constrain(pct, 0, 100), 0, 100, 1, 11);
    _tft.fillRect(303, 8, fillW, 7, battery.getStatusColor());

    _tft.drawFastHLine(0, 24, 320, 0x2124);
}

// ============================================================================
// 1. PANTALLA DE INICIO (ANDROID LAUNCHER CON 6 CAJAS DE APLICACIÓN)
// ============================================================================

void UIManager::drawAppCard(int x, int y, int w, int h, uint16_t bgColor, const char *icon, const char *title, const char *subtitle) {
    _tft.fillRoundRect(x, y, w, h, 8, bgColor);
    _tft.drawRoundRect(x, y, w, h, 8, 0x4A69);

    _tft.setTextColor(TFT_WHITE, bgColor);
    _tft.drawCentreString(icon, x + w / 2, y + 10, 4);
    _tft.drawCentreString(title, x + w / 2, y + 42, 2);

    _tft.setTextColor(0xD6BA, bgColor);
    _tft.drawCentreString(subtitle, x + w / 2, y + 62, 1);
}

void UIManager::drawHomeScreen(const PomodoroEngine &engine, const TaskManager &tasks, 
                            const BatteryManager &battery, bool wifiConnected, 
                            bool audioMuted, bool autoBrightness) {
    _tft.fillScreen(COLOR_BG);

    // 1. Barra superior unificada
    drawUnifiedStatusBar(battery, wifiConnected, audioMuted, autoBrightness, 
                         engine.getCompletedSessionsToday(), nullptr);

    // 2. Encabezado "APLICACIONES"
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawString("APLICACIONES", 12, 30, 2);

    // Subtítulos dinámicos
    char pomoSub[16];
    if (engine.isRunning()) {
        uint32_t s = engine.getRemainingSeconds();
        snprintf(pomoSub, sizeof(pomoSub), "%02d:%02d", s / 60, s % 60);
    } else if (engine.isPaused()) {
        snprintf(pomoSub, sizeof(pomoSub), "En pausa");
    } else {
        snprintf(pomoSub, sizeof(pomoSub), "%d min", engine.getFocusMinutes());
    }

    char taskSub[16];
    snprintf(taskSub, sizeof(taskSub), "%d/%d hechas", tasks.getCompletedCount(), tasks.getTaskCount());

    char batSub[16];
    snprintf(batSub, sizeof(batSub), "%d%% - %.2fV", battery.getPercentage(), battery.getVoltage());

    // 3. Cuadrícula de Apps estilo Android (3 columnas x 2 filas)
    // FILA 1 (y: 48, h: 84)
    drawAppCard(10, 48, 94, 84, APP_COLOR_POMO, "[T]", "Pomodoro", pomoSub);
    drawAppCard(113, 48, 94, 84, APP_COLOR_TODO, "[V]", "Tareas", taskSub);
    drawAppCard(216, 48, 94, 84, APP_COLOR_FLASH, "[?]", "Fichas", "Repaso");

    // FILA 2 (y: 140, h: 84)
    drawAppCard(10, 140, 94, 84, APP_COLOR_AGENDA, "[#]", "Agenda", "Reloj/NTP");
    drawAppCard(113, 140, 94, 84, APP_COLOR_CONFIG, "[*]", "Ajustes", batSub);
    drawAppCard(216, 140, 94, 84, APP_COLOR_INFO, "[i]", "Info", "Deck OS");
}

int UIManager::handleHomeTouch(int x, int y) {
    // Toque en barra de estado superior (x: 220..320, y <= 24) abre Ajustes
    if (x >= 220 && x <= 320 && y <= 24) {
        return 5;
    }

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
        if (x >= 214 && x <= 312) return 6; // INFO & STATS
    }

    return 0;
}

// ============================================================================
// 2. APLICACIÓN POMODORO
// ============================================================================

void UIManager::drawPomodoroView(const PomodoroEngine &engine, const BatteryManager &battery, 
                                bool wifiConnected, bool audioMuted, bool autoBrightness) {
    _tft.fillScreen(COLOR_BG);

    drawUnifiedStatusBar(battery, wifiConnected, audioMuted, autoBrightness, 
                         engine.getCompletedSessionsToday(), "POMODORO");

    _lastRemainingSec = 0xFFFFFFFF;
    _lastState = (PomodoroState)0xFF;
    _lastCycle = 0xFF;
    _lastFocusMin = 0xFF;
    _lastBatteryPct = battery.getPercentage();
    _lastWifiStatus = wifiConnected;

    drawModeButtons(engine);
    drawCountdown(engine);
    drawProgressBar(engine);
    drawActionButtons(engine);
}

void UIManager::updatePomodoroTimer(const PomodoroEngine &engine, const BatteryManager &battery, bool wifiConnected) {
    if (engine.getState() != _lastState || engine.getCurrentCycle() != _lastCycle ||
        engine.getFocusMinutes() != _lastFocusMin || battery.getPercentage() != _lastBatteryPct ||
        wifiConnected != _lastWifiStatus) {
        
        drawUnifiedStatusBar(battery, wifiConnected, false, true, 
                             engine.getCompletedSessionsToday(), "POMODORO");
        drawModeButtons(engine);
        drawActionButtons(engine);

        _lastState = engine.getState();
        _lastCycle = engine.getCurrentCycle();
        _lastFocusMin = engine.getFocusMinutes();
        _lastBatteryPct = battery.getPercentage();
        _lastWifiStatus = wifiConnected;
    }

    if (engine.getRemainingSeconds() != _lastRemainingSec) {
        drawCountdown(engine);
        drawProgressBar(engine);
        _lastRemainingSec = engine.getRemainingSeconds();
    }
}

void UIManager::drawModeButtons(const PomodoroEngine &engine) {
    bool is25 = (engine.getFocusMinutes() == 25);
    bool is50 = (engine.getFocusMinutes() == 50);

    uint16_t bg25 = is25 ? 0x21E7 : COLOR_CARD;
    uint16_t border25 = is25 ? COLOR_IDLE_BLUE : 0x3186;
    _tft.fillRoundRect(50, 32, 100, 24, 4, bg25);
    _tft.drawRoundRect(50, 32, 100, 24, 4, border25);
    _tft.setTextColor(is25 ? TFT_WHITE : COLOR_TEXT_MUTED, bg25);
    _tft.drawCentreString("25 min", 100, 36, 2);

    uint16_t bg50 = is50 ? 0x21E7 : COLOR_CARD;
    uint16_t border50 = is50 ? COLOR_IDLE_BLUE : 0x3186;
    _tft.fillRoundRect(170, 32, 100, 24, 4, bg50);
    _tft.drawRoundRect(170, 32, 100, 24, 4, border50);
    _tft.setTextColor(is50 ? TFT_WHITE : COLOR_TEXT_MUTED, bg50);
    _tft.drawCentreString("50 min", 220, 36, 2);
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

    _tft.fillRect(20, 60, 280, 56, COLOR_BG);
    _tft.setTextColor(timerColor, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 62, 7);
}

void UIManager::drawProgressBar(const PomodoroEngine &engine) {
    float progress = engine.getProgress();
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    const int barX = 25;
    const int barY = 124;
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

    _tft.fillRect(20, 138, 280, 16, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);

    const char *subtext = "Toca INICIAR para comenzar";
    if (engine.getState() == POMO_STATE_FOCUS) subtext = "Modo Enfoque - No Molestar";
    else if (engine.getState() == POMO_STATE_SHORT_BREAK) subtext = "Descanso Corto - Estira la vista";
    else if (engine.getState() == POMO_STATE_LONG_BREAK) subtext = "Descanso Largo - Gran trabajo";
    else if (engine.isPaused()) subtext = "Sesion en Pausa";

    _tft.drawCentreString(subtext, 160, 139, 2);
}

void UIManager::drawActionButtons(const PomodoroEngine &engine) {
    _tft.fillRoundRect(15, 164, 85, 48, 8, COLOR_CARD);
    _tft.drawRoundRect(15, 164, 85, 48, 8, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("REINICIAR", 57, 180, 2);

    uint16_t playBtnColor = COLOR_BREAK_GREEN;
    const char *playLabel = "INICIAR";
    if (engine.isRunning()) {
        playBtnColor = COLOR_PAUSE_AMBER;
        playLabel = "PAUSAR";
    } else if (engine.isPaused()) {
        playBtnColor = COLOR_IDLE_BLUE;
        playLabel = "SEGUIR";
    }

    _tft.fillRoundRect(110, 162, 100, 52, 10, playBtnColor);
    _tft.drawRoundRect(110, 162, 100, 52, 10, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, playBtnColor);
    _tft.drawCentreString(playLabel, 160, 178, 2);

    _tft.fillRoundRect(220, 164, 85, 48, 8, COLOR_CARD);
    _tft.drawRoundRect(220, 164, 85, 48, 8, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("SALTAR", 262, 180, 2);
}

UIButtonAction UIManager::handlePomodoroTouch(int x, int y) {
    // 1. Botón [< INICIO] (x: 4..64, y: 0..24)
    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        return UI_ACTION_BACK_HOME;
    }

    // 2. Selectores 25m / 50m (y: 30..58)
    if (x >= 45 && x <= 155 && y >= 30 && y <= 58) return UI_ACTION_SELECT_25;
    if (x >= 165 && x <= 275 && y >= 30 && y <= 58) return UI_ACTION_SELECT_50;

    // 3. Botonera inferior (y: 160..220)
    if (x >= 10 && x <= 104 && y >= 160 && y <= 220) return UI_ACTION_RESET;
    if (x >= 106 && x <= 214 && y >= 160 && y <= 220) return UI_ACTION_PLAY_PAUSE;
    if (x >= 216 && x <= 310 && y >= 160 && y <= 220) return UI_ACTION_SKIP;

    return UI_ACTION_NONE;
}

// ============================================================================
// 3. APLICACIÓN TAREAS TO-DO
// ============================================================================

void UIManager::drawTodoView(const TaskManager &taskMgr, const BatteryManager &battery, bool wifiConnected) {
    _tft.fillScreen(COLOR_BG);
    char headStr[32];
    snprintf(headStr, sizeof(headStr), "TAREAS (%d/%d)", taskMgr.getCompletedCount(), taskMgr.getTaskCount());
    drawUnifiedStatusBar(battery, wifiConnected, false, true, 0, headStr);

    int count = taskMgr.getTaskCount();
    for (int i = 0; i < count; i++) {
        StudyTask task = taskMgr.getTask(i);
        int cardY = 32 + i * 36;

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

    _tft.fillRoundRect(85, 188, 150, 32, 6, 0x21E7);
    _tft.drawRoundRect(85, 188, 150, 32, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, 0x21E7);
    _tft.drawCentreString("Reiniciar Lista", 160, 196, 2);
}

int UIManager::handleTodoTouch(int x, int y, bool *outReset, bool *outBack) {
    *outReset = false;
    *outBack = false;

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return -1;
    }

    for (int i = 0; i < MAX_TASKS; i++) {
        int cardY = 32 + i * 36;
        if (x >= 12 && x <= 308 && y >= cardY && y <= cardY + 32) {
            return i;
        }
    }

    if (x >= 80 && x <= 240 && y >= 185 && y <= 225) {
        *outReset = true;
        return -1;
    }

    return -1;
}

// ============================================================================
// 4. APLICACIÓN FLASHCARDS
// ============================================================================

void UIManager::drawFlashcardsView(const FlashcardManager &flashcards, const BatteryManager &battery, bool wifiConnected) {
    _tft.fillScreen(COLOR_BG);
    Flashcard card = flashcards.getCurrentCard();

    char headStr[48];
    snprintf(headStr, sizeof(headStr), "FICHAS (%d/%d)", 
             flashcards.getCurrentIndex() + 1, flashcards.getTotalCards());
    drawUnifiedStatusBar(battery, wifiConnected, false, true, 0, headStr);

    bool flipped = flashcards.isFlipped();
    uint16_t cardBg = flipped ? 0x0A85 : COLOR_CARD;
    uint16_t cardBorder = flipped ? COLOR_BREAK_GREEN : COLOR_IDLE_BLUE;

    _tft.fillRoundRect(15, 32, 290, 126, 8, cardBg);
    _tft.drawRoundRect(15, 32, 290, 126, 8, cardBorder);

    if (!flipped) {
        _tft.setTextColor(COLOR_IDLE_BLUE, cardBg);
        _tft.drawCentreString("PREGUNTA", 160, 46, 2);
        _tft.setTextColor(TFT_WHITE, cardBg);
        _tft.drawCentreString(card.question, 160, 76, 4);
        _tft.setTextColor(COLOR_TEXT_MUTED, cardBg);
        _tft.drawCentreString("[ Toca para voltear la tarjeta ]", 160, 130, 2);
    } else {
        _tft.setTextColor(COLOR_BREAK_GREEN, cardBg);
        _tft.drawCentreString("RESPUESTA", 160, 42, 2);
        _tft.setTextColor(TFT_WHITE, cardBg);

        int lineY = 68;
        int lastPos = 0, nextPos = 0;
        while ((nextPos = card.answer.indexOf('\n', lastPos)) != -1) {
            String line = card.answer.substring(lastPos, nextPos);
            _tft.drawCentreString(line, 160, lineY, 2);
            lineY += 18;
            lastPos = nextPos + 1;
        }
        if (lastPos < (int)card.answer.length()) {
            _tft.drawCentreString(card.answer.substring(lastPos), 160, lineY, 2);
        }
    }

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

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return true;
    }

    if (x >= 15 && x <= 305 && y >= 30 && y <= 160) {
        *outFlipped = true;
        return true;
    }

    if (x >= 20 && x <= 152 && y >= 168 && y <= 228) {
        *outHard = true;
        return true;
    }

    if (x >= 168 && x <= 298 && y >= 168 && y <= 228) {
        *outEasy = true;
        return true;
    }

    return false;
}

// ============================================================================
// 5. APLICACIÓN AGENDA & RELOJ & EXÁMENES
// ============================================================================

void UIManager::drawAgendaView(CalendarManager &calendar, const WiFiConfig &wifi, const BatteryManager &battery) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifi.isConnected(), false, true, 0, "AGENDA & RELOJ");

    updateAgendaClock(calendar);

    // Tarjeta Cuenta Regresiva de Exámenes
    String examTitle;
    String examCountdown = calendar.getNextDeadlineCountdown(examTitle);

    _tft.fillRoundRect(12, 122, 296, 44, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 122, 296, 44, 6, COLOR_PAUSE_AMBER);
    _tft.setTextColor(COLOR_PAUSE_AMBER, COLOR_CARD);
    _tft.drawString("Proximo Examen / Hito:", 22, 128, 2);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString(examTitle, 22, 146, 2);
    _tft.setTextColor(COLOR_BREAK_GREEN, COLOR_CARD);
    _tft.drawRightString(examCountdown, 298, 146, 2);

    // Tarjeta Estado de Red y Sincronización
    _tft.fillRoundRect(12, 172, 296, 48, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 172, 296, 48, 6, 0x3186);
    if (wifi.isConnected()) {
        _tft.setTextColor(COLOR_BREAK_GREEN, COLOR_CARD);
        _tft.drawString("Wi-Fi Sincronizado (NTP Activo)", 22, 180, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        char netInfo[48];
        snprintf(netInfo, sizeof(netInfo), "IP: %s | RSSI: %d dBm", wifi.getLocalIP().c_str(), wifi.getRSSI());
        _tft.drawString(netInfo, 22, 198, 2);
    } else {
        _tft.setTextColor(COLOR_PAUSE_AMBER, COLOR_CARD);
        _tft.drawString("Modo Offline (Reloj interno)", 22, 180, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawString("Conecta Wi-Fi en Ajustes para hora exacta.", 22, 198, 2);
    }
}

void UIManager::updateAgendaClock(CalendarManager &calendar) {
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

    _tft.fillRect(20, 32, 280, 52, COLOR_BG);
    _tft.setTextColor(TFT_WHITE, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 36, 7);

    _tft.fillRect(20, 90, 280, 20, COLOR_BG);
    _tft.setTextColor(TFT_GOLD, COLOR_BG);
    _tft.drawCentreString(dateStr, 160, 92, 2);
}

bool UIManager::handleAgendaTouch(int x, int y) {
    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        return true;
    }
    return false;
}

// ============================================================================
// 6. APLICACIÓN AJUSTES DEL SISTEMA
// ============================================================================

void UIManager::drawSettingsView(const WiFiConfig &wifi, const BatteryManager &battery, 
                                int brightness, bool autoBrightness, bool soundMuted) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifi.isConnected(), soundMuted, autoBrightness, 0, "AJUSTES");

    // 1. Tarjeta Monitor de Batería (y: 30..72)
    _tft.fillRoundRect(12, 30, 296, 44, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 30, 296, 44, 6, battery.getStatusColor());
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Bateria LiPo", 22, 36, 2);

    char batInfo[48];
    snprintf(batInfo, sizeof(batInfo), "%d%% (%.2f V) - %s", 
             battery.getPercentage(), battery.getVoltage(), 
             battery.isCharging() ? "Cargando/USB" : "En descarga");
    _tft.setTextColor(battery.getStatusColor(), COLOR_CARD);
    _tft.drawString(batInfo, 22, 54, 2);

    // 2. Tarjeta Brillo de Pantalla (y: 78..124)
    _tft.fillRoundRect(12, 78, 296, 46, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 78, 296, 46, 6, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Brillo Pantalla", 22, 84, 2);

    char briStr[16];
    snprintf(briStr, sizeof(briStr), "%d%%", (brightness * 100) / 255);
    _tft.setTextColor(COLOR_IDLE_BLUE, COLOR_CARD);
    _tft.drawString(briStr, 22, 102, 2);

    _tft.fillRoundRect(120, 84, 36, 32, 4, 0x2A6B);
    _tft.setTextColor(TFT_WHITE, 0x2A6B);
    _tft.drawCentreString("-", 138, 90, 4);

    _tft.fillRoundRect(164, 84, 36, 32, 4, 0x2A6B);
    _tft.setTextColor(TFT_WHITE, 0x2A6B);
    _tft.drawCentreString("+", 182, 90, 4);

    uint16_t ldrBg = autoBrightness ? COLOR_BREAK_GREEN : 0x10A2;
    _tft.fillRoundRect(210, 84, 90, 32, 4, ldrBg);
    _tft.setTextColor(TFT_WHITE, ldrBg);
    _tft.drawCentreString(autoBrightness ? "Auto ON" : "Auto OFF", 255, 92, 2);

    // 3. Tarjeta Sonido (y: 128..168)
    _tft.fillRoundRect(12, 128, 296, 42, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 128, 296, 42, 6, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Alertas Sonoras", 22, 140, 2);

    uint16_t soundBg = soundMuted ? COLOR_FOCUS_RED : COLOR_BREAK_GREEN;
    _tft.fillRoundRect(190, 133, 110, 32, 4, soundBg);
    _tft.setTextColor(TFT_WHITE, soundBg);
    _tft.drawCentreString(soundMuted ? "SILENCIO" : "SONIDO ON", 245, 141, 2);

    // 4. Tarjeta Wi-Fi (y: 174..220)
    _tft.fillRoundRect(12, 174, 296, 46, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 174, 296, 46, 6, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Red Wi-Fi", 22, 180, 2);

    String netInfo = wifi.isConnected() ? ("Conectado: " + wifi.getSSID()) : "Desconectado";
    _tft.setTextColor(wifi.isConnected() ? COLOR_BREAK_GREEN : COLOR_PAUSE_AMBER, COLOR_CARD);
    _tft.drawString(netInfo, 22, 200, 1);

    _tft.fillRoundRect(200, 180, 100, 34, 4, COLOR_IDLE_BLUE);
    _tft.setTextColor(TFT_WHITE, COLOR_IDLE_BLUE);
    _tft.drawCentreString("CONFIG", 250, 188, 2);
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

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return 0;
    }

    // Control de brillo (y: 78..124)
    if (y >= 78 && y <= 124) {
        if (x >= 115 && x <= 158) { *outBriDown = true; return 1; }
        if (x >= 160 && x <= 204) { *outBriUp = true; return 1; }
        if (x >= 208 && x <= 305) { *outToggleAuto = true; return 1; }
    }

    // Toggle Sonido (x: 185..305, y: 128..168)
    if (x >= 185 && x <= 305 && y >= 128 && y <= 168) {
        *outToggleMute = true;
        return 2;
    }

    // Botón Configurar Wi-Fi (x: 195..305, y: 174..220)
    if (x >= 195 && x <= 305 && y >= 174 && y <= 220) {
        *outOpenWifi = true;
        return 3;
    }

    return 0;
}

// ============================================================================
// 7. APLICACIÓN INFO & ESTADÍSTICAS & MICROSD
// ============================================================================

void UIManager::drawInfoView(const BatteryManager &battery, StorageManager &storage, 
                            const PomodoroEngine &pomodoro, bool wifiConnected) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifiConnected, false, true, 
                         pomodoro.getCompletedSessionsToday(), "INFO & STATS");

    // Tarjeta Estadísticas de Estudio
    _tft.fillRoundRect(12, 32, 296, 56, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 32, 296, 56, 6, COLOR_BREAK_GREEN);
    _tft.setTextColor(COLOR_BREAK_GREEN, COLOR_CARD);
    _tft.drawString("Estadisticas de Estudio", 22, 38, 2);

    char statBuf[64];
    int totalMins = storage.getTotalStudyMinutes();
    snprintf(statBuf, sizeof(statBuf), "Sesiones Hoy: %d | Total en SD: %d min", 
             pomodoro.getCompletedSessionsToday(), totalMins);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString(statBuf, 22, 60, 2);

    // Tarjeta Almacenamiento MicroSD
    _tft.fillRoundRect(12, 94, 296, 50, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 94, 296, 50, 6, storage.isAvailable() ? COLOR_IDLE_BLUE : 0x3186);
    _tft.setTextColor(COLOR_IDLE_BLUE, COLOR_CARD);
    _tft.drawString("Tarjeta MicroSD", 22, 100, 2);

    char sdBuf[48];
    if (storage.isAvailable()) {
        snprintf(sdBuf, sizeof(sdBuf), "Insertada: %llu MB | /study_log.csv OK", storage.getCardSizeMB());
        _tft.setTextColor(COLOR_BREAK_GREEN, COLOR_CARD);
    } else {
        snprintf(sdBuf, sizeof(sdBuf), "No insertada (Registro en memoria NVS)");
        _tft.setTextColor(COLOR_PAUSE_AMBER, COLOR_CARD);
    }
    _tft.drawString(sdBuf, 22, 120, 2);

    // Tarjeta Hardware y Firmware
    _tft.fillRoundRect(12, 150, 296, 68, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 150, 296, 68, 6, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Hardware: Guition ESP32-2432S024C", 22, 156, 2);

    char hwBuf[64];
    snprintf(hwBuf, sizeof(hwBuf), "CPU: 240MHz Dual Core | Free RAM: %d KB", ESP.getFreeHeap() / 1024);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawString(hwBuf, 22, 176, 1);

    _tft.setTextColor(COLOR_IDLE_BLUE, COLOR_CARD);
    _tft.drawString("Study Deck OS v0.3.0 - Codigo Abierto MIT", 22, 194, 2);
}

bool UIManager::handleInfoTouch(int x, int y) {
    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        return true;
    }
    return false;
}

// ============================================================================
// SUBVISTAS DE CONFIGURACIÓN WI-FI
// ============================================================================

void UIManager::drawWiFiScanView(const WiFiConfig &wifi, const BatteryManager &battery, bool scanning, int page) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifi.isConnected(), false, true, 0, "REDES WI-FI");

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
            int cardY = 32 + i * 36;

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

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return -1;
    }

    for (int i = 0; i < 4; i++) {
        int cardY = 32 + i * 36;
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
