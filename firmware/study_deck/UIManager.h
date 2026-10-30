#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "PomodoroEngine.h"
#include "HardwareConfig.h"
#include "WiFiConfig.h"
#include "TaskManager.h"
#include "FlashcardManager.h"

enum AppView {
    VIEW_POMODORO = 0,
    VIEW_TODO     = 1,
    VIEW_FLASH    = 2,
    VIEW_AGENDA   = 3,
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
    UI_ACTION_OPEN_WIFI,
    UI_ACTION_SWITCH_TAB
};

class UIManager {
public:
    UIManager(TFT_eSPI &tft);

    void begin();
    
    // Vista 1: Pomodoro
    void drawPomodoroView(const PomodoroEngine &engine, int ldrValue, bool wifiConnected);
    void updatePomodoroTimer(const PomodoroEngine &engine, int ldrValue, bool wifiConnected);
    UIButtonAction handlePomodoroTouch(int x, int y);

    // Vista 2: Tareas To-Do
    void drawTodoView(const TaskManager &taskMgr);
    int handleTodoTouch(int x, int y, bool *outReset);

    // Vista 3: Flashcards
    void drawFlashcardsView(const FlashcardManager &flashcards);
    bool handleFlashcardTouch(int x, int y, bool *outFlipped, bool *outEasy, bool *outHard);

    // Vista 4: Agenda & Reloj NTP
    void drawAgendaView(const WiFiConfig &wifi);
    void updateAgendaClock();

    // Barra de navegación inferior común
    void drawBottomTabs(AppView currentTab);
    int handleBottomTabsTouch(int x, int y);

    // Subvistas de Wi-Fi
    void drawWiFiScanView(const WiFiConfig &wifi, bool scanning, int page = 0);
    int handleWiFiScanTouch(int x, int y, int netCount, int page, bool *outRescan, bool *outBack, bool *outNextPage);
    void drawWiFiConnecting(const String &ssid);
    void drawWiFiResult(bool success, const String &info);

private:
    TFT_eSPI &_tft;

    // Estados previos
    uint32_t _lastRemainingSec = 0xFFFFFFFF;
    PomodoroState _lastState = (PomodoroState)0xFF;
    uint8_t _lastCycle = 0xFF;
    uint16_t _lastFocusMin = 0xFF;
    int _lastLdr = -1;
    bool _lastWifiStatus = false;
    int _lastSecClock = -1;

    void drawTopBar(const PomodoroEngine &engine, int ldrValue, bool wifiConnected);
    void drawModeButtons(const PomodoroEngine &engine);
    void drawCountdown(const PomodoroEngine &engine);
    void drawProgressBar(const PomodoroEngine &engine);
    void drawActionButtons(const PomodoroEngine &engine);
};

#endif // UI_MANAGER_H
