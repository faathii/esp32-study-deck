#include "UIManager.h"
#include <time.h>

// ============================================================================
// PALETA MINIMALISTA Y SOBRIA (DARK MONOCHROME / SWISS PRECISION AESTHETIC)
// Sin colorines, fondo negro absoluto (OLED), tarjetas grafito mate y alto contraste
// ============================================================================
#define COLOR_BG              0x0000 // Negro absoluto (OLED Pitch Black)
#define COLOR_STATUS_BAR      0x0000 // Barra de estado integrada en negro puro
#define COLOR_SEPARATOR       0x18C3 // Línea de separación sutil grafito (24, 24, 24)

#define COLOR_CARD            0x1082 // Tarjeta grafito mate profundo (16, 16, 16)
#define COLOR_CARD_HL         0x18C3 // Tarjeta activa / ranura (24, 24, 24)
#define COLOR_BORDER          0x2945 // Borde fino sutil (41, 42, 41)
#define COLOR_BORDER_HL       0xFFFF // Borde destacado blanco nítido

#define COLOR_TEXT_PRIMARY    0xFFFF // Blanco nítido (alto contraste)
#define COLOR_TEXT_SECONDARY  0xAD55 // Plata / Gris neutro medio (173, 170, 173)
#define COLOR_TEXT_MUTED      0x632C // Gris grafito secundario (99, 97, 99)

UIManager::UIManager(TFT_eSPI &tft) : _tft(tft) {}

void UIManager::begin() {
    _tft.begin();
    _tft.setRotation(SCREEN_ROTATION);
    _tft.fillScreen(COLOR_BG);
}

// ============================================================================
// BARRA SUPERIOR UNIFICADA MINIMALISTA (STATUS BAR)
// ============================================================================

void UIManager::drawUnifiedStatusBar(const BatteryManager &battery, bool wifiConnected, 
                                     bool audioMuted, bool autoBrightness, 
                                     int streakCount, const char *appTitle) {
    _tft.fillRect(0, 0, 320, 24, COLOR_STATUS_BAR);

    // 1. ZONA IZQUIERDA: Reloj o Botón Volver
    if (appTitle == nullptr) {
        time_t now = time(nullptr);
        struct tm *t = localtime(&now);
        char timeStr[16];
        if (t->tm_year > 120) {
            snprintf(timeStr, sizeof(timeStr), "%02d:%02d", t->tm_hour, t->tm_min);
        } else {
            snprintf(timeStr, sizeof(timeStr), "Study Deck");
        }
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_STATUS_BAR);
        _tft.drawString(timeStr, 8, 4, 2);
    } else {
        _tft.fillRoundRect(4, 2, 60, 20, 3, COLOR_CARD);
        _tft.drawRoundRect(4, 2, 60, 20, 3, COLOR_BORDER);
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawCentreString("< INICIO", 34, 6, 1);

        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_STATUS_BAR);
        _tft.drawString(appTitle, 68, 4, 2);
    }

    // 2. ZONA CENTRO: Racha e indicadores sobrios
    char streakStr[16];
    snprintf(streakStr, sizeof(streakStr), "* %d", streakCount);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_STATUS_BAR);
    _tft.drawString(streakStr, 142, 4, 2);

    if (audioMuted) {
        _tft.fillRoundRect(172, 3, 26, 18, 3, COLOR_CARD);
        _tft.drawRoundRect(172, 3, 26, 18, 3, COLOR_BORDER);
        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
        _tft.drawCentreString("MUTE", 185, 6, 1);
    }

    if (autoBrightness) {
        _tft.fillRoundRect(202, 3, 24, 18, 3, COLOR_CARD);
        _tft.drawRoundRect(202, 3, 24, 18, 3, COLOR_BORDER);
        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
        _tft.drawCentreString("AUTO", 214, 6, 1);
    }

    // 3. ZONA DERECHA: Wi-Fi y Batería (diseño monocromo sobrio)
    _tft.fillRoundRect(230, 3, 34, 18, 3, COLOR_CARD);
    _tft.drawRoundRect(230, 3, 34, 18, 3, wifiConnected ? COLOR_BORDER_HL : COLOR_BORDER);
    _tft.setTextColor(wifiConnected ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawCentreString(wifiConnected ? "WiFi" : "OFF", 247, 6, 1);

    int pct = battery.getPercentage();
    char pctStr[8];
    snprintf(pctStr, sizeof(pctStr), "%d%%", pct);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_STATUS_BAR);
    _tft.drawRightString(pctStr, 298, 6, 1);

    // Glifo minimalista de batería
    _tft.drawRoundRect(301, 6, 15, 11, 2, COLOR_TEXT_PRIMARY);
    _tft.fillRect(316, 9, 2, 5, COLOR_TEXT_PRIMARY);
    int fillW = map(constrain(pct, 0, 100), 0, 100, 1, 11);
    _tft.fillRect(303, 8, fillW, 7, COLOR_TEXT_PRIMARY);

    _tft.drawFastHLine(0, 23, 320, COLOR_SEPARATOR);
}

// ============================================================================
// 1. PANTALLA DE INICIO (LAUNCHER MINIMALISTA CON PESTAÑAS 12 APPS)
// ============================================================================

void UIManager::drawAppCard(int x, int y, int w, int h, uint16_t /*bgColor*/, const char *icon, const char *title, const char *subtitle) {
    _tft.fillRoundRect(x, y, w, h, 6, COLOR_CARD);
    _tft.drawRoundRect(x, y, w, h, 6, COLOR_BORDER);

    // Monograma / Icono en blanco nítido
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString(icon, x + w / 2, y + 10, 4);

    // Título de la app en blanco nítido
    _tft.drawCentreString(title, x + w / 2, y + 42, 2);

    // Subtítulo técnico en plata / gris neutro
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString(subtitle, x + w / 2, y + 62, 1);
}

void UIManager::drawHomeScreen(const PomodoroEngine &engine, const TaskManager &tasks, 
                            const BatteryManager &battery, const WeatherManager &weather,
                            bool wifiConnected, bool audioMuted, bool autoBrightness, int homeTab) {
    _tft.fillScreen(COLOR_BG);

    drawUnifiedStatusBar(battery, wifiConnected, audioMuted, autoBrightness, 
                         engine.getCompletedSessionsToday(), nullptr);

    // PESTAÑAS SUPERIORES DE NAVEGACIÓN: [ 1. ESTUDIO ] y [ 2. UTILIDADES ]
    // Pestaña 1: ESTUDIO (x: 10, y: 25, w: 146, h: 22)
    bool tab0Active = (homeTab == 0);
    _tft.fillRoundRect(10, 25, 146, 22, 4, tab0Active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawRoundRect(10, 25, 146, 22, 4, tab0Active ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
    _tft.setTextColor(tab0Active ? COLOR_BG : COLOR_TEXT_SECONDARY, tab0Active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawCentreString("1. MODO ESTUDIO", 83, 29, 2);

    // Pestaña 2: UTILIDADES (x: 164, y: 25, w: 146, h: 22)
    bool tab1Active = (homeTab == 1);
    _tft.fillRoundRect(164, 25, 146, 22, 4, tab1Active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawRoundRect(164, 25, 146, 22, 4, tab1Active ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
    _tft.setTextColor(tab1Active ? COLOR_BG : COLOR_TEXT_SECONDARY, tab1Active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawCentreString("2. UTILIDADES DESK", 237, 29, 2);

    // Textos informativos de estado
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

    char batSub[24];
    if (battery.isCharging()) {
        snprintf(batSub, sizeof(batSub), "Cargando");
    } else {
        snprintf(batSub, sizeof(batSub), "%d%% (~%dh)", battery.getPercentage(), (int)battery.getEstimatedHoursLeft());
    }

    char wSub[24];
    if (weather.hasData()) {
        snprintf(wSub, sizeof(wSub), "%.1f C", weather.getTemperature());
    } else {
        snprintf(wSub, sizeof(wSub), "Open-Meteo");
    }

    if (homeTab == 0) {
        // --- PÁGINA 1: HERRAMIENTAS DE ESTUDIO (6 APPS) ---
        // FILA 1 (y: 52, h: 84)
        drawAppCard(10, 52, 94, 84, COLOR_CARD, "[T]", "Pomodoro", pomoSub);
        drawAppCard(113, 52, 94, 84, COLOR_CARD, "[V]", "Tareas", taskSub);
        drawAppCard(216, 52, 94, 84, COLOR_CARD, "[F]", "Fichas", "Repaso");

        // FILA 2 (y: 142, h: 84)
        drawAppCard(10, 142, 94, 84, COLOR_CARD, "[A]", "Agenda", "Examenes");
        drawAppCard(113, 142, 94, 84, COLOR_CARD, "[S]", "Stats SD", "Historial");
        drawAppCard(216, 142, 94, 84, COLOR_CARD, "[*]", "Ajustes", batSub);
    } else {
        // --- PÁGINA 2: UTILIDADES & DESK COMPANION (6 APPS) ---
        // FILA 1 (y: 52, h: 84)
        drawAppCard(10, 52, 94, 84, COLOR_CARD, "[C]", "Clima", wSub);
        drawAppCard(113, 52, 94, 84, COLOR_CARD, "[N]", "Reloj Noche", "Tenue");
        drawAppCard(216, 52, 94, 84, COLOR_CARD, "[R]", "Relax Box", "4-4-4-4");

        // FILA 2 (y: 142, h: 84)
        drawAppCard(10, 142, 94, 84, COLOR_CARD, "[L]", "Lampara", "Luz RGB");
        drawAppCard(113, 142, 94, 84, COLOR_CARD, "[B]", "Bateria 5Ah", batSub);
        drawAppCard(216, 142, 94, 84, COLOR_CARD, "[i]", "Info Deck", "v1.2.0");
    }
}

int UIManager::handleHomeTouch(int x, int y, int &homeTab) {
    // 1. Selector de Pestañas Superiores (y: 22..48)
    if (y >= 22 && y <= 48) {
        if (x >= 8 && x <= 158) {
            homeTab = 0;
            return -10; // Cambio de pestaña
        }
        if (x >= 160 && x <= 312) {
            homeTab = 1;
            return -10; // Cambio de pestaña
        }
    }

    // 2. Fila 1 (y: 50..138)
    if (y >= 50 && y <= 138) {
        if (x >= 8 && x <= 106)   return (homeTab == 0) ? 1 : 8;  // Col 1: Pomodoro / Clima
        if (x >= 111 && x <= 209) return (homeTab == 0) ? 2 : 9;  // Col 2: Tareas / Reloj Noche
        if (x >= 214 && x <= 312) return (homeTab == 0) ? 3 : 10; // Col 3: Fichas / Respiración Relax
    }

    // 3. Fila 2 (y: 140..228)
    if (y >= 140 && y <= 228) {
        if (x >= 8 && x <= 106)   return (homeTab == 0) ? 4 : 11; // Col 1: Agenda / Lámpara RGB
        if (x >= 111 && x <= 209) return (homeTab == 0) ? 7 : 12; // Col 2: Stats SD / Batería 5000mAh
        if (x >= 214 && x <= 312) return (homeTab == 0) ? 5 : 6;  // Col 3: Ajustes / Info
    }

    return 0;
}

// ============================================================================
// 2. APLICACIÓN POMODORO MINIMALISTA
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
    bool is90 = (engine.getFocusMinutes() == 90);

    // Botón 25 min
    _tft.fillRoundRect(15, 30, 85, 24, 4, is25 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawRoundRect(15, 30, 85, 24, 4, is25 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
    _tft.setTextColor(is25 ? COLOR_BG : COLOR_TEXT_SECONDARY, is25 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawCentreString("25 min", 57, 34, 2);

    // Botón 50 min
    _tft.fillRoundRect(117, 30, 85, 24, 4, is50 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawRoundRect(117, 30, 85, 24, 4, is50 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
    _tft.setTextColor(is50 ? COLOR_BG : COLOR_TEXT_SECONDARY, is50 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawCentreString("50 min", 159, 34, 2);

    // Botón 90 min
    _tft.fillRoundRect(220, 30, 85, 24, 4, is90 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawRoundRect(220, 30, 85, 24, 4, is90 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
    _tft.setTextColor(is90 ? COLOR_BG : COLOR_TEXT_SECONDARY, is90 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawCentreString("90 min", 262, 34, 2);
}

void UIManager::drawCountdown(const PomodoroEngine &engine) {
    uint32_t totalSec = engine.getRemainingSeconds();
    uint16_t mins = totalSec / 60;
    uint16_t secs = totalSec % 60;

    char timeStr[16];
    snprintf(timeStr, sizeof(timeStr), "%02d:%02d", mins, secs);

    uint16_t timerColor = COLOR_TEXT_PRIMARY;
    if (engine.isPaused()) timerColor = COLOR_TEXT_SECONDARY;

    _tft.fillRect(20, 58, 280, 56, COLOR_BG);
    _tft.setTextColor(timerColor, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 60, 7);
}

void UIManager::drawProgressBar(const PomodoroEngine &engine) {
    float progress = engine.getProgress();
    if (progress < 0.0f) progress = 0.0f;
    if (progress > 1.0f) progress = 1.0f;

    const int barX = 25;
    const int barY = 122;
    const int barW = 270;
    const int barH = 8;

    _tft.fillRoundRect(barX, barY, barW, barH, 4, COLOR_CARD_HL);
    _tft.drawRoundRect(barX, barY, barW, barH, 4, COLOR_BORDER);

    int fillW = (int)(progress * barW);
    if (fillW > 4) {
        _tft.fillRoundRect(barX, barY, fillW, barH, 4, COLOR_TEXT_PRIMARY);
    }

    _tft.fillRect(20, 136, 280, 16, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);

    const char *subtext = "Toca INICIAR para comenzar";
    if (engine.getState() == POMO_STATE_FOCUS) subtext = "Modo Enfoque - No Molestar";
    else if (engine.getState() == POMO_STATE_SHORT_BREAK) subtext = "Descanso Corto - Estira la vista";
    else if (engine.getState() == POMO_STATE_LONG_BREAK) subtext = "Descanso Largo - Gran sesion";
    else if (engine.isPaused()) subtext = "Sesion en Pausa";

    _tft.drawCentreString(subtext, 160, 137, 2);
}

void UIManager::drawActionButtons(const PomodoroEngine &engine) {
    // 1. REINICIAR (Izquierda)
    _tft.fillRoundRect(15, 162, 85, 48, 6, COLOR_CARD);
    _tft.drawRoundRect(15, 162, 85, 48, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("REINICIAR", 57, 178, 2);

    // 2. INICIAR / PAUSAR / SEGUIR (Centro)
    if (engine.isRunning()) {
        _tft.fillRoundRect(110, 160, 100, 52, 6, COLOR_CARD);
        _tft.drawRoundRect(110, 160, 100, 52, 6, COLOR_BORDER_HL);
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawCentreString("PAUSAR", 160, 176, 2);
    } else {
        _tft.fillRoundRect(110, 160, 100, 52, 6, COLOR_TEXT_PRIMARY);
        _tft.drawRoundRect(110, 160, 100, 52, 6, COLOR_TEXT_PRIMARY);
        _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
        _tft.drawCentreString(engine.isPaused() ? "SEGUIR" : "INICIAR", 160, 176, 2);
    }

    // 3. SALTAR (Derecha)
    _tft.fillRoundRect(220, 162, 85, 48, 6, COLOR_CARD);
    _tft.drawRoundRect(220, 162, 85, 48, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("SALTAR", 262, 178, 2);
}

UIButtonAction UIManager::handlePomodoroTouch(int x, int y) {
    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        return UI_ACTION_BACK_HOME;
    }

    if (y >= 28 && y <= 56) {
        if (x >= 10 && x <= 102) return UI_ACTION_SELECT_25;
        if (x >= 110 && x <= 204) return UI_ACTION_SELECT_50;
        if (x >= 212 && x <= 308) return UI_ACTION_SELECT_90;
    }

    if (x >= 10 && x <= 104 && y >= 158 && y <= 220) return UI_ACTION_RESET;
    if (x >= 106 && x <= 214 && y >= 158 && y <= 220) return UI_ACTION_PLAY_PAUSE;
    if (x >= 216 && x <= 310 && y >= 158 && y <= 220) return UI_ACTION_SKIP;

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
        int cardY = 30 + i * 32;

        _tft.fillRoundRect(12, cardY, 296, 28, 4, COLOR_CARD);
        _tft.drawRoundRect(12, cardY, 296, 28, 4, COLOR_BORDER);

        _tft.fillRoundRect(18, cardY + 4, 20, 20, 3, task.completed ? COLOR_TEXT_PRIMARY : COLOR_CARD);
        _tft.drawRoundRect(18, cardY + 4, 20, 20, 3, COLOR_BORDER);
        if (task.completed) {
            _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
            _tft.drawCentreString("V", 28, cardY + 6, 2);
        }

        String txt = task.text;
        if (txt.length() > 24) txt = txt.substring(0, 22) + "...";
        _tft.setTextColor(task.completed ? COLOR_TEXT_MUTED : COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawString(txt, 44, cardY + 6, 2);

        if (task.completed) {
            _tft.drawFastHLine(44, cardY + 14, txt.length() * 7, COLOR_TEXT_MUTED);
        }

        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawCentreString("x", 294, cardY + 6, 2);
    }

    _tft.fillRoundRect(12, 194, 142, 34, 5, COLOR_TEXT_PRIMARY);
    _tft.drawRoundRect(12, 194, 142, 34, 5, COLOR_TEXT_PRIMARY);
    _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
    _tft.drawCentreString("+ NUEVA TAREA", 83, 203, 2);

    _tft.fillRoundRect(166, 194, 142, 34, 5, COLOR_CARD);
    _tft.drawRoundRect(166, 194, 142, 34, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString("LIMPIAR HECHAS", 237, 203, 2);
}

int UIManager::handleTodoTouch(int x, int y, bool *outAdd, bool *outClear, int *outDelete, bool *outBack) {
    *outAdd = false;
    *outClear = false;
    *outDelete = -1;
    *outBack = false;

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return -1;
    }

    if (x >= 12 && x <= 154 && y >= 190 && y <= 235) {
        *outAdd = true;
        return -1;
    }

    if (x >= 165 && x <= 308 && y >= 190 && y <= 235) {
        *outClear = true;
        return -1;
    }

    for (int i = 0; i < 6; i++) {
        int cardY = 30 + i * 32;
        if (y >= cardY && y <= cardY + 28) {
            if (x >= 280 && x <= 310) {
                *outDelete = i;
                return -1;
            }
            if (x >= 12 && x < 280) {
                return i;
            }
        }
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

    _tft.fillRoundRect(15, 30, 290, 126, 6, COLOR_CARD);
    _tft.drawRoundRect(15, 30, 290, 126, 6, flipped ? COLOR_BORDER_HL : COLOR_BORDER);

    if (!flipped) {
        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
        _tft.drawString(card.category, 25, 38, 2);
        _tft.drawRightString("[ PREGUNTA ]", 295, 38, 2);

        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawCentreString(card.question, 160, 74, 4);

        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawCentreString("[ Toca la tarjeta para ver respuesta ]", 160, 128, 2);
    } else {
        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
        _tft.drawString("RESPUESTA", 25, 36, 2);
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);

        int lineY = 60;
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

    _tft.fillRoundRect(12, 168, 92, 48, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 168, 92, 48, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("DIFICIL", 58, 184, 2);

    _tft.fillRoundRect(114, 168, 92, 48, 6, COLOR_CARD);
    _tft.drawRoundRect(114, 168, 92, 48, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString("+ NUEVA", 160, 184, 2);

    _tft.fillRoundRect(216, 168, 92, 48, 6, COLOR_TEXT_PRIMARY);
    _tft.drawRoundRect(216, 168, 92, 48, 6, COLOR_TEXT_PRIMARY);
    _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
    _tft.drawCentreString("FACIL", 262, 184, 2);
}

bool UIManager::handleFlashcardTouch(int x, int y, bool *outFlipped, bool *outEasy, bool *outHard, bool *outAdd, bool *outBack) {
    *outFlipped = false;
    *outEasy = false;
    *outHard = false;
    *outAdd = false;
    *outBack = false;

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return true;
    }

    if (x >= 15 && x <= 305 && y >= 30 && y <= 158) {
        *outFlipped = true;
        return true;
    }

    if (x >= 10 && x <= 106 && y >= 165 && y <= 225) {
        *outHard = true;
        return true;
    }

    if (x >= 110 && x <= 210 && y >= 165 && y <= 225) {
        *outAdd = true;
        return true;
    }

    if (x >= 214 && x <= 312 && y >= 165 && y <= 225) {
        *outEasy = true;
        return true;
    }

    return false;
}

// ============================================================================
// 5. APLICACIÓN AGENDA & RELOJ & EXÁMENES
// ============================================================================

void UIManager::drawAgendaView(CalendarManager &calendar, const WiFiConfig &wifi, 
                              const BatteryManager &battery, const WeatherManager &weather) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifi.isConnected(), false, true, 0, "AGENDA & EXAMENES");

    updateAgendaClock(calendar, weather);

    // 1. Tarjeta Clima y Temperatura (Open-Meteo) (y: 110..150)
    _tft.fillRoundRect(12, 110, 296, 40, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 110, 296, 40, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString("Clima actual (Open-Meteo):", 20, 114, 1);

    if (weather.hasData()) {
        char wLine[48];
        snprintf(wLine, sizeof(wLine), "%s: %.1f C | %s | %d%% HR", 
                 weather.getCityName().c_str(), weather.getTemperature(), 
                 weather.getConditionText().c_str(), weather.getHumidity());
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawString(wLine, 20, 128, 2);
    } else {
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawString(wifi.isConnected() ? "Consultando estacion..." : "Conecta Wi-Fi para clima real", 20, 128, 2);
    }

    // 2. Tarjeta Cuenta Regresiva de Exámenes (y: 154..194)
    String examTitle;
    String examCountdown = calendar.getNextDeadlineCountdown(examTitle);

    _tft.fillRoundRect(12, 154, 296, 40, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 154, 296, 40, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString("Proximo Examen:", 20, 158, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(examTitle, 20, 172, 2);
    _tft.drawRightString(examCountdown, 298, 172, 2);

    // 3. Botonera inferior: [+ EXAMEN] y [RELOJ NOCTURNO]
    _tft.fillRoundRect(12, 198, 142, 34, 5, COLOR_TEXT_PRIMARY);
    _tft.drawRoundRect(12, 198, 142, 34, 5, COLOR_TEXT_PRIMARY);
    _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
    _tft.drawCentreString("+ EXAMEN", 83, 207, 2);

    _tft.fillRoundRect(166, 198, 142, 34, 5, COLOR_CARD);
    _tft.drawRoundRect(166, 198, 142, 34, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString("RELOJ NOCTURNO", 237, 207, 2);
}

void UIManager::updateAgendaClock(CalendarManager &calendar, const WeatherManager &/*weather*/) {
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
        snprintf(dateStr, sizeof(dateStr), "Conecta a Wi-Fi para hora exacta");
    }

    _tft.fillRect(20, 28, 280, 50, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 30, 7);

    _tft.fillRect(20, 80, 280, 22, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
    _tft.drawCentreString(dateStr, 160, 82, 2);
}

bool UIManager::handleAgendaTouch(int x, int y, bool *outAddExam, bool *outNightMode, bool *outBack) {
    *outAddExam = false;
    *outNightMode = false;
    *outBack = false;

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return true;
    }

    if (x >= 12 && x <= 155 && y >= 194 && y <= 236) {
        *outAddExam = true;
        return true;
    }

    if (x >= 165 && x <= 308 && y >= 194 && y <= 236) {
        *outNightMode = true;
        return true;
    }

    return false;
}

// ============================================================================
// 6. APLICACIÓN DEDICADA: ESTACIÓN DE CLIMA (OPEN-METEO)
// ============================================================================

void UIManager::drawWeatherView(const WeatherManager &weather, const BatteryManager &battery, bool wifiConnected) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifiConnected, false, true, 0, "ESTACION CLIMA");

    // Tarjeta principal Temperatura (y: 30..106)
    _tft.fillRoundRect(12, 30, 296, 76, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 30, 296, 76, 6, COLOR_BORDER);

    if (weather.hasData()) {
        char tempStr[16];
        snprintf(tempStr, sizeof(tempStr), "%.1f C", weather.getTemperature());
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawCentreString(tempStr, 160, 36, 7);

        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
        _tft.drawCentreString(weather.getConditionText().c_str(), 160, 88, 2);
    } else {
        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
        _tft.drawCentreString("Sin datos meteorologicos", 160, 50, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawCentreString(wifiConnected ? "Conectando con Open-Meteo..." : "Conecta la red Wi-Fi en Ajustes", 160, 76, 2);
    }

    // Tarjetas de detalles (y: 112..176)
    _tft.fillRoundRect(12, 112, 142, 60, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 112, 142, 60, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString("Humedad:", 20, 118, 1);
    char humStr[16];
    snprintf(humStr, sizeof(humStr), "%d%% HR", weather.getHumidity());
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(humStr, 20, 134, 4);

    _tft.fillRoundRect(166, 112, 142, 60, 6, COLOR_CARD);
    _tft.drawRoundRect(166, 112, 142, 60, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString("Estacion / API:", 174, 118, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Open-Meteo", 174, 134, 2);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawString("HTTPS Libre", 174, 152, 1);

    // Botón inferior: [ ACTUALIZAR AHORA ]
    _tft.fillRoundRect(12, 186, 296, 42, 6, COLOR_TEXT_PRIMARY);
    _tft.drawRoundRect(12, 186, 296, 42, 6, COLOR_TEXT_PRIMARY);
    _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
    _tft.drawCentreString("ACTUALIZAR DATOS", 160, 198, 2);
}

bool UIManager::handleWeatherTouch(int x, int y, bool *outRefresh, bool *outBack) {
    *outRefresh = false;
    *outBack = false;

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return true;
    }

    if (x >= 12 && x <= 308 && y >= 180 && y <= 236) {
        *outRefresh = true;
        return true;
    }

    return false;
}

// ============================================================================
// 7. APLICACIÓN DEDICADA: RELOJ NOCTURNO (NIGHT DESK CLOCK)
// ============================================================================

void UIManager::drawNightClockView(const BatteryManager &battery, const WeatherManager &weather) {
    _tft.fillScreen(COLOR_BG);

    updateNightClock(battery, weather);

    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawCentreString("[ Toca en cualquier lugar para salir ]", 160, 220, 1);
}

void UIManager::updateNightClock(const BatteryManager &battery, const WeatherManager &weather) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    char timeStr[16];
    char dateStr[36];

    if (t->tm_year > 120) {
        snprintf(timeStr, sizeof(timeStr), "%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec);
        const char *dias[7] = { "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado" };
        const char *meses[12] = { "Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic" };
        snprintf(dateStr, sizeof(dateStr), "%s, %d de %s", dias[t->tm_wday], t->tm_mday, meses[t->tm_mon]);
    } else {
        snprintf(timeStr, sizeof(timeStr), "Study Deck");
        snprintf(dateStr, sizeof(dateStr), "Modo Reloj Nocturno");
    }

    _tft.fillRect(10, 36, 300, 68, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 42, 7);

    _tft.fillRect(10, 114, 300, 24, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawCentreString(dateStr, 160, 118, 2);

    _tft.fillRect(10, 144, 300, 24, COLOR_BG);
    if (weather.hasData()) {
        char wBuf[48];
        snprintf(wBuf, sizeof(wBuf), "%s %.1f C · %s", 
                 weather.getCityName().c_str(), weather.getTemperature(), weather.getConditionText().c_str());
        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
        _tft.drawCentreString(wBuf, 160, 148, 2);
    } else {
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
        _tft.drawCentreString("Ambiente Nocturno", 160, 148, 2);
    }

    _tft.fillRect(10, 174, 300, 24, COLOR_BG);
    char batBuf[48];
    snprintf(batBuf, sizeof(batBuf), "Bateria: %d%% (%s)", battery.getPercentage(), battery.getRuntimeString().c_str());
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawCentreString(batBuf, 160, 178, 2);
}

bool UIManager::handleNightClockTouch(int /*x*/, int /*y*/) {
    return true;
}

// ============================================================================
// 8. APLICACIÓN DEDICADA: RESPIRACIÓN CONSCIENTE (BOX BREATHING 4-4-4-4)
// ============================================================================

void UIManager::drawBreathingView(int phase, int secondsLeft, int cycle, const BatteryManager &battery) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, true, false, true, 0, "RESPIRACION RELAX");

    const char *phaseTitles[4] = { "INHALA LENTAMENTE", "MANTEN EL AIRE", "EXHALA DESPACIO", "PAUSA Y RELAJA" };
    const char *phaseHints[4] = { "Llena tus pulmones con calma", "Manten la calma y serenidad", "Vacia el aire suavemente", "Mente despejada y lista" };

    // Título de fase en blanco nítido
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
    _tft.drawCentreString(phaseTitles[phase % 4], 160, 48, 4);

    // Contador de segundos gigante (Font 7)
    char secStr[8];
    snprintf(secStr, sizeof(secStr), "%d", secondsLeft);
    _tft.drawCentreString(secStr, 160, 86, 7);

    // Subtítulo explicativo
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
    _tft.drawCentreString(phaseHints[phase % 4], 160, 148, 2);

    // Ciclo actual
    char cycStr[32];
    snprintf(cycStr, sizeof(cycStr), "Ciclo %d de 4 · Tecnica Box (4-4-4-4)", cycle);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawCentreString(cycStr, 160, 176, 2);

    // Indicador inferior
    _tft.drawCentreString("[ Toca '< INICIO' para volver cuando quieras ]", 160, 212, 1);
}

bool UIManager::handleBreathingTouch(int x, int y) {
    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        return true;
    }
    return false;
}

// ============================================================================
// 9. APLICACIÓN DEDICADA: LÁMPARA RGB DE ESCRITORIO
// ============================================================================

void UIManager::drawRgbLampView(int currentMode, const BatteryManager &battery) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, true, false, true, 0, "LAMPARA AMBIENTE");

    const char *names[5] = { "APAGADO", "BLANCO CALIDO", "ENFOQUE CIAN", "RELAX AZUL", "AMBAR VELA" };

    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
    _tft.drawString("Selecciona el modo de luz de escritorio:", 15, 32, 2);

    for (int i = 0; i < 5; i++) {
        int by = 54 + i * 34;
        bool active = (currentMode == i);

        _tft.fillRoundRect(15, by, 290, 30, 4, active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
        _tft.drawRoundRect(15, by, 290, 30, 4, active ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
        _tft.setTextColor(active ? COLOR_BG : COLOR_TEXT_PRIMARY, active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
        _tft.drawCentreString(names[i], 160, by + 7, 2);
    }
}

int UIManager::handleRgbLampTouch(int x, int y, bool *outBack) {
    *outBack = false;

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return -1;
    }

    for (int i = 0; i < 5; i++) {
        int by = 54 + i * 34;
        if (x >= 15 && x <= 305 && y >= by && y <= by + 30) {
            return i;
        }
    }

    return -1;
}

// ============================================================================
// 10. APLICACIÓN DEDICADA: TELEMETRÍA BATERÍA 5000 mAh
// ============================================================================

void UIManager::drawBatteryView(const BatteryManager &battery, bool wifiConnected) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifiConnected, false, true, 0, "TELEMETRIA BATERIA");

    // Tarjeta porcentaje gigante (y: 28..98)
    _tft.fillRoundRect(12, 28, 296, 70, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 28, 296, 70, 6, COLOR_BORDER);

    char pctStr[16];
    snprintf(pctStr, sizeof(pctStr), "%d%%", battery.getPercentage());
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString(pctStr, 160, 32, 7);

    char vStr[48];
    snprintf(vStr, sizeof(vStr), "%.2f V  |  Celda LiPo 1S 3.7V (18.5 Wh)", battery.getVoltage());
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString(vStr, 160, 80, 1);

    // Tarjeta métricas detalladas (y: 104..180)
    _tft.fillRoundRect(12, 104, 296, 80, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 104, 296, 80, 6, COLOR_BORDER);

    char line1[48], line2[48], line3[48];
    snprintf(line1, sizeof(line1), "Capacidad: %d / 5000 mAh", battery.getRemainingCapacityMah());
    snprintf(line2, sizeof(line2), "Autonomia estimada: %s", battery.getRuntimeString().c_str());
    snprintf(line3, sizeof(line3), "Estado: %s (Circuito IP5603)", battery.isCharging() ? "Cargando USB" : "En bateria");

    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(line1, 22, 114, 2);
    _tft.drawString(line2, 22, 134, 2);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(line3, 22, 156, 1);

    // Botón volver
    _tft.fillRoundRect(12, 192, 296, 36, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 192, 296, 36, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("< VOLVER AL MENU", 160, 202, 2);
}

bool UIManager::handleBatteryTouch(int x, int y) {
    if ((x >= 4 && x <= 64 && y >= 0 && y <= 24) || (y >= 190 && y <= 235)) {
        return true;
    }
    return false;
}

// ============================================================================
// 11. APLICACIÓN DEDICADA: ESTADÍSTICAS & HISTORIAL MICROSD
// ============================================================================

void UIManager::drawStatsView(StorageManager &storage, const PomodoroEngine &pomodoro, const BatteryManager &battery, bool wifiConnected) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifiConnected, false, true, 
                         pomodoro.getCompletedSessionsToday(), "HISTORIAL ESTUDIO");

    _tft.fillRoundRect(12, 30, 296, 52, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 30, 296, 52, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Sesiones Hoy:", 20, 36, 2);
    char pBuf[32];
    snprintf(pBuf, sizeof(pBuf), "%d Pomodoros completados", pomodoro.getCompletedSessionsToday());
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(pBuf, 20, 56, 2);

    _tft.fillRoundRect(12, 88, 296, 52, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 88, 296, 52, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Tiempo Total Acumulado:", 20, 94, 2);
    char tBuf[32];
    int totalMins = storage.getTotalStudyMinutes();
    snprintf(tBuf, sizeof(tBuf), "%d min (~%.1f horas)", totalMins, (float)totalMins / 60.0f);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(tBuf, 20, 114, 2);

    _tft.fillRoundRect(12, 146, 296, 44, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 146, 296, 44, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Registro MicroSD /study_log.csv:", 20, 152, 1);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(storage.isAvailable() ? "Activo (Guardando datos reales)" : "Modo interno NVS", 20, 168, 2);

    _tft.fillRoundRect(12, 196, 296, 36, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 196, 296, 36, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("< VOLVER AL MENU", 160, 206, 2);
}

bool UIManager::handleStatsTouch(int x, int y) {
    if ((x >= 4 && x <= 64 && y >= 0 && y <= 24) || (y >= 190 && y <= 236)) {
        return true;
    }
    return false;
}

// ============================================================================
// 12. APLICACIÓN AJUSTES DEL SISTEMA
// ============================================================================

void UIManager::drawSettingsView(const WiFiConfig &wifi, const BatteryManager &battery, 
                                int brightness, bool autoBrightness, bool soundMuted, int /*timeoutMinutes*/) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifi.isConnected(), soundMuted, autoBrightness, 0, "AJUSTES");

    // 1. Tarjeta Batería LiPo 5000 mAh (y: 28..70)
    _tft.fillRoundRect(12, 28, 296, 44, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 28, 296, 44, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Bateria 5000mAh 3.7V (18.5Wh)", 20, 34, 2);

    char batLine[48];
    snprintf(batLine, sizeof(batLine), "%d%% (%.2fV) | %s", 
             battery.getPercentage(), battery.getVoltage(), battery.getRuntimeString().c_str());
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(batLine, 20, 52, 1);

    // 2. Tarjeta Brillo de Pantalla (y: 76..118)
    _tft.fillRoundRect(12, 76, 296, 44, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 76, 296, 44, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Brillo Pantalla", 20, 82, 2);

    char briStr[16];
    snprintf(briStr, sizeof(briStr), "%d%%", (brightness * 100) / 255);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(briStr, 20, 98, 2);

    _tft.fillRoundRect(115, 82, 34, 30, 4, COLOR_CARD_HL);
    _tft.drawRoundRect(115, 82, 34, 30, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
    _tft.drawCentreString("-", 132, 88, 4);

    _tft.fillRoundRect(155, 82, 34, 30, 4, COLOR_CARD_HL);
    _tft.drawRoundRect(155, 82, 34, 30, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
    _tft.drawCentreString("+", 172, 88, 4);

    // Auto LDR toggle
    _tft.fillRoundRect(198, 82, 102, 30, 4, autoBrightness ? COLOR_TEXT_PRIMARY : COLOR_CARD_HL);
    _tft.drawRoundRect(198, 82, 102, 30, 4, COLOR_BORDER);
    _tft.setTextColor(autoBrightness ? COLOR_BG : COLOR_TEXT_SECONDARY, autoBrightness ? COLOR_TEXT_PRIMARY : COLOR_CARD_HL);
    _tft.drawCentreString(autoBrightness ? "Auto LDR" : "Manual", 249, 88, 2);

    // 3. Tarjeta Sonido y Modo Noche (y: 124..166)
    _tft.fillRoundRect(12, 124, 296, 44, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 124, 296, 44, 5, COLOR_BORDER);
    
    _tft.fillRoundRect(20, 130, 130, 30, 4, soundMuted ? COLOR_CARD_HL : COLOR_TEXT_PRIMARY);
    _tft.drawRoundRect(20, 130, 130, 30, 4, COLOR_BORDER);
    _tft.setTextColor(soundMuted ? COLOR_TEXT_MUTED : COLOR_BG, soundMuted ? COLOR_CARD_HL : COLOR_TEXT_PRIMARY);
    _tft.drawCentreString(soundMuted ? "SILENCIO" : "SONIDO ON", 85, 136, 2);

    _tft.fillRoundRect(165, 130, 135, 30, 4, COLOR_CARD);
    _tft.drawRoundRect(165, 130, 135, 30, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("MODO NOCHE", 232, 136, 2);

    // 4. Tarjeta Wi-Fi (y: 172..218)
    _tft.fillRoundRect(12, 172, 296, 48, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 172, 296, 48, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Red Wi-Fi", 20, 178, 2);

    String netInfo = wifi.isConnected() ? ("Conectado: " + wifi.getSSID()) : "Desconectado";
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(netInfo, 20, 198, 1);

    _tft.fillRoundRect(200, 178, 100, 34, 4, COLOR_TEXT_PRIMARY);
    _tft.drawRoundRect(200, 178, 100, 34, 4, COLOR_TEXT_PRIMARY);
    _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
    _tft.drawCentreString("CONFIG", 250, 186, 2);
}

int UIManager::handleSettingsTouch(int x, int y, bool *outToggleAuto, bool *outBriDown, bool *outBriUp, 
                                   bool *outToggleMute, bool *outTimeoutToggle, bool *outNightClock, bool *outOpenWifi, bool *outBack) {
    *outToggleAuto = false;
    *outBriDown = false;
    *outBriUp = false;
    *outToggleMute = false;
    *outTimeoutToggle = false;
    *outNightClock = false;
    *outOpenWifi = false;
    *outBack = false;

    if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
        *outBack = true;
        return 0;
    }

    if (y >= 76 && y <= 118) {
        if (x >= 110 && x <= 150) { *outBriDown = true; return 1; }
        if (x >= 152 && x <= 192) { *outBriUp = true; return 1; }
        if (x >= 195 && x <= 305) { *outToggleAuto = true; return 1; }
    }

    if (y >= 124 && y <= 166) {
        if (x >= 18 && x <= 155) { *outToggleMute = true; return 2; }
        if (x >= 160 && x <= 305) { *outNightClock = true; return 2; }
    }

    if (x >= 195 && x <= 305 && y >= 172 && y <= 218) {
        *outOpenWifi = true;
        return 3;
    }

    return 0;
}

// ============================================================================
// 13. APLICACIÓN INFO & ESTADÍSTICAS & MICROSD
// ============================================================================

void UIManager::drawInfoView(const BatteryManager &battery, StorageManager &storage, 
                            const PomodoroEngine &pomodoro, bool wifiConnected) {
    _tft.fillScreen(COLOR_BG);
    drawUnifiedStatusBar(battery, wifiConnected, false, true, 
                         pomodoro.getCompletedSessionsToday(), "INFO & STATS");

    _tft.fillRoundRect(12, 30, 296, 54, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 30, 296, 54, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Registro de Estudio Real", 20, 36, 2);

    char statBuf[64];
    int totalMins = storage.getTotalStudyMinutes();
    snprintf(statBuf, sizeof(statBuf), "Sesiones Hoy: %d | Total en MicroSD: %d min", 
             pomodoro.getCompletedSessionsToday(), totalMins);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(statBuf, 20, 56, 2);

    _tft.fillRoundRect(12, 90, 296, 48, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 90, 296, 48, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Almacenamiento MicroSD", 20, 96, 2);

    char sdBuf[56];
    if (storage.isAvailable()) {
        snprintf(sdBuf, sizeof(sdBuf), "Montada (%llu MB) | /study_log.csv OK", storage.getCardSizeMB());
    } else {
        snprintf(sdBuf, sizeof(sdBuf), "Modo NVS interno (SD no insertada)");
    }
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(sdBuf, 20, 114, 1);

    _tft.fillRoundRect(12, 144, 296, 76, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 144, 296, 76, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("Hardware: Guition ESP32-2432S024C", 20, 150, 2);

    char hwBuf[64];
    snprintf(hwBuf, sizeof(hwBuf), "CPU: 240MHz | RAM: %d KB | Bat: 5000mAh (%.2fV)", 
             ESP.getFreeHeap() / 1024, battery.getVoltage());
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(hwBuf, 20, 172, 1);

    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawString("Study Deck OS v1.2.0 - Codigo Abierto MIT", 20, 192, 2);
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
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
        _tft.drawCentreString("Buscando redes en el aire...", 160, 100, 2);
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
        _tft.drawCentreString("Espera un momento", 160, 125, 2);
        return;
    }

    int totalNetworks = wifi.getNetworkCount();
    if (totalNetworks == 0) {
        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
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
            _tft.drawRoundRect(12, cardY, 296, 32, 6, COLOR_BORDER);

            String name = net.ssid;
            if (name.length() > 20) {
                name = name.substring(0, 18) + "...";
            }
            _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
            _tft.drawString(name, 22, cardY + 8, 2);

            String info = (net.isEncrypted ? "[*] " : "[Abierta] ");
            info += String(net.rssi) + " dBm";
            _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
            _tft.drawRightString(info, 298, cardY + 8, 2);
        }
    }

    _tft.fillRoundRect(12, 194, 85, 36, 6, COLOR_CARD);
    _tft.drawRoundRect(12, 194, 85, 36, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("VOLVER", 54, 204, 2);

    if (totalNetworks > 4) {
        _tft.fillRoundRect(105, 194, 95, 36, 6, COLOR_CARD);
        _tft.drawRoundRect(105, 194, 95, 36, 6, COLOR_BORDER);
        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
        char pagStr[16];
        snprintf(pagStr, sizeof(pagStr), "Pag %d/%d", page + 1, (totalNetworks + 3) / 4);
        _tft.drawCentreString(pagStr, 152, 204, 2);
    }

    _tft.fillRoundRect(208, 194, 100, 36, 6, COLOR_TEXT_PRIMARY);
    _tft.drawRoundRect(208, 194, 100, 36, 6, COLOR_TEXT_PRIMARY);
    _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
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

    _tft.fillRoundRect(30, 45, 260, 150, 8, COLOR_CARD);
    _tft.drawRoundRect(30, 45, 260, 150, 8, COLOR_BORDER);

    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("Conectando a la red...", 160, 65, 2);

    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString(ssid, 160, 95, 4);

    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawCentreString("Comprobando contrasena...", 160, 140, 2);
}

void UIManager::drawWiFiResult(bool success, const String &info) {
    _tft.fillScreen(COLOR_BG);

    _tft.fillRoundRect(30, 40, 260, 160, 8, COLOR_CARD);
    _tft.drawRoundRect(30, 40, 260, 160, 8, COLOR_BORDER_HL);

    if (success) {
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawCentreString("CONEXION EXITOSA", 160, 58, 4);

        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
        _tft.drawCentreString("IP asignada:", 160, 100, 2);
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawCentreString(info, 160, 122, 2);

        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawCentreString("Guardada en NVS. Volviendo...", 160, 160, 2);
    } else {
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawCentreString("FALLO DE CONEXION", 160, 58, 4);

        _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
        _tft.drawCentreString("No se pudo conectar.", 160, 105, 2);

        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawCentreString(info, 160, 130, 2);

        _tft.fillRoundRect(85, 155, 150, 32, 6, COLOR_TEXT_PRIMARY);
        _tft.drawRoundRect(85, 155, 150, 32, 6, COLOR_TEXT_PRIMARY);
        _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
        _tft.drawCentreString("REINTENTAR", 160, 162, 2);
    }
}
