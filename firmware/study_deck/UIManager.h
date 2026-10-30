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

enum AppView {
    VIEW_HOME = 0,
    VIEW_POMODORO,
    VIEW_TODO,
    VIEW_FLASH,
    VIEW_AGENDA,
    VIEW_SETTINGS,
    VIEW_INFO,
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
    
    // --- BARRA SUPERIOR UNIFICADA ESTILO ANDROID (STATUS BAR) ---
    void drawUnifiedStatusBar(const BatteryManager &battery, bool wifiConnected, bool audioMuted, 
                             bool autoBrightness, int streakCount, const char *appTitle = nullptr);

    // --- 1. PANTALLA DE INICIO (ANDROID LAUNCHER CON 6 CAJAS DE APP) ---
    void drawHomeScreen(const PomodoroEngine &engine, const TaskManager &tasks, 
                        const BatteryManager &battery, bool wifiConnected, 
                        bool audioMuted, bool autoBrightness);
    int handleHomeTouch(int x, int y); // 1=Pomo, 2=Todo, 3=Flash, 4=Agenda, 5=Ajustes, 6=Info

    // --- 2. APLICACIÓN POMODORO ---
    void drawPomodoroView(const PomodoroEngine &engine, const BatteryManager &battery, 
                          bool wifiConnected, bool audioMuted, bool autoBrightness);
    void updatePomodoroTimer(const PomodoroEngine &engine, const BatteryManager &battery, bool wifiConnected);
    UIButtonAction handlePomodoroTouch(int x, int y);

    // --- 3. APLICACIÓN TAREAS TO-DO (INTERACTIVA 100%) ---
    void drawTodoView(const TaskManager &taskMgr, const BatteryManager &battery, bool wifiConnected);
    int handleTodoTouch(int x, int y, bool *outAdd, bool *outClear, int *outDelete, bool *outBack);

    // --- 4. APLICACIÓN FLASHCARDS (LEITNER ACTIVO 100%) ---
    void drawFlashcardsView(const FlashcardManager &flashcards, const BatteryManager &battery, bool wifiConnected);
    bool handleFlashcardTouch(int x, int y, bool *outFlipped, bool *outEasy, bool *outHard, bool *outAdd, bool *outBack);

    // --- 5. APLICACIÓN AGENDA & RELOJ & EXÁMENES ---
    void drawAgendaView(CalendarManager &calendar, const WiFiConfig &wifi, const BatteryManager &battery);
    void updateAgendaClock(CalendarManager &calendar);
    bool handleAgendaTouch(int x, int y, bool *outAddExam, bool *outBack);

    // --- 6. APLICACIÓN AJUSTES DEL SISTEMA ---
    void drawSettingsView(const WiFiConfig &wifi, const BatteryManager &battery, 
                          int brightness, bool autoBrightness, bool soundMuted, int timeoutMinutes);
    int handleSettingsTouch(int x, int y, bool *outToggleAuto, bool *outBriDown, bool *outBriUp, 
                           bool *outToggleMute, bool *outTimeoutToggle, bool *outOpenWifi, bool *outBack);

    // --- 7. APLICACIÓN INFO & ESTADÍSTICAS ---
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

    // Estados previos para evitar parpadeos
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
