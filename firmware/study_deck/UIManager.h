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
    VIEW_HOME = 0,
    VIEW_POMODORO,
    VIEW_TODO,
    VIEW_FLASH,
    VIEW_AGENDA,
    VIEW_SETTINGS,
    VIEW_WIFI_SCAN,
    VIEW_WIFI_KEYBOARD,
    VIEW_WIFI_CONNECTING,
    VIEW_WIFI_RESULT
};

enum UIButtonAction {
    UI_ACTION_NONE,
    UI_ACTION_BACK_HOME,
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
    
    // --- 1. PANTALLA DE INICIO (ANDROID LAUNCHER CON CAJAS DE APP) ---
    void drawHomeScreen(const PomodoroEngine &engine, const TaskManager &tasks, bool wifiConnected, int ldrValue);
    int handleHomeTouch(int x, int y); // Devuelve: 1=Pomo, 2=Todo, 3=Flash, 4=Agenda, 5=Ajustes

    // --- 2. APLICACIÓN POMODORO ---
    void drawPomodoroView(const PomodoroEngine &engine, int ldrValue, bool wifiConnected);
    void updatePomodoroTimer(const PomodoroEngine &engine, int ldrValue, bool wifiConnected);
    UIButtonAction handlePomodoroTouch(int x, int y);

    // --- 3. APLICACIÓN TAREAS TO-DO ---
    void drawTodoView(const TaskManager &taskMgr);
    int handleTodoTouch(int x, int y, bool *outReset, bool *outBack);

    // --- 4. APLICACIÓN FLASHCARDS ---
    void drawFlashcardsView(const FlashcardManager &flashcards);
    bool handleFlashcardTouch(int x, int y, bool *outFlipped, bool *outEasy, bool *outHard, bool *outBack);

    // --- 5. APLICACIÓN AGENDA & RELOJ ---
    void drawAgendaView(const WiFiConfig &wifi);
    void updateAgendaClock();
    bool handleAgendaTouch(int x, int y); // true = click en Volver

    // --- 6. APLICACIÓN AJUSTES DEL SISTEMA ---
    void drawSettingsView(const WiFiConfig &wifi, int brightness, bool autoBrightness, bool soundMuted);
    int handleSettingsTouch(int x, int y, bool *outToggleAuto, bool *outBriDown, bool *outBriUp, 
                           bool *outToggleMute, bool *outOpenWifi, bool *outClearWifi, bool *outBack);

    // --- SUBVISTAS DE CONFIGURACIÓN WI-FI ---
    void drawWiFiScanView(const WiFiConfig &wifi, bool scanning, int page = 0);
    int handleWiFiScanTouch(int x, int y, int netCount, int page, bool *outRescan, bool *outBack, bool *outNextPage);
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

    void drawAppCard(int x, int y, int w, int h, uint16_t bgColor, const char *icon, const char *title, const char *subtitle);
    void drawHeaderWithBack(const char *title, uint16_t barColor = 0x1082);
    void drawTopBar(const PomodoroEngine &engine, int ldrValue, bool wifiConnected);
    void drawModeButtons(const PomodoroEngine &engine);
    void drawCountdown(const PomodoroEngine &engine);
    void drawProgressBar(const PomodoroEngine &engine);
    void drawActionButtons(const PomodoroEngine &engine);
};

#endif // UI_MANAGER_H
