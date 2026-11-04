#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <Arduino.h>
#include <TFT_eSPI.h>
#include "PomodoroEngine.h"
#include "HardwareConfig.h"
#include "WiFiConfig.h"
#include "BatteryManager.h"
#include "CalendarManager.h"
#include "StorageManager.h"
#include "WeatherManager.h"

enum AppView {
    VIEW_WEATHER = 0,     // Pantalla inicial predefinida (Standby con Clima y Reloj)
    VIEW_HOME,            // Menú lanzador de aplicaciones
    VIEW_POMODORO,        // Pomodoro con información de estudio del día
    VIEW_AGENDA,          // Agenda y calendario académico UNED Ávila
    VIEW_SETTINGS,        // Ajustes del sistema (Brillo, audio, reposo)
    VIEW_INFO,            // Información de hardware y especificaciones
    VIEW_NIGHT_CLOCK,     // Reloj nocturno minimalista
    VIEW_BREATHING,       // Respiración consciente Box Breathing
    VIEW_RGB_LAMP,        // Lámpara de escritorio RGB
    VIEW_BATTERY,         // Monitor de telemetría de batería 5000 mAh
    VIEW_STATS,           // Estadísticas de estudio en MicroSD
    VIEW_KEYBOARD_EXAM,   // Teclado virtual para añadir hito/examen
    VIEW_WIFI_SCAN,       // Escaneo de redes Wi-Fi
    VIEW_WIFI_KEYBOARD,   // Teclado para contraseña Wi-Fi
    VIEW_WIFI_CONNECTING, // Pantalla de conexión Wi-Fi
    VIEW_WIFI_RESULT,     // Resultado de conexión Wi-Fi
    VIEW_STUDY_PROMPT,    // Pantalla de cortesía 60s antes de auto-arranque Pomodoro
    VIEW_DAILY_CHECKPOINT // Pantalla de aviso de checkpoint diario (5 al día)
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
    UI_ACTION_OPEN_WIFI,
    UI_ACTION_PROMPT_START,
    UI_ACTION_PROMPT_SNOOZE,
    UI_ACTION_DISMISS_CHECKPOINT
};

class UIManager {
public:
    UIManager(TFT_eSPI &tft);

    void begin();
    
    // --- BARRA SUPERIOR UNIFICADA MINIMALISTA (STATUS BAR) ---
    void drawUnifiedStatusBar(const BatteryManager &battery, bool wifiConnected, bool audioMuted, 
                             bool autoBrightness, int streakCount, const char *appTitle = nullptr);

    // --- NUEVO LEFT CYBER DOCK (RAIL OS) ---
    void drawLeftRail(int activeTab, const BatteryManager &battery, bool wifiConnected);

    // --- ICONOS VECTORIALES A TODO COLOR ---
    void drawAppColorIcon(int cx, int cy, int appType);

    // --- 1. PANTALLA INICIAL PREDEFINIDA: ESTACIÓN CLIMA & RELOJ DESK (STANDBY) ---
    void setDeskStyle(int style) { _deskStyle = (style >= 0) ? (style % 5) : 0; }
    int getDeskStyle() const { return _deskStyle; }

    void drawWeatherView(const WeatherManager &weather, CalendarManager &calendar, 
                         const BatteryManager &battery, bool wifiConnected);
    void updateWeatherViewClock(const WeatherManager &weather, CalendarManager &calendar);
    bool handleWeatherTouch(int x, int y, bool *outOpenMenu);

    // --- 2. MENÚ LANZADOR DE APLICACIONES ---
    void drawHomeScreen(const PomodoroEngine &engine, const BatteryManager &battery, 
                        const WeatherManager &weather, bool wifiConnected, bool audioMuted, 
                        bool autoBrightness, int homeTab = 0);
    int handleHomeTouch(int x, int y, int &homeTab);

    // --- 3. APLICACIÓN POMODORO VINCULADA AL CALENDARIO ---
    void drawPomodoroView(const PomodoroEngine &engine, const CalendarManager &calendar, 
                          const BatteryManager &battery, bool wifiConnected, bool audioMuted, 
                          bool autoBrightness);
    void updatePomodoroTimer(const PomodoroEngine &engine, const CalendarManager &calendar, 
                             const BatteryManager &battery, bool wifiConnected);
    UIButtonAction handlePomodoroTouch(int x, int y);

    // --- 4. APLICACIÓN AGENDA ERGONÓMICA PARA PANTALLA 2.4" CON SCROLL Y DETALLE ---
    void drawAgendaView(CalendarManager &calendar, const BatteryManager &battery, 
                        int scrollOffset = 0, int selectedIndex = -1);
    void updateAgendaClock(CalendarManager &calendar, const WeatherManager &weather);
    bool handleAgendaTouch(int x, int y, CalendarManager &calendar, int &scrollOffset, 
                           int &selectedIndex, bool *outStartFocus, String *outFocusTitle);

    // --- 5. APLICACIÓN DEDICADA: RELOJ NOCTURNO (NIGHT DESK CLOCK) ---
    void drawNightClockView(const BatteryManager &battery, const WeatherManager &weather);
    void updateNightClock(const BatteryManager &battery, const WeatherManager &weather);
    bool handleNightClockTouch(int x, int y);

    // --- 6. APLICACIÓN DEDICADA: RESPIRACIÓN CONSCIENTE (BOX BREATHING) ---
    void drawBreathingView(int phase, int secondsLeft, int cycle, const BatteryManager &battery);
    bool handleBreathingTouch(int x, int y);

    // --- 7. APLICACIÓN DEDICADA: LÁMPARA RGB DE ESCRITORIO ---
    void drawRgbLampView(int currentMode, const BatteryManager &battery);
    int handleRgbLampTouch(int x, int y, bool *outBack);

    // --- 8. APLICACIÓN DEDICADA: TELEMETRÍA BATERÍA 5000 mAh ---
    void drawBatteryView(const BatteryManager &battery, bool wifiConnected);
    bool handleBatteryTouch(int x, int y);

    // --- 9. APLICACIÓN DEDICADA: ESTADÍSTICAS & HISTORIAL MICROSD ---
    void drawStatsView(StorageManager &storage, const PomodoroEngine &pomodoro, 
                       const BatteryManager &battery, bool wifiConnected);
    bool handleStatsTouch(int x, int y);

    // --- 10. APLICACIÓN AJUSTES DEL SISTEMA ---
    void drawSettingsView(const WiFiConfig &wifi, const BatteryManager &battery, 
                          int brightness, bool autoBrightness, bool soundMuted, int timeoutMinutes, int deskStyle = 0);
    int handleSettingsTouch(int x, int y, bool *outToggleAuto, bool *outBriDown, bool *outBriUp, 
                           bool *outToggleMute, bool *outTimeoutToggle, bool *outNightClock, bool *outOpenWifi, bool *outBack);

    // --- 11. APLICACIÓN INFO DEL SISTEMA & HARDWARE ---
    void drawInfoView(const BatteryManager &battery, StorageManager &storage, 
                      const PomodoroEngine &pomodoro, bool wifiConnected);
    bool handleInfoTouch(int x, int y);

    // --- SUBVISTAS DE CONFIGURACIÓN WI-FI ---
    void drawWiFiScanView(const WiFiConfig &wifi, const BatteryManager &battery, bool scanning, int page = 0);
    int handleWiFiScanTouch(int x, int y, int netCount, int page, bool *outRescan, bool *outBack, bool *outNextPage);
    void drawWiFiConnecting(const String &ssid);
    void drawWiFiResult(bool success, const String &info);

    // --- NUEVAS VISTAS DE DISCIPLINA Y AUTOMATIZACIÓN (CORE) ---
    void drawStudyPromptView(const String &subject, int countdownSec, const BatteryManager &battery, bool wifiConnected);
    void updateStudyPromptCountdown(int countdownSec);
    UIButtonAction handleStudyPromptTouch(int x, int y);

    void drawDailyCheckpointView(int checkpointIndex, CalendarManager &calendar, const BatteryManager &battery, bool wifiConnected);
    bool handleDailyCheckpointTouch(int x, int y);

private:
    TFT_eSPI &_tft;

    uint32_t _lastRemainingSec = 0xFFFFFFFF;
    PomodoroState _lastState = (PomodoroState)0xFF;
    uint8_t _lastCycle = 0xFF;
    uint16_t _lastFocusMin = 0xFF;
    int _lastBatteryPct = -1;
    bool _lastWifiStatus = false;
    int _selectedMonthDay = 0;

    void drawAppCard(int x, int y, int w, int h, uint16_t bgColor, int appType, const char *title, const char *subtitle);
    void drawModeButtons(const PomodoroEngine &engine);
    void drawCountdown(const PomodoroEngine &engine);
    void drawProgressBar(const PomodoroEngine &engine);
    void drawActionButtons(const PomodoroEngine &engine);

    int _deskStyle = 0; // 0=Minimalista, 1=Cyberdeck, 2=Analogico, 3=Dashboard UNED, 4=Flip Clock

    // Los 5 modelos de pantalla de escritorio predeterminada
    void drawDeskMinimal(const WeatherManager &weather, CalendarManager &calendar, const BatteryManager &battery, bool wifiConnected);
    void updateDeskMinimalClock(const WeatherManager &weather, CalendarManager &calendar);

    void drawDeskCyberdeck(const WeatherManager &weather, CalendarManager &calendar, const BatteryManager &battery, bool wifiConnected);
    void updateDeskCyberdeckClock(const WeatherManager &weather, CalendarManager &calendar);

    void drawDeskAnalog(const WeatherManager &weather, CalendarManager &calendar, const BatteryManager &battery, bool wifiConnected);
    void updateDeskAnalogClock(const WeatherManager &weather, CalendarManager &calendar);

    void drawDeskDashboard(const WeatherManager &weather, CalendarManager &calendar, const BatteryManager &battery, bool wifiConnected);
    void updateDeskDashboardClock(const WeatherManager &weather, CalendarManager &calendar);

    void drawDeskFlipClock(const WeatherManager &weather, CalendarManager &calendar, const BatteryManager &battery, bool wifiConnected);
    void updateDeskFlipClock(const WeatherManager &weather, CalendarManager &calendar);
};

#endif // UI_MANAGER_H
