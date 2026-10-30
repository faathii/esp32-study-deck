#include <Arduino.h>
#include <TFT_eSPI.h>
#include <time.h>
#include "HardwareConfig.h"
#include "CST820.h"
#include "AudioNotifier.h"
#include "LedIndicator.h"
#include "PomodoroEngine.h"
#include "UIManager.h"
#include "WiFiConfig.h"
#include "KeyboardUI.h"
#include "TaskManager.h"
#include "FlashcardManager.h"

// Periféricos y componentes
TFT_eSPI tft = TFT_eSPI();
CST820 touch(I2C_SDA_PIN, I2C_SCL_PIN, TP_RST_PIN, TP_INT_PIN);
AudioNotifier audio;
LedIndicator led;
PomodoroEngine pomodoro;
UIManager ui(tft);
WiFiConfig wifi;
KeyboardUI keyboard(tft);
TaskManager taskMgr;
FlashcardManager flashcards;

// Estado de la interfaz (inicia en el Launcher estilo Android)
AppView currentView = VIEW_HOME;
int scanPage = 0;
String selectedSSID = "";
String typedPassword = "";
unsigned long resultStartTime = 0;
bool connectionSuccess = false;

// Control de hardware y tiempos
unsigned long lastLdrCheck = 0;
unsigned long lastTouchTime = 0;
unsigned long lastClockUpdate = 0;
int lastHomeMin = -1;
int currentLdr = 2000;
int currentBrightness = 220;
bool autoBrightness = true;

void refreshCurrentView() {
    switch (currentView) {
        case VIEW_HOME:
            ui.drawHomeScreen(pomodoro, taskMgr, wifi.isConnected(), currentLdr);
            break;
        case VIEW_POMODORO:
            ui.drawPomodoroView(pomodoro, currentLdr, wifi.isConnected());
            break;
        case VIEW_TODO:
            ui.drawTodoView(taskMgr);
            break;
        case VIEW_FLASH:
            ui.drawFlashcardsView(flashcards);
            break;
        case VIEW_AGENDA:
            ui.drawAgendaView(wifi);
            break;
        case VIEW_SETTINGS:
            ui.drawSettingsView(wifi, currentBrightness, autoBrightness, audio.isMuted());
            break;
        case VIEW_WIFI_SCAN:
            ui.drawWiFiScanView(wifi, false, scanPage);
            break;
        default:
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("\n==============================================");
    Serial.println("  ESTACION DE ESTUDIO INTELIGENTE - STUDY DECK");
    Serial.println("  Hardware: Guition ESP32-2432S024C (2.4\")");
    Serial.println("  Interfaz: Android App Launcher Grid");
    Serial.println("==============================================\n");

    // 1. Inicializar retroiluminación con nivel confortable
    pinMode(PIN_BACKLIGHT, OUTPUT);
    analogWrite(PIN_BACKLIGHT, currentBrightness);

    // 2. Inicializar sensor LDR
    pinMode(PIN_LDR, INPUT);
    currentLdr = analogRead(PIN_LDR);

    // 3. Inicializar audio y LED RGB
    audio.begin();
    led.begin();
    led.setMode(LED_MODE_IDLE);

    // 4. Inicializar motores de estudio
    pomodoro.begin();
    taskMgr.begin();
    flashcards.begin();

    // 5. Inicializar subsistema Wi-Fi
    wifi.begin();

    // 6. Inicializar pantalla táctil y dibujar Home Launcher
    ui.begin();
    touch.begin();
    ui.drawHomeScreen(pomodoro, taskMgr, wifi.isConnected(), currentLdr);

    // 7. Auto-conexión si hay credenciales guardadas en NVS
    if (wifi.hasSavedCredentials()) {
        Serial.printf("[WiFi] Auto-conectando a '%s' guardada en NVS...\n", wifi.getSavedSSID().c_str());
        wifi.startConnect(wifi.getSavedSSID(), wifi.getSavedPassword());
    }

    // Sonido de inicio
    audio.playStart();
    Serial.println("[Sistema] Study Deck listo y a la espera de interaccion.");
}

void loop() {
    // 1. Actualización no bloqueante del LED RGB
    led.update();

    // 2. Comprobar estado de conexión Wi-Fi en segundo plano
    if (currentView == VIEW_WIFI_CONNECTING) {
        WiFiConnectStatus st = wifi.checkConnectStatus();
        if (st == WIFI_STATUS_CONNECTED) {
            Serial.printf("[WiFi] Conectado con exito. IP: %s\n", wifi.getLocalIP().c_str());
            audio.playWorkFinished();
            wifi.saveCredentials(selectedSSID, typedPassword);
            
            // Sincronizar reloj por NTP (Hora peninsular España: UTC+1, 3600s daylight)
            configTime(3600, 3600, "pool.ntp.org", "time.nist.gov");
            
            currentView = VIEW_WIFI_RESULT;
            connectionSuccess = true;
            ui.drawWiFiResult(true, wifi.getLocalIP());
            resultStartTime = millis();
        } else if (st == WIFI_STATUS_FAILED) {
            Serial.println("[WiFi] Error: tiempo de espera agotado al conectar.");
            audio.playPause();
            currentView = VIEW_WIFI_RESULT;
            connectionSuccess = false;
            ui.drawWiFiResult(false, "Revisa la clave o la distancia.");
            resultStartTime = millis();
        }
    } else if (currentView == VIEW_WIFI_RESULT) {
        if (connectionSuccess && (millis() - resultStartTime >= 2500)) {
            currentView = VIEW_SETTINGS;
            refreshCurrentView();
        }
    }

    // 3. Actualización de la máquina de estados Pomodoro (siempre activa en segundo plano)
    PomodoroEvent event = pomodoro.update();
    if (event == POMO_EVENT_FOCUS_COMPLETE) {
        Serial.println("[Pomodoro] ¡Bloque de estudio completado! Iniciando descanso.");
        audio.playWorkFinished();
        led.setMode(LED_MODE_BREAK);
        if (currentView == VIEW_POMODORO || currentView == VIEW_HOME) {
            refreshCurrentView();
        }
    } else if (event == POMO_EVENT_BREAK_COMPLETE) {
        Serial.println("[Pomodoro] ¡Descanso finalizado! Volviendo al modo enfoque.");
        audio.playBreakFinished();
        led.setMode(LED_MODE_FOCUS);
        if (currentView == VIEW_POMODORO || currentView == VIEW_HOME) {
            refreshCurrentView();
        }
    }

    // 4. Lectura táctil
    uint16_t rawX = 0, rawY = 0;
    uint8_t gesture = GESTURE_NONE;
    if (touch.getRawTouch(&rawX, &rawY, &gesture)) {
        if (millis() - lastTouchTime >= 200) {
            lastTouchTime = millis();

            // Transformar a coordenadas horizontales (320x240)
            int touchX = rawY;
            int touchY = 240 - rawX;
            if (touchX < 0) touchX = 0;
            if (touchX >= 320) touchX = 319;
            if (touchY < 0) touchY = 0;
            if (touchY >= 240) touchY = 239;

            // --- VISTA 0: LAUNCHER HOME ESTILO ANDROID ---
            if (currentView == VIEW_HOME) {
                int appSelected = ui.handleHomeTouch(touchX, touchY);
                if (appSelected == 1) { // Pomodoro
                    audio.playClick();
                    currentView = VIEW_POMODORO;
                    refreshCurrentView();
                } else if (appSelected == 2) { // Tareas
                    audio.playClick();
                    currentView = VIEW_TODO;
                    refreshCurrentView();
                } else if (appSelected == 3) { // Fichas Flashcards
                    audio.playClick();
                    currentView = VIEW_FLASH;
                    refreshCurrentView();
                } else if (appSelected == 4) { // Agenda & Reloj
                    audio.playClick();
                    currentView = VIEW_AGENDA;
                    refreshCurrentView();
                } else if (appSelected == 5) { // Ajustes del Sistema
                    audio.playClick();
                    currentView = VIEW_SETTINGS;
                    refreshCurrentView();
                }
            }
            // --- VISTA 1: APLICACIÓN POMODORO ---
            else if (currentView == VIEW_POMODORO) {
                UIButtonAction action = ui.handlePomodoroTouch(touchX, touchY);
                switch (action) {
                    case UI_ACTION_BACK_HOME:
                        audio.playClick();
                        currentView = VIEW_HOME;
                        refreshCurrentView();
                        break;

                    case UI_ACTION_OPEN_WIFI:
                        audio.playClick();
                        currentView = VIEW_SETTINGS;
                        refreshCurrentView();
                        break;

                    case UI_ACTION_PLAY_PAUSE:
                        pomodoro.togglePlayPause();
                        if (pomodoro.isRunning()) {
                            audio.playStart();
                            led.setMode(pomodoro.isFocus() ? LED_MODE_FOCUS : LED_MODE_BREAK);
                        } else if (pomodoro.isPaused()) {
                            audio.playPause();
                            led.setMode(LED_MODE_PAUSED);
                        }
                        break;

                    case UI_ACTION_RESET:
                        audio.playClick();
                        pomodoro.reset();
                        led.setMode(LED_MODE_IDLE);
                        break;

                    case UI_ACTION_SKIP:
                        audio.playSkip();
                        pomodoro.skip();
                        led.setMode(pomodoro.isFocus() ? LED_MODE_FOCUS : LED_MODE_BREAK);
                        break;

                    case UI_ACTION_SELECT_25:
                        if (!pomodoro.isRunning()) {
                            audio.playClick();
                            pomodoro.setFocusMinutes(25);
                        }
                        break;

                    case UI_ACTION_SELECT_50:
                        if (!pomodoro.isRunning()) {
                            audio.playClick();
                            pomodoro.setFocusMinutes(50);
                        }
                        break;

                    default:
                        break;
                }
            } 
            // --- VISTA 2: APLICACIÓN TAREAS TO-DO ---
            else if (currentView == VIEW_TODO) {
                bool resetTasks = false, back = false;
                int taskIdx = ui.handleTodoTouch(touchX, touchY, &resetTasks, &back);
                if (back) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    refreshCurrentView();
                } else if (resetTasks) {
                    audio.playClick();
                    taskMgr.resetToDefaults();
                    ui.drawTodoView(taskMgr);
                } else if (taskIdx >= 0) {
                    audio.playClick();
                    taskMgr.toggleTask(taskIdx);
                    ui.drawTodoView(taskMgr);
                }
            } 
            // --- VISTA 3: APLICACIÓN FLASHCARDS ---
            else if (currentView == VIEW_FLASH) {
                bool flipped = false, easy = false, hard = false, back = false;
                if (ui.handleFlashcardTouch(touchX, touchY, &flipped, &easy, &hard, &back)) {
                    if (back) {
                        audio.playClick();
                        currentView = VIEW_HOME;
                        refreshCurrentView();
                    } else if (flipped) {
                        audio.playClick();
                        flashcards.flip();
                        ui.drawFlashcardsView(flashcards);
                    } else if (easy) {
                        audio.playWorkFinished();
                        flashcards.nextCard(true);
                        ui.drawFlashcardsView(flashcards);
                    } else if (hard) {
                        audio.playPause();
                        flashcards.nextCard(false);
                        ui.drawFlashcardsView(flashcards);
                    }
                }
            } 
            // --- VISTA 4: APLICACIÓN AGENDA & RELOJ ---
            else if (currentView == VIEW_AGENDA) {
                if (ui.handleAgendaTouch(touchX, touchY)) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    refreshCurrentView();
                } else {
                    ui.drawAgendaView(wifi);
                }
            } 
            // --- VISTA 5: APLICACIÓN AJUSTES DEL SISTEMA ---
            else if (currentView == VIEW_SETTINGS) {
                bool toggleAuto = false, briDown = false, briUp = false;
                bool toggleMute = false, openWifi = false, clearWifi = false, back = false;
                ui.handleSettingsTouch(touchX, touchY, &toggleAuto, &briDown, &briUp, 
                                       &toggleMute, &openWifi, &clearWifi, &back);

                if (back) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    refreshCurrentView();
                } else if (toggleAuto) {
                    autoBrightness = !autoBrightness;
                    audio.playClick();
                    ui.drawSettingsView(wifi, currentBrightness, autoBrightness, audio.isMuted());
                } else if (briDown) {
                    autoBrightness = false;
                    currentBrightness = max(40, currentBrightness - 25);
                    analogWrite(PIN_BACKLIGHT, currentBrightness);
                    audio.playClick();
                    ui.drawSettingsView(wifi, currentBrightness, autoBrightness, audio.isMuted());
                } else if (briUp) {
                    autoBrightness = false;
                    currentBrightness = min(255, currentBrightness + 25);
                    analogWrite(PIN_BACKLIGHT, currentBrightness);
                    audio.playClick();
                    ui.drawSettingsView(wifi, currentBrightness, autoBrightness, audio.isMuted());
                } else if (toggleMute) {
                    audio.toggleMute();
                    if (!audio.isMuted()) audio.playClick();
                    ui.drawSettingsView(wifi, currentBrightness, autoBrightness, audio.isMuted());
                } else if (openWifi) {
                    audio.playClick();
                    currentView = VIEW_WIFI_SCAN;
                    ui.drawWiFiScanView(wifi, true);
                    wifi.scanNetworks();
                    scanPage = 0;
                    ui.drawWiFiScanView(wifi, false, scanPage);
                }
            }
            // --- VISTAS WI-FI CONFIG ---
            else if (currentView == VIEW_WIFI_SCAN) {
                bool rescan = false, back = false, nextPage = false;
                int selectedIdx = ui.handleWiFiScanTouch(touchX, touchY, wifi.getNetworkCount(), scanPage, &rescan, &back, &nextPage);

                if (back) {
                    audio.playClick();
                    currentView = VIEW_SETTINGS;
                    refreshCurrentView();
                } else if (rescan) {
                    audio.playClick();
                    ui.drawWiFiScanView(wifi, true);
                    wifi.scanNetworks();
                    scanPage = 0;
                    ui.drawWiFiScanView(wifi, false, scanPage);
                } else if (nextPage) {
                    audio.playClick();
                    int totalP = (wifi.getNetworkCount() + 3) / 4;
                    if (totalP > 0) {
                        scanPage = (scanPage + 1) % totalP;
                    }
                    ui.drawWiFiScanView(wifi, false, scanPage);
                } else if (selectedIdx >= 0) {
                    audio.playClick();
                    selectedSSID = wifi.getNetwork(selectedIdx).ssid;
                    currentView = VIEW_WIFI_KEYBOARD;
                    keyboard.reset(selectedSSID);
                    keyboard.drawFull();
                }
            } 
            else if (currentView == VIEW_WIFI_KEYBOARD) {
                KeyAction kAction = keyboard.handleTouch(touchX, touchY);
                if (kAction == KEY_ACTION_CHAR_TYPED || kAction == KEY_ACTION_DELETE) {
                    audio.playClick();
                } else if (kAction == KEY_ACTION_BACK) {
                    audio.playClick();
                    currentView = VIEW_WIFI_SCAN;
                    ui.drawWiFiScanView(wifi, false, scanPage);
                } else if (kAction == KEY_ACTION_SUBMIT) {
                    audio.playStart();
                    typedPassword = keyboard.getText();
                    currentView = VIEW_WIFI_CONNECTING;
                    ui.drawWiFiConnecting(selectedSSID);
                    wifi.startConnect(selectedSSID, typedPassword);
                }
            } 
            else if (currentView == VIEW_WIFI_RESULT) {
                audio.playClick();
                if (!connectionSuccess) {
                    currentView = VIEW_WIFI_SCAN;
                    ui.drawWiFiScanView(wifi, false, scanPage);
                } else {
                    currentView = VIEW_SETTINGS;
                    refreshCurrentView();
                }
            }
        }
    }

    // 5. Actualizaciones periódicas de visualización según la vista activa
    if (currentView == VIEW_HOME) {
        time_t now = time(nullptr);
        struct tm *t = localtime(&now);
        if (t->tm_min != lastHomeMin) {
            lastHomeMin = t->tm_min;
            ui.drawHomeScreen(pomodoro, taskMgr, wifi.isConnected(), currentLdr);
        }
    } else if (currentView == VIEW_POMODORO) {
        ui.updatePomodoroTimer(pomodoro, currentLdr, wifi.isConnected());
    } else if (currentView == VIEW_AGENDA) {
        if (millis() - lastClockUpdate >= 1000) {
            lastClockUpdate = millis();
            ui.updateAgendaClock();
        }
    }

    // 6. Sensor LDR y ajuste suave de retroiluminación cada 400 ms si autoBrightness está activo
    if (autoBrightness && (millis() - lastLdrCheck >= 400)) {
        lastLdrCheck = millis();
        currentLdr = analogRead(PIN_LDR);

        int targetBrightness = map(currentLdr, 4000, 500, 70, 255);
        targetBrightness = constrain(targetBrightness, 70, 255);

        if (abs(targetBrightness - currentBrightness) > 8) {
            currentBrightness += (targetBrightness > currentBrightness) ? 5 : -5;
            analogWrite(PIN_BACKLIGHT, currentBrightness);
        }
    }
}
