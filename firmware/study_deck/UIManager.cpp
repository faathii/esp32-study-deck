#include "UIManager.h"
#include <time.h>

// Paleta de colores
#define COLOR_BG          0x0841 // Pizarra oscuro
#define COLOR_CARD        0x18E3 // Tarjeta elevada
#define COLOR_CARD_ACTIVE 0x2A6B // Tarjeta activa
#define COLOR_FOCUS_RED   0xF986 // Rojo brillante
#define COLOR_BREAK_GREEN 0x05E6 // Verde esmeralda
#define COLOR_PAUSE_AMBER 0xFDA0 // Ámbar
#define COLOR_IDLE_BLUE   0x3BFF // Azul cian
#define COLOR_TEXT_MUTED  0x9CD3 // Gris suave
#define COLOR_WIFI_OK     0x2E6A // Verde Wi-Fi
#define COLOR_WIFI_OFF    0x4208 // Gris Wi-Fi

UIManager::UIManager(TFT_eSPI &tft) : _tft(tft) {}

void UIManager::begin() {
    _tft.begin();
    _tft.setRotation(SCREEN_ROTATION);
    _tft.fillScreen(COLOR_BG);
}

// ============================================================================
// BARRA DE NAVEGACIÓN INFERIOR (COMÚN A TODAS LAS VISTAS)
// ============================================================================

void UIManager::drawBottomTabs(AppView currentTab) {
    const char *tabNames[4] = { "Pomo", "Tareas", "Fichas", "Agenda" };
    int tabX[4] = { 4, 82, 160, 238 };
    int tabW[4] = { 74, 74, 74, 78 };

    _tft.fillRect(0, 204, 320, 36, 0x0821);
    _tft.drawFastHLine(0, 204, 320, 0x3186);

    for (int i = 0; i < 4; i++) {
        bool isActive = (currentTab == (AppView)i);
        uint16_t bg = isActive ? COLOR_IDLE_BLUE : 0x10A2;
        uint16_t txt = isActive ? TFT_WHITE : COLOR_TEXT_MUTED;

        _tft.fillRoundRect(tabX[i], 208, tabW[i], 28, 4, bg);
        if (isActive) {
            _tft.drawRoundRect(tabX[i], 208, tabW[i], 28, 4, TFT_WHITE);
        }
        _tft.setTextColor(txt, bg);
        _tft.drawCentreString(tabNames[i], tabX[i] + tabW[i] / 2, 214, 2);
    }
}

int UIManager::handleBottomTabsTouch(int x, int y) {
    if (y >= 204 && y <= 240) {
        if (x >= 2 && x <= 80) return 0;   // POMODORO
        if (x >= 81 && x <= 158) return 1; // TODO
        if (x >= 159 && x <= 236) return 2;// FLASHCARDS
        if (x >= 237 && x <= 318) return 3;// AGENDA
    }
    return -1;
}

// ============================================================================
// VISTA 1: POMODORO & CONCENTRACIÓN
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
    drawBottomTabs(VIEW_POMODORO);
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
    _tft.fillRect(0, 0, 320, 30, COLOR_BG);

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

    _tft.fillRoundRect(8, 4, 76, 22, 5, pillColor);
    _tft.setTextColor(TFT_WHITE, pillColor);
    _tft.drawCentreString(stateLabel, 46, 7, 2);

    _tft.setTextColor(TFT_WHITE, COLOR_BG);
    char cycleStr[16];
    snprintf(cycleStr, sizeof(cycleStr), "Ciclo %d/%d", engine.getCurrentCycle(), engine.getTotalCycles());
    _tft.drawString(cycleStr, 94, 7, 2);

    _tft.setTextColor(TFT_GOLD, COLOR_BG);
    char streakStr[16];
    snprintf(streakStr, sizeof(streakStr), "* %d", engine.getCompletedSessionsToday());
    _tft.drawString(streakStr, 172, 7, 2);

    uint16_t wifiBg = wifiConnected ? COLOR_WIFI_OK : COLOR_WIFI_OFF;
    _tft.fillRoundRect(232, 4, 80, 22, 5, wifiBg);
    _tft.drawRoundRect(232, 4, 80, 22, 5, wifiConnected ? TFT_GREEN : 0x5AEB);
    _tft.setTextColor(TFT_WHITE, wifiBg);
    _tft.drawCentreString(wifiConnected ? "WiFi: ON" : "WiFi +", 272, 7, 2);

    _tft.drawFastHLine(8, 29, 304, 0x2124);
}

void UIManager::drawModeButtons(const PomodoroEngine &engine) {
    bool is25 = (engine.getFocusMinutes() == 25);
    bool is50 = (engine.getFocusMinutes() == 50);

    uint16_t bg25 = is25 ? 0x21E7 : COLOR_CARD;
    uint16_t border25 = is25 ? COLOR_IDLE_BLUE : 0x3186;
    _tft.fillRoundRect(50, 33, 100, 24, 5, bg25);
    _tft.drawRoundRect(50, 33, 100, 24, 5, border25);
    _tft.setTextColor(is25 ? TFT_WHITE : COLOR_TEXT_MUTED, bg25);
    _tft.drawCentreString("25 min", 100, 37, 2);

    uint16_t bg50 = is50 ? 0x21E7 : COLOR_CARD;
    uint16_t border50 = is50 ? COLOR_IDLE_BLUE : 0x3186;
    _tft.fillRoundRect(170, 33, 100, 24, 5, bg50);
    _tft.drawRoundRect(170, 33, 100, 24, 5, border50);
    _tft.setTextColor(is50 ? TFT_WHITE : COLOR_TEXT_MUTED, bg50);
    _tft.drawCentreString("50 min", 220, 37, 2);
}

void UIManager::drawCountdown(const PomodoroEngine &engine) {
    uint32_t totalSec = engine.getRemainingSeconds();
    uint16_t mins = totalSec / 60;
    uint16_t secs = totalSec % 60;

    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", mins, secs);

    uint16_t timerColor = TFT_WHITE;
    if (engine.getState() == POMO_STATE_FOCUS) {
        timerColor = TFT_WHITE;
    } else if (engine.isPaused()) {
        timerColor = COLOR_PAUSE_AMBER;
    } else if (engine.isBreak()) {
        timerColor = COLOR_BREAK_GREEN;
    }

    _tft.fillRect(20, 60, 280, 52, COLOR_BG);
    _tft.setTextColor(timerColor, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 62, 7);
}

void UIManager::drawProgressBar(const PomodoroEngine &engine) {
    float progress = engine.getProgress();
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    const int barX = 30;
    const int barY = 118;
    const int barW = 260;
    const int barH = 8;

    uint16_t barFillColor = COLOR_IDLE_BLUE;
    if (engine.getState() == POMO_STATE_FOCUS) {
        barFillColor = COLOR_FOCUS_RED;
    } else if (engine.isBreak()) {
        barFillColor = COLOR_BREAK_GREEN;
    } else if (engine.isPaused()) {
        barFillColor = COLOR_PAUSE_AMBER;
    }

    _tft.fillRoundRect(barX, barY, barW, barH, 4, 0x2124);

    int fillW = (int)(progress * barW);
    if (fillW > 4) {
        _tft.fillRoundRect(barX, barY, fillW, barH, 4, barFillColor);
    }

    _tft.fillRect(20, 130, 280, 16, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);

    const char *subtext = "Toca INICIAR para comenzar";
    if (engine.getState() == POMO_STATE_FOCUS) {
        subtext = "Modo Enfoque Profundo - No Molestar";
    } else if (engine.getState() == POMO_STATE_SHORT_BREAK) {
        subtext = "Descanso Corto - Estira la vista";
    } else if (engine.getState() == POMO_STATE_LONG_BREAK) {
        subtext = "Descanso Largo - Gran sesion";
    } else if (engine.isPaused()) {
        subtext = "Sesion en Pausa";
    }

    _tft.drawCentreString(subtext, 160, 131, 2);
}

void UIManager::drawActionButtons(const PomodoroEngine &engine) {
    _tft.fillRoundRect(15, 152, 80, 42, 6, COLOR_CARD);
    _tft.drawRoundRect(15, 152, 80, 42, 6, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("REINICIAR", 55, 165, 2);

    uint16_t playBtnColor = COLOR_BREAK_GREEN;
    const char *playLabel = "INICIAR";

    if (engine.isRunning()) {
        playBtnColor = COLOR_PAUSE_AMBER;
        playLabel = "PAUSAR";
    } else if (engine.isPaused()) {
        playBtnColor = COLOR_IDLE_BLUE;
        playLabel = "CONTINUAR";
    }

    _tft.fillRoundRect(105, 150, 110, 46, 8, playBtnColor);
    _tft.drawRoundRect(105, 150, 110, 46, 8, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, playBtnColor);
    _tft.drawCentreString(playLabel, 160, 163, 2);

    _tft.fillRoundRect(225, 152, 80, 42, 6, COLOR_CARD);
    _tft.drawRoundRect(225, 152, 80, 42, 6, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("SALTAR", 265, 165, 2);
}

UIButtonAction UIManager::handlePomodoroTouch(int x, int y) {
    if (x >= 230 && x <= 318 && y >= 0 && y <= 30) {
        return UI_ACTION_OPEN_WIFI;
    }
    if (x >= 45 && x <= 155 && y >= 30 && y <= 58) {
        return UI_ACTION_SELECT_25;
    }
    if (x >= 165 && x <= 275 && y >= 30 && y <= 58) {
        return UI_ACTION_SELECT_50;
    }
    if (x >= 10 && x <= 98 && y >= 148 && y <= 198) {
        return UI_ACTION_RESET;
    }
    if (x >= 102 && x <= 218 && y >= 146 && y <= 200) {
        return UI_ACTION_PLAY_PAUSE;
    }
    if (x >= 222 && x <= 310 && y >= 148 && y <= 198) {
        return UI_ACTION_SKIP;
    }
    return UI_ACTION_NONE;
}

// ============================================================================
// VISTA 2: LISTA DE TAREAS TO-DO INTERACTIVA
// ============================================================================

void UIManager::drawTodoView(const TaskManager &taskMgr) {
    _tft.fillScreen(COLOR_BG);

    // Cabecera
    _tft.fillRect(0, 0, 320, 30, 0x1082);
    _tft.setTextColor(TFT_WHITE, 0x1082);
    char headStr[32];
    snprintf(headStr, sizeof(headStr), "MIS TAREAS (%d/%d)", taskMgr.getCompletedCount(), taskMgr.getTaskCount());
    _tft.drawCentreString(headStr, 160, 7, 2);
    _tft.drawFastHLine(0, 30, 320, 0x3186);

    // Dibujar tarjetas de tareas
    int count = taskMgr.getTaskCount();
    for (int i = 0; i < count; i++) {
        StudyTask task = taskMgr.getTask(i);
        int cardY = 36 + i * 35;

        _tft.fillRoundRect(12, cardY, 296, 31, 6, COLOR_CARD);
        _tft.drawRoundRect(12, cardY, 296, 31, 6, task.completed ? COLOR_BREAK_GREEN : 0x3186);

        // Checkbox táctil
        _tft.fillRoundRect(20, cardY + 5, 22, 22, 4, task.completed ? COLOR_BREAK_GREEN : 0x10A2);
        _tft.drawRoundRect(20, cardY + 5, 22, 22, 4, TFT_WHITE);
        if (task.completed) {
            _tft.setTextColor(TFT_WHITE, COLOR_BREAK_GREEN);
            _tft.drawCentreString("V", 31, cardY + 8, 2);
        }

        // Texto de la tarea
        String txt = task.text;
        if (txt.length() > 27) txt = txt.substring(0, 25) + "...";
        _tft.setTextColor(task.completed ? COLOR_TEXT_MUTED : TFT_WHITE, COLOR_CARD);
        _tft.drawString(txt, 50, cardY + 8, 2);

        // Línea de tachado si está completa
        if (task.completed) {
            _tft.drawFastHLine(50, cardY + 16, txt.length() * 7, COLOR_TEXT_MUTED);
        }
    }

    // Botón Restaurar tareas
    _tft.fillRoundRect(85, 178, 150, 24, 4, 0x21E7);
    _tft.setTextColor(COLOR_TEXT_MUTED, 0x21E7);
    _tft.drawCentreString("Reiniciar Tareas", 160, 182, 2);

    drawBottomTabs(VIEW_TODO);
}

int UIManager::handleTodoTouch(int x, int y, bool *outReset) {
    *outReset = false;

    // Tocar una tarea (y: 36..175)
    for (int i = 0; i < MAX_TASKS; i++) {
        int cardY = 36 + i * 35;
        if (x >= 12 && x <= 308 && y >= cardY && y <= cardY + 31) {
            return i;
        }
    }

    // Tocar reiniciar tareas (y: 175..202)
    if (x >= 80 && x <= 240 && y >= 175 && y <= 202) {
        *outReset = true;
        return -1;
    }

    return -1;
}

// ============================================================================
// VISTA 3: FLASHCARDS DE REPASO ACTIVO
// ============================================================================

void UIManager::drawFlashcardsView(const FlashcardManager &flashcards) {
    _tft.fillScreen(COLOR_BG);

    Flashcard card = flashcards.getCurrentCard();

    // Cabecera
    _tft.fillRect(0, 0, 320, 30, 0x1082);
    _tft.setTextColor(TFT_WHITE, 0x1082);
    char headStr[48];
    snprintf(headStr, sizeof(headStr), "FLASHCARD %d/%d - %s", 
             flashcards.getCurrentIndex() + 1, flashcards.getTotalCards(), card.category.c_str());
    _tft.drawCentreString(headStr, 160, 7, 2);
    _tft.drawFastHLine(0, 30, 320, 0x3186);

    // Tarjeta central (x: 15, y: 36, w: 290, h: 108)
    bool flipped = flashcards.isFlipped();
    uint16_t cardBg = flipped ? 0x0A85 : COLOR_CARD;
    uint16_t cardBorder = flipped ? COLOR_BREAK_GREEN : COLOR_IDLE_BLUE;

    _tft.fillRoundRect(15, 36, 290, 108, 8, cardBg);
    _tft.drawRoundRect(15, 36, 290, 108, 8, cardBorder);

    if (!flipped) {
        _tft.setTextColor(COLOR_IDLE_BLUE, cardBg);
        _tft.drawCentreString("PREGUNTA / CONCEPTO", 160, 48, 2);

        _tft.setTextColor(TFT_WHITE, cardBg);
        _tft.drawCentreString(card.question, 160, 74, 4);

        _tft.setTextColor(COLOR_TEXT_MUTED, cardBg);
        _tft.drawCentreString("[ Toca la tarjeta para voltear ]", 160, 118, 2);
    } else {
        _tft.setTextColor(COLOR_BREAK_GREEN, cardBg);
        _tft.drawCentreString("RESPUESTA", 160, 44, 2);

        _tft.setTextColor(TFT_WHITE, cardBg);
        // Dibujar multilínea de respuesta
        int lineY = 68;
        int lastPos = 0;
        int nextPos = 0;
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
    _tft.fillRoundRect(25, 154, 125, 42, 6, COLOR_FOCUS_RED);
    _tft.drawRoundRect(25, 154, 125, 42, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, COLOR_FOCUS_RED);
    _tft.drawCentreString("DIFICIL", 87, 167, 2);

    _tft.fillRoundRect(170, 154, 125, 42, 6, COLOR_BREAK_GREEN);
    _tft.drawRoundRect(170, 154, 125, 42, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, COLOR_BREAK_GREEN);
    _tft.drawCentreString("FACIL", 232, 167, 2);

    drawBottomTabs(VIEW_FLASH);
}

bool UIManager::handleFlashcardTouch(int x, int y, bool *outFlipped, bool *outEasy, bool *outHard) {
    *outFlipped = false;
    *outEasy = false;
    *outHard = false;

    // Tocar tarjeta central para voltear (y: 36..145)
    if (x >= 15 && x <= 305 && y >= 36 && y <= 145) {
        *outFlipped = true;
        return true;
    }

    // Botón DIFÍCIL (x: 20..152, y: 152..198)
    if (x >= 20 && x <= 152 && y >= 152 && y <= 198) {
        *outHard = true;
        return true;
    }

    // Botón FÁCIL (x: 168..298, y: 152..198)
    if (x >= 168 && x <= 298 && y >= 152 && y <= 198) {
        *outEasy = true;
        return true;
    }

    return false;
}

// ============================================================================
// VISTA 4: AGENDA Y RELOJ NTP
// ============================================================================

void UIManager::drawAgendaView(const WiFiConfig &wifi) {
    _tft.fillScreen(COLOR_BG);

    // Cabecera
    _tft.fillRect(0, 0, 320, 30, 0x1082);
    _tft.setTextColor(TFT_WHITE, 0x1082);
    _tft.drawCentreString("RELOJ NTP & AGENDA", 160, 7, 2);
    _tft.drawFastHLine(0, 30, 320, 0x3186);

    updateAgendaClock();

    // Tarjeta del próximo evento
    _tft.fillRoundRect(15, 110, 290, 42, 6, COLOR_CARD);
    _tft.drawRoundRect(15, 110, 290, 42, 6, COLOR_IDLE_BLUE);
    _tft.setTextColor(COLOR_IDLE_BLUE, COLOR_CARD);
    _tft.drawString("Proximo evento:", 25, 116, 2);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawString("Bloque de Enfoque y Estudio", 25, 132, 2);

    // Tarjeta estado de red
    _tft.fillRoundRect(15, 158, 290, 42, 6, COLOR_CARD);
    _tft.drawRoundRect(15, 158, 290, 42, 6, 0x3186);
    if (wifi.isConnected()) {
        _tft.setTextColor(COLOR_BREAK_GREEN, COLOR_CARD);
        _tft.drawString("Wi-Fi Conectado (NTP Activo)", 25, 164, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        char netInfo[48];
        snprintf(netInfo, sizeof(netInfo), "IP: %s | %d dBm", wifi.getLocalIP().c_str(), wifi.getRSSI());
        _tft.drawString(netInfo, 25, 180, 2);
    } else {
        _tft.setTextColor(COLOR_PAUSE_AMBER, COLOR_CARD);
        _tft.drawString("Wi-Fi Desconectado (Modo Offline)", 25, 164, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawString("Toca 'WiFi +' en Pomodoro para conectar.", 25, 180, 2);
    }

    drawBottomTabs(VIEW_AGENDA);
}

void UIManager::updateAgendaClock() {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    char timeStr[16];
    char dateStr[32];

    if (t->tm_year > 120) { // Si el año es >= 2021 (NTP sincronizado)
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
        const char *dias[7] = { "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado" };
        const char *meses[12] = { "Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic" };
        snprintf(dateStr, sizeof(dateStr), "%s, %d de %s", dias[t->tm_wday], t->tm_mday, meses[t->tm_mon]);
    } else {
        snprintf(timeStr, sizeof(timeStr), "--:--:--");
        snprintf(dateStr, sizeof(dateStr), "Conecta a Wi-Fi para hora real");
    }

    _tft.fillRect(20, 36, 280, 48, COLOR_BG);
    _tft.setTextColor(TFT_WHITE, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 40, 7);

    _tft.fillRect(20, 88, 280, 18, COLOR_BG);
    _tft.setTextColor(TFT_GOLD, COLOR_BG);
    _tft.drawCentreString(dateStr, 160, 89, 2);
}

// ============================================================================
// SUBVISTAS DE CONFIGURACIÓN WI-FI
// ============================================================================

void UIManager::drawWiFiScanView(const WiFiConfig &wifi, bool scanning, int page) {
    _tft.fillScreen(COLOR_BG);

    _tft.fillRect(0, 0, 320, 30, 0x1082);
    _tft.setTextColor(TFT_WHITE, 0x1082);
    _tft.drawCentreString("REDES WI-FI CERCANAS", 160, 7, 2);
    _tft.drawFastHLine(0, 30, 320, 0x3186);

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
