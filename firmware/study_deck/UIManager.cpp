#include "UIManager.h"

// Colores de la interfaz
#define COLOR_BG          0x0841 // Pizarra oscuro
#define COLOR_CARD        0x18E3 // Superficie de tarjeta elevada
#define COLOR_CARD_HL     0x2966 // Tarjeta resaltada al tocar
#define COLOR_FOCUS_RED   0xF986 // Rojo brillante
#define COLOR_BREAK_GREEN 0x05E6 // Verde esmeralda
#define COLOR_PAUSE_AMBER 0xFDA0 // Ámbar / Naranja
#define COLOR_IDLE_BLUE   0x3BFF // Azul cian moderno
#define COLOR_TEXT_MUTED  0x9CD3 // Gris suave
#define COLOR_WIFI_OK     0x2E6A // Verde Wi-Fi
#define COLOR_WIFI_OFF    0x4208 // Gris Wi-Fi desconectado

UIManager::UIManager(TFT_eSPI &tft) : _tft(tft) {}

void UIManager::begin() {
    _tft.begin();
    _tft.setRotation(SCREEN_ROTATION);
    _tft.fillScreen(COLOR_BG);
}

void UIManager::fullRedraw(const PomodoroEngine &engine, int ldrValue, bool wifiConnected) {
    _tft.fillScreen(COLOR_BG);

    _lastRemainingSec = 0xFFFFFFFF;
    _lastState = (PomodoroState)0xFF;
    _lastCycle = 0xFF;
    _lastFocusMin = 0xFF;
    _lastLdr = -1;
    _lastWifiStatus = !wifiConnected;
    _lastProgress = -1.0f;

    drawTopBar(engine, ldrValue, wifiConnected);
    drawModeButtons(engine);
    drawCountdown(engine);
    drawProgressBar(engine);
    drawActionButtons(engine);
}

void UIManager::updateTimerDisplay(const PomodoroEngine &engine, int ldrValue, bool wifiConnected) {
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

    // 1. Pastilla de estado Pomodoro
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

    _tft.fillRoundRect(8, 5, 80, 22, 6, pillColor);
    _tft.setTextColor(TFT_WHITE, pillColor);
    _tft.drawCentreString(stateLabel, 48, 8, 2);

    // 2. Ciclo actual
    _tft.setTextColor(TFT_WHITE, COLOR_BG);
    char cycleStr[16];
    snprintf(cycleStr, sizeof(cycleStr), "Ciclo %d/%d", engine.getCurrentCycle(), engine.getTotalCycles());
    _tft.drawString(cycleStr, 98, 8, 2);

    // 3. Racha
    _tft.setTextColor(TFT_GOLD, COLOR_BG);
    char streakStr[16];
    snprintf(streakStr, sizeof(streakStr), "* %d", engine.getCompletedSessionsToday());
    _tft.drawString(streakStr, 178, 8, 2);

    // 4. Botón táctil de Wi-Fi (x: 235, y: 5, w: 78, h: 22)
    uint16_t wifiBg = wifiConnected ? COLOR_WIFI_OK : COLOR_WIFI_OFF;
    _tft.fillRoundRect(235, 5, 78, 22, 6, wifiBg);
    _tft.drawRoundRect(235, 5, 78, 22, 6, wifiConnected ? TFT_GREEN : 0x5AEB);
    _tft.setTextColor(TFT_WHITE, wifiBg);
    _tft.drawCentreString(wifiConnected ? "WiFi: ON" : "WiFi +", 274, 8, 2);

    _tft.drawFastHLine(8, 31, 304, 0x2124);
}

void UIManager::drawModeButtons(const PomodoroEngine &engine) {
    bool is25 = (engine.getFocusMinutes() == 25);
    bool is50 = (engine.getFocusMinutes() == 50);

    uint16_t bg25 = is25 ? 0x21E7 : COLOR_CARD;
    uint16_t border25 = is25 ? COLOR_IDLE_BLUE : 0x3186;
    _tft.fillRoundRect(50, 38, 100, 26, 6, bg25);
    _tft.drawRoundRect(50, 38, 100, 26, 6, border25);
    _tft.setTextColor(is25 ? TFT_WHITE : COLOR_TEXT_MUTED, bg25);
    _tft.drawCentreString("25 min", 100, 43, 2);

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

    uint16_t timerColor = TFT_WHITE;
    if (engine.getState() == POMO_STATE_FOCUS) {
        timerColor = TFT_WHITE;
    } else if (engine.isPaused()) {
        timerColor = COLOR_PAUSE_AMBER;
    } else if (engine.isBreak()) {
        timerColor = COLOR_BREAK_GREEN;
    }

    _tft.fillRect(20, 72, 280, 60, COLOR_BG);
    _tft.setTextColor(timerColor, COLOR_BG);
    _tft.drawCentreString(timeStr, 160, 74, 7);
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

    _tft.fillRoundRect(barX, barY, barW, barH, 5, 0x2124);

    int fillW = (int)(progress * barW);
    if (fillW > 4) {
        _tft.fillRoundRect(barX, barY, fillW, barH, 5, barFillColor);
    }

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
    _tft.fillRoundRect(15, 188, 80, 44, 8, COLOR_CARD);
    _tft.drawRoundRect(15, 188, 80, 44, 8, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("REINICIAR", 55, 202, 2);

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

    _tft.fillRoundRect(225, 188, 80, 44, 8, COLOR_CARD);
    _tft.drawRoundRect(225, 188, 80, 44, 8, 0x3186);
    _tft.setTextColor(TFT_WHITE, COLOR_CARD);
    _tft.drawCentreString("SALTAR", 265, 202, 2);
}

UIButtonAction UIManager::handleTouch(int x, int y) {
    // 1. Toque en botón de Wi-Fi (x: 230..318, y: 0..32)
    if (x >= 230 && x <= 318 && y >= 0 && y <= 32) {
        return UI_ACTION_OPEN_WIFI;
    }

    // 2. Selector 25 min (x: 45..155, y: 35..68)
    if (x >= 45 && x <= 155 && y >= 35 && y <= 68) {
        return UI_ACTION_SELECT_25;
    }

    // 3. Selector 50 min (x: 165..275, y: 35..68)
    if (x >= 165 && x <= 275 && y >= 35 && y <= 68) {
        return UI_ACTION_SELECT_50;
    }

    // 4. Botón REINICIAR (x: 10..98, y: 182..238)
    if (x >= 10 && x <= 98 && y >= 182 && y <= 238) {
        return UI_ACTION_RESET;
    }

    // 5. Botón INICIAR / PAUSAR (x: 102..218, y: 182..238)
    if (x >= 102 && x <= 218 && y >= 182 && y <= 238) {
        return UI_ACTION_PLAY_PAUSE;
    }

    // 6. Botón SALTAR (x: 222..310, y: 182..238)
    if (x >= 222 && x <= 310 && y >= 182 && y <= 238) {
        return UI_ACTION_SKIP;
    }

    return UI_ACTION_NONE;
}

// ============================================================================
// VISTA: ESCANEO DE REDES WI-FI
// ============================================================================

void UIManager::drawWiFiScanView(const WiFiConfig &wifi, bool scanning, int page) {
    _tft.fillScreen(COLOR_BG);

    // Cabecera
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
        // Mostrar hasta 4 redes por página
        int startIndex = page * 4;
        for (int i = 0; i < 4; i++) {
            int netIdx = startIndex + i;
            if (netIdx >= totalNetworks) break;

            DiscoveredNetwork net = wifi.getNetwork(netIdx);
            int cardY = 36 + i * 36;

            // Tarjeta de la red
            _tft.fillRoundRect(12, cardY, 296, 32, 6, COLOR_CARD);
            _tft.drawRoundRect(12, cardY, 296, 32, 6, 0x3186);

            // Nombre SSID
            String name = net.ssid;
            if (name.length() > 20) {
                name = name.substring(0, 18) + "...";
            }
            _tft.setTextColor(TFT_WHITE, COLOR_CARD);
            _tft.drawString(name, 22, cardY + 8, 2);

            // Señal RSSI e icono de candado
            String info = (net.isEncrypted ? "[*] " : "[Abierta] ");
            info += String(net.rssi) + " dBm";
            _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
            _tft.drawRightString(info, 298, cardY + 8, 2);
        }
    }

    // Botonera inferior (y: 194..234)
    // 1. [ VOLVER ] (x: 12, w: 85, h: 36)
    _tft.fillRoundRect(12, 194, 85, 36, 6, 0xC8A2);
    _tft.drawRoundRect(12, 194, 85, 36, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, 0xC8A2);
    _tft.drawCentreString("VOLVER", 54, 204, 2);

    // 2. [ PAG SIG ] (x: 105, w: 95, h: 36)
    if (totalNetworks > 4) {
        _tft.fillRoundRect(105, 194, 95, 36, 6, COLOR_CARD);
        _tft.drawRoundRect(105, 194, 95, 36, 6, 0x3186);
        _tft.setTextColor(TFT_WHITE, COLOR_CARD);
        char pagStr[16];
        snprintf(pagStr, sizeof(pagStr), "Pag %d/%d", page + 1, (totalNetworks + 3) / 4);
        _tft.drawCentreString(pagStr, 152, 204, 2);
    }

    // 3. [ ESCANEAR ] (x: 208, w: 100, h: 36)
    _tft.fillRoundRect(208, 194, 100, 36, 6, COLOR_IDLE_BLUE);
    _tft.drawRoundRect(208, 194, 100, 36, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, COLOR_IDLE_BLUE);
    _tft.drawCentreString("ESCANEAR", 258, 204, 2);
}

int UIManager::handleWiFiScanTouch(int x, int y, int netCount, int page, bool *outRescan, bool *outBack, bool *outNextPage) {
    *outRescan = false;
    *outBack = false;
    *outNextPage = false;

    // Toque en una de las 4 tarjetas (y: 36..180)
    for (int i = 0; i < 4; i++) {
        int cardY = 36 + i * 36;
        if (x >= 12 && x <= 308 && y >= cardY && y <= cardY + 32) {
            int selectedIndex = page * 4 + i;
            if (selectedIndex < netCount) {
                return selectedIndex;
            }
        }
    }

    // Botón [ VOLVER ] (x: 10..98, y: 190..236)
    if (x >= 10 && x <= 98 && y >= 190 && y <= 236) {
        *outBack = true;
        return -1;
    }

    // Botón [ PAG SIG ] (x: 102..202, y: 190..236)
    if (x >= 102 && x <= 202 && y >= 190 && y <= 236) {
        *outNextPage = true;
        return -1;
    }

    // Botón [ ESCANEAR ] (x: 205..312, y: 190..236)
    if (x >= 205 && x <= 312 && y >= 190 && y <= 236) {
        *outRescan = true;
        return -1;
    }

    return -1;
}

// ============================================================================
// VISTA: CONECTANDO Y RESULTADO
// ============================================================================

void UIManager::drawWiFiConnecting(const String &ssid) {
    _tft.fillScreen(COLOR_BG);

    // Diálogo flotante central
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

        // Botón reintentar
        _tft.fillRoundRect(85, 155, 150, 32, 6, COLOR_IDLE_BLUE);
        _tft.setTextColor(TFT_WHITE, COLOR_IDLE_BLUE);
        _tft.drawCentreString("REINTENTAR", 160, 162, 2);
    }
}
