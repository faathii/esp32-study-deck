#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "PomodoroEngine.h"
#include "HardwareConfig.h"
#include "WiFiConfig.h"

enum AppView {
    VIEW_POMODORO,
    VIEW_WIFI_SCAN,
    VIEW_WIFI_KEYBOARD,
    VIEW_WIFI_CONNECTING,
    VIEW_WIFI_RESULT
};

enum UIButtonAction {
    UI_ACTION_NONE,
    UI_ACTION_PLAY_PAUSE,
    UI_ACTION_RESET,
    UI_ACTION_SKIP,
    UI_ACTION_SELECT_25,
    UI_ACTION_SELECT_50,
    UI_ACTION_OPEN_WIFI
};

class UIManager {
public:
    UIManager(TFT_eSPI &tft);

    void begin();
    
    // Vista Pomodoro
    void fullRedraw(const PomodoroEngine &engine, int ldrValue, bool wifiConnected);
    void updateTimerDisplay(const PomodoroEngine &engine, int ldrValue, bool wifiConnected);
    UIButtonAction handleTouch(int x, int y);

    // Vista de Escaneo de Redes Wi-Fi
    void drawWiFiScanView(const WiFiConfig &wifi, bool scanning, int page = 0);
    int handleWiFiScanTouch(int x, int y, int netCount, int page, bool *outRescan, bool *outBack, bool *outNextPage);

    // Vista de Conexión en progreso y Resultado
    void drawWiFiConnecting(const String &ssid);
    void drawWiFiResult(bool success, const String &info);

private:
    TFT_eSPI &_tft;

    // Estados previos para evitar parpadeos
    uint32_t _lastRemainingSec = 0xFFFFFFFF;
    PomodoroState _lastState = (PomodoroState)0xFF;
    uint8_t _lastCycle = 0xFF;
    uint16_t _lastFocusMin = 0xFF;
    int _lastLdr = -1;
    bool _lastWifiStatus = false;
    float _lastProgress = -1.0f;

    void drawTopBar(const PomodoroEngine &engine, int ldrValue, bool wifiConnected);
    void drawModeButtons(const PomodoroEngine &engine);
    void drawCountdown(const PomodoroEngine &engine);
    void drawProgressBar(const PomodoroEngine &engine);
    void drawActionButtons(const PomodoroEngine &engine);
};

#endif // UI_MANAGER_H
