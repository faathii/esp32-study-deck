#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "PomodoroEngine.h"
#include "HardwareConfig.h"
#include "WiFiConfig.h"
#include "TaskManager.h"
#include "FlashcardManager.h"
#include "BatteryManager.h"
#include "CalendarManager.h"
#include "StorageManager.h"
#include "WeatherManager.h"

enum AppView {
    VIEW_HOME = 0,
    VIEW_POMODORO,
    VIEW_TODO,
    VIEW_FLASH,
    VIEW_AGENDA,
    VIEW_SETTINGS,
    VIEW_INFO,
    VIEW_NIGHT_CLOCK,
    VIEW_WEATHER,
    VIEW_BREATHING,
    VIEW_RGB_LAMP,
    VIEW_BATTERY,
    VIEW_STATS,
    VIEW_KEYBOARD_TASK,
    VIEW_KEYBOARD_CARD_Q,
    VIEW_KEYBOARD_CARD_A,
    VIEW_KEYBOARD_EXAM,
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
    UI_ACTION_SELECT_90,
    UI_ACTION_OPEN_WIFI
};

class UIManager {
public:
    UIManager(TFT_eSPI &tft);

    void begin();
    
    // --- BARRA SUPERIOR UNIFICADA MINIMALISTA (STATUS BAR) ---
    void drawUnifiedStatusBar(const BatteryManager &battery, bool wifiConnected, bool audioMuted, 
                             bool autoBrightness, int streakCount, const char *appTitle = nullptr);

    // --- 1. PANTALLA DE INICIO (LAUNCHER CON PESTAÑAS: ESTUDIO / UTILIDADES) ---
    void drawHomeScreen(const PomodoroEngine &engine, const TaskManager &tasks, 
                        const BatteryManager &battery, const WeatherManager &weather,
                        bool wifiConnected, bool audioMuted, bool autoBrightness, int homeTab = 0);
    int handleHomeTouch(int x, int y, int &homeTab);

    // --- 2. APLICACIÓN POMODORO ---
    void drawPomodoroView(const PomodoroEngine &engine, const BatteryManager &battery, 
                          bool wifiConnected, bool audioMuted, bool autoBrightness);
    void updatePomodoroTimer(const PomodoroEngine &engine, const BatteryManager &battery, bool wifiConnected);
    UIButtonAction handlePomodoroTouch(int x, int y);

    // --- 3. APLICACIÓN TAREAS TO-DO ---
    void drawTodoView(const TaskManager &taskMgr, const BatteryManager &battery, bool wifiConnected);
    int handleTodoTouch(int x, int y, bool *outAdd, bool *outClear, int *outDelete, bool *outBack);

    // --- 4. APLICACIÓN FLASHCARDS ---
    void drawFlashcardsView(const FlashcardManager &flashcards, const BatteryManager &battery, bool wifiConnected);
    bool handleFlashcardTouch(int x, int y, bool *outFlipped, bool *outEasy, bool *outHard, bool *outAdd, bool *outBack);

    // --- 5. APLICACIÓN AGENDA & GOOGLE CALENDAR & EXÁMENES ---
    void drawAgendaView(CalendarManager &calendar, const WiFiConfig &wifi, 
                        const BatteryManager &battery, const WeatherManager &weather);
    void updateAgendaClock(CalendarManager &calendar, const WeatherManager &weather);
    bool handleAgendaTouch(int x, int y, CalendarManager &calendar, bool *outAddExam, bool *outSyncGCal, bool *outBack);

    // --- 6. APLICACIÓN DEDICADA: ESTACIÓN DE CLIMA (OPEN-METEO) ---
    void drawWeatherView(const WeatherManager &weather, const BatteryManager &battery, bool wifiConnected);
    bool handleWeatherTouch(int x, int y, bool *outRefresh, bool *outBack);

    // --- 7. APLICACIÓN DEDICADA: RELOJ NOCTURNO (NIGHT DESK CLOCK) ---
    void drawNightClockView(const BatteryManager &battery, const WeatherManager &weather);
    void updateNightClock(const BatteryManager &battery, const WeatherManager &weather);
    bool handleNightClockTouch(int x, int y);

    // --- 8. APLICACIÓN DEDICADA: RESPIRACIÓN CONSCIENTE (BOX BREATHING) ---
    void drawBreathingView(int phase, int secondsLeft, int cycle, const BatteryManager &battery);
    bool handleBreathingTouch(int x, int y);

    // --- 9. APLICACIÓN DEDICADA: LÁMPARA RGB DE ESCRITORIO ---
    void drawRgbLampView(int currentMode, const BatteryManager &battery);
    int handleRgbLampTouch(int x, int y, bool *outBack);

    // --- 10. APLICACIÓN DEDICADA: TELEMETRÍA BATERÍA 5000 mAh ---
    void drawBatteryView(const BatteryManager &battery, bool wifiConnected);
    bool handleBatteryTouch(int x, int y);

    // --- 11. APLICACIÓN DEDICADA: ESTADÍSTICAS & HISTORIAL MICROSD ---
    void drawStatsView(StorageManager &storage, const PomodoroEngine &pomodoro, const BatteryManager &battery, bool wifiConnected);
    bool handleStatsTouch(int x, int y);

    // --- 12. APLICACIÓN AJUSTES DEL SISTEMA ---
    void drawSettingsView(const WiFiConfig &wifi, const BatteryManager &battery, 
                          int brightness, bool autoBrightness, bool soundMuted, int timeoutMinutes);
    int handleSettingsTouch(int x, int y, bool *outToggleAuto, bool *outBriDown, bool *outBriUp, 
                           bool *outToggleMute, bool *outTimeoutToggle, bool *outNightClock, bool *outOpenWifi, bool *outBack);

    // --- 13. APLICACIÓN INFO DEL SISTEMA & HARDWARE ---
    void drawInfoView(const BatteryManager &battery, StorageManager &storage, 
                      const PomodoroEngine &pomodoro, bool wifiConnected);
    bool handleInfoTouch(int x, int y);

    // --- SUBVISTAS DE CONFIGURACIÓN WI-FI ---
    void drawWiFiScanView(const WiFiConfig &wifi, const BatteryManager &battery, bool scanning, int page = 0);
    int handleWiFiScanTouch(int x, int y, int netCount, int page, bool *outRescan, bool *outBack, bool *outNextPage);
    void drawWiFiConnecting(const String &ssid);
    void drawWiFiResult(bool success, const String &info);

private:
    TFT_eSPI &_tft;

    uint32_t _lastRemainingSec = 0xFFFFFFFF;
    PomodoroState _lastState = (PomodoroState)0xFF;
    uint8_t _lastCycle = 0xFF;
    uint16_t _lastFocusMin = 0xFF;
    int _lastBatteryPct = -1;
    bool _lastWifiStatus = false;

    void drawAppCard(int x, int y, int w, int h, uint16_t bgColor, const char *icon, const char *title, const char *subtitle);
    void drawModeButtons(const PomodoroEngine &engine);
    void drawCountdown(const PomodoroEngine &engine);
    void drawProgressBar(const PomodoroEngine &engine);
    void drawActionButtons(const PomodoroEngine &engine);
};

#endif // UI_MANAGER_H
