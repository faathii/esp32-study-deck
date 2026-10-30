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
#include "BatteryManager.h"
#include "CalendarManager.h"
#include "StorageManager.h"
#include "WeatherManager.h"

// Periféricos y controladores de hardware
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
BatteryManager battery(PIN_BATTERY);
CalendarManager calendar;
StorageManager storage;
WeatherManager weather;

// Estado de la interfaz
AppView currentView = VIEW_HOME;
int homeTab = 0; // 0 = Estudio (6 apps), 1 = Utilidades Desk (6 apps)
int scanPage = 0;
String selectedSSID = "";
String typedPassword = "";
String newCardQuestion = "";
unsigned long resultStartTime = 0;
bool connectionSuccess = false;

// Estado de Lámpara RGB y Respiración
int lampMode = 0; // 0 = Apagado, 1 = Blanco Cálido, 2 = Cian, 3 = Azul, 4 = Vela
int breathingPhase = 0;
int breathingSec = 4;
int breathingCycle = 1;
unsigned long lastBreathingTick = 0;
unsigned long lastCandleTick = 0;
bool candleFlickerState = false;

// Configuración de pantalla y ahorro de energía
unsigned long lastLdrCheck = 0;
unsigned long lastTouchTime = 0;
unsigned long lastClockUpdate = 0;
int lastHomeMin = -1;
int currentLdr = 2000;
int currentBrightness = 220;
int targetBrightness = 220;
bool autoBrightness = true;
int timeoutMinutes = 2; // 1, 2, 5 o 0 (desactivado)
bool screenDimmed = false;

void refreshCurrentView() {
    switch (currentView) {
        case VIEW_HOME:
            ui.drawHomeScreen(pomodoro, taskMgr, battery, weather, wifi.isConnected(), audio.isMuted(), autoBrightness, homeTab);
            break;
        case VIEW_POMODORO:
            ui.drawPomodoroView(pomodoro, battery, wifi.isConnected(), audio.isMuted(), autoBrightness);
            break;
        case VIEW_TODO:
            ui.drawTodoView(taskMgr, battery, wifi.isConnected());
            break;
        case VIEW_FLASH:
            ui.drawFlashcardsView(flashcards, battery, wifi.isConnected());
            break;
        case VIEW_AGENDA:
            ui.drawAgendaView(calendar, wifi, battery, weather);
            break;
        case VIEW_WEATHER:
            ui.drawWeatherView(weather, battery, wifi.isConnected());
            break;
        case VIEW_NIGHT_CLOCK:
            ui.drawNightClockView(battery, weather);
            break;
        case VIEW_BREATHING:
            ui.drawBreathingView(breathingPhase, breathingSec, breathingCycle, battery);
            break;
        case VIEW_RGB_LAMP:
            ui.drawRgbLampView(lampMode, battery);
            break;
        case VIEW_BATTERY:
            ui.drawBatteryView(battery, wifi.isConnected());
            break;
        case VIEW_STATS:
            ui.drawStatsView(storage, pomodoro, battery, wifi.isConnected());
            break;
        case VIEW_SETTINGS:
            ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness, audio.isMuted(), timeoutMinutes);
            break;
        case VIEW_INFO:
            ui.drawInfoView(battery, storage, pomodoro, wifi.isConnected());
            break;
        case VIEW_WIFI_SCAN:
            ui.drawWiFiScanView(wifi, battery, false, scanPage);
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
    Serial.println("  Bateria: LiPo 5000 mAh 3.7V (18.5 Wh)");
    Serial.println("  Clima: Open-Meteo + Reloj Noche + Relax");
    Serial.println("==============================================\n");

    // 1. Inicializar retroiluminación
    pinMode(PIN_BACKLIGHT, OUTPUT);
    analogWrite(PIN_BACKLIGHT, currentBrightness);

    // 2. Inicializar sensor LDR y Monitor de Batería 5000 mAh
    pinMode(PIN_LDR, INPUT);
    currentLdr = analogRead(PIN_LDR);
    battery.begin();

    // 3. Inicializar audio y LED RGB
    audio.begin();
    led.begin();
    led.setMode(LED_MODE_IDLE);

    // 4. Inicializar MicroSD, Estudio, Agenda y Clima
    storage.begin();
    pomodoro.begin();
    taskMgr.begin();
    flashcards.begin();
    calendar.begin();
    weather.begin();

    // 5. Inicializar Wi-Fi
    wifi.begin();

    // 6. Inicializar pantalla táctil y dibujar Home Launcher
    ui.begin();
    touch.begin();

    lastTouchTime = millis();
    refreshCurrentView();
}

void loop() {
    // 1. Motores lógicos
    PomodoroEvent pomoEv = pomodoro.update();
    if (pomoEv == POMO_EVENT_FOCUS_COMPLETE) {
        audio.playWorkFinished();
        led.setMode(LED_MODE_BREAK);
        storage.logSession("Enfoque", pomodoro.getFocusMinutes(), true);
    } else if (pomoEv == POMO_EVENT_BREAK_COMPLETE) {
        audio.playBreakFinished();
        led.setMode(LED_MODE_FOCUS);
    }

    led.update();
    battery.update();
    weather.update(wifi.isConnected());

    // 2. Animación de Respiración Box Breathing (4-4-4-4)
    if (currentView == VIEW_BREATHING) {
        if (millis() - lastBreathingTick >= 1000) {
            lastBreathingTick = millis();
            breathingSec--;
            if (breathingSec <= 0) {
                breathingSec = 4;
                breathingPhase = (breathingPhase + 1) % 4;
                if (breathingPhase == 0) {
                    breathingCycle = (breathingCycle % 4) + 1;
                }
                // Sincronizar color de LED calmante
                if (breathingPhase == 0)      led.setRaw(false, true, true);  // Inhala: Cian suave
                else if (breathingPhase == 1) led.setRaw(false, true, true);  // Mantén: Cian
                else if (breathingPhase == 2) led.setRaw(false, false, true); // Exhala: Azul profundo
                else                          led.setRaw(false, false, false);// Pausa: Apagado
            }
            ui.drawBreathingView(breathingPhase, breathingSec, breathingCycle, battery);
        }
    }

    // 3. Efecto parpadeo suave vela en modo lámpara vela
    if (lampMode == 4 && (currentView == VIEW_RGB_LAMP || currentView == VIEW_HOME)) {
        if (millis() - lastCandleTick >= 120) {
            lastCandleTick = millis();
            candleFlickerState = !candleFlickerState;
            if (candleFlickerState) {
                led.setRaw(true, true, false); // Ámbar cálido
            } else {
                led.setRaw(true, false, false); // Oscilación suave
            }
        }
    }

    // 4. Gestión de conexión Wi-Fi asíncrona
    if (currentView == VIEW_WIFI_CONNECTING) {
        WiFiConnectStatus status = wifi.checkConnectStatus();
        if (status == WIFI_STATUS_CONNECTED) {
            connectionSuccess = true;
            configTime(0, 0, "pool.ntp.org", "time.google.com");
            weather.update(true); // Forzar descarga inmediata de clima
            currentView = VIEW_WIFI_RESULT;
            ui.drawWiFiResult(true, wifi.getLocalIP());
            resultStartTime = millis();
        } else if (status == WIFI_STATUS_FAILED) {
            connectionSuccess = false;
            currentView = VIEW_WIFI_RESULT;
            ui.drawWiFiResult(false, "Clave incorrecta o tiempo agotado");
            resultStartTime = millis();
        }
    }

    // 5. Apagado / atenuación suave de pantalla por inactividad
    if (timeoutMinutes > 0 && currentView != VIEW_NIGHT_CLOCK) {
        unsigned long maxIdleMs = (unsigned long)timeoutMinutes * 60000;
        if ((millis() - lastTouchTime > maxIdleMs) && !screenDimmed) {
            screenDimmed = true;
            analogWrite(PIN_BACKLIGHT, 30);
        }
    }

    // 6. Lectura táctil capacitiva CST820
    uint16_t rawX = 0, rawY = 0;
    uint8_t gesture = GESTURE_NONE;
    if (touch.getRawTouch(&rawX, &rawY, &gesture)) {
        // Despertar pantalla si estaba en reposo
        if (screenDimmed) {
            screenDimmed = false;
            analogWrite(PIN_BACKLIGHT, currentBrightness);
            lastTouchTime = millis();
            return;
        }

        // Salir de Modo Reloj Nocturno al tocar
        if (currentView == VIEW_NIGHT_CLOCK) {
            audio.playClick();
            currentView = VIEW_HOME;
            analogWrite(PIN_BACKLIGHT, currentBrightness);
            refreshCurrentView();
            lastTouchTime = millis();
            return;
        }

        if (millis() - lastTouchTime >= 180) {
            lastTouchTime = millis();

            // Coordenadas apaisadas (320x240)
            int touchX = rawY;
            int touchY = 240 - rawX;
            if (touchX < 0) touchX = 0;
            if (touchX >= 320) touchX = 319;
            if (touchY < 0) touchY = 0;
            if (touchY >= 240) touchY = 239;

            // --- VISTA 0: LAUNCHER HOME CON PESTAÑAS (12 APPS) ---
            if (currentView == VIEW_HOME) {
                int appSelected = ui.handleHomeTouch(touchX, touchY, homeTab);
                if (appSelected == -10) {
                    audio.playClick();
                    refreshCurrentView();
                } else if (appSelected == 1) { // ⏱️ Pomodoro
                    audio.playClick();
                    currentView = VIEW_POMODORO;
                    refreshCurrentView();
                } else if (appSelected == 2) { // 📝 Tareas
                    audio.playClick();
                    currentView = VIEW_TODO;
                    refreshCurrentView();
                } else if (appSelected == 3) { // 💡 Fichas Flashcards
                    audio.playClick();
                    currentView = VIEW_FLASH;
                    refreshCurrentView();
                } else if (appSelected == 4) { // 📅 Agenda & Exámenes
                    audio.playClick();
                    currentView = VIEW_AGENDA;
                    refreshCurrentView();
                } else if (appSelected == 5) { // ⚙️ Ajustes del Sistema
                    audio.playClick();
                    currentView = VIEW_SETTINGS;
                    refreshCurrentView();
                } else if (appSelected == 6) { // ℹ️ Info & Hardware
                    audio.playClick();
                    currentView = VIEW_INFO;
                    refreshCurrentView();
                } else if (appSelected == 7) { // 📊 Stats & MicroSD
                    audio.playClick();
                    currentView = VIEW_STATS;
                    refreshCurrentView();
                } else if (appSelected == 8) { // 🌤️ Estación Clima Open-Meteo
                    audio.playClick();
                    currentView = VIEW_WEATHER;
                    refreshCurrentView();
                } else if (appSelected == 9) { // 🌙 Reloj Nocturno
                    audio.playClick();
                    currentView = VIEW_NIGHT_CLOCK;
                    analogWrite(PIN_BACKLIGHT, 25);
                    ui.drawNightClockView(battery, weather);
                } else if (appSelected == 10) { // 🫁 Respiración Consciente Box
                    audio.playClick();
                    currentView = VIEW_BREATHING;
                    breathingPhase = 0;
                    breathingSec = 4;
                    breathingCycle = 1;
                    led.setRaw(false, true, true);
                    ui.drawBreathingView(breathingPhase, breathingSec, breathingCycle, battery);
                } else if (appSelected == 11) { // 💡 Lámpara RGB de Escritorio
                    audio.playClick();
                    currentView = VIEW_RGB_LAMP;
                    refreshCurrentView();
                } else if (appSelected == 12) { // 🔋 Telemetría Batería 5000 mAh
                    audio.playClick();
                    currentView = VIEW_BATTERY;
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
                    case UI_ACTION_SELECT_90:
                        if (!pomodoro.isRunning()) {
                            audio.playClick();
                            pomodoro.setFocusMinutes(90);
                        }
                        break;
                    default:
                        break;
                }
            } 
            // --- VISTA 2: APLICACIÓN TAREAS TO-DO ---
            else if (currentView == VIEW_TODO) {
                bool add = false, clear = false, back = false;
                int delIdx = -1;
                int taskIdx = ui.handleTodoTouch(touchX, touchY, &add, &clear, &delIdx, &back);
                
                if (back) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    refreshCurrentView();
                } else if (add) {
                    audio.playClick();
                    currentView = VIEW_KEYBOARD_TASK;
                    keyboard.reset("Escribe la nueva tarea:", false, "");
                    keyboard.drawFull();
                } else if (clear) {
                    audio.playClick();
                    taskMgr.clearCompleted();
                    ui.drawTodoView(taskMgr, battery, wifi.isConnected());
                } else if (delIdx >= 0) {
                    audio.playPause();
                    taskMgr.deleteTask(delIdx);
                    ui.drawTodoView(taskMgr, battery, wifi.isConnected());
                } else if (taskIdx >= 0) {
                    audio.playClick();
                    taskMgr.toggleTask(taskIdx);
                    ui.drawTodoView(taskMgr, battery, wifi.isConnected());
                }
            }
            // --- ENTRADA DE TEXTO PARA NUEVA TAREA ---
            else if (currentView == VIEW_KEYBOARD_TASK) {
                KeyAction kAction = keyboard.handleTouch(touchX, touchY);
                if (kAction == KEY_ACTION_CHAR_TYPED || kAction == KEY_ACTION_DELETE) {
                    audio.playClick();
                } else if (kAction == KEY_ACTION_BACK) {
                    audio.playClick();
                    currentView = VIEW_TODO;
                    refreshCurrentView();
                } else if (kAction == KEY_ACTION_SUBMIT) {
                    audio.playWorkFinished();
                    taskMgr.addTask(keyboard.getText());
                    currentView = VIEW_TODO;
                    refreshCurrentView();
                }
            }
            // --- VISTA 3: APLICACIÓN FLASHCARDS ---
            else if (currentView == VIEW_FLASH) {
                bool flipped = false, easy = false, hard = false, add = false, back = false;
                if (ui.handleFlashcardTouch(touchX, touchY, &flipped, &easy, &hard, &add, &back)) {
                    if (back) {
                        audio.playClick();
                        currentView = VIEW_HOME;
                        refreshCurrentView();
                    } else if (add) {
                        audio.playClick();
                        currentView = VIEW_KEYBOARD_CARD_Q;
                        keyboard.reset("Pregunta de la ficha:", false, "");
                        keyboard.drawFull();
                    } else if (flipped) {
                        audio.playClick();
                        flashcards.flip();
                        ui.drawFlashcardsView(flashcards, battery, wifi.isConnected());
                    } else if (easy) {
                        audio.playWorkFinished();
                        flashcards.nextCard(true);
                        ui.drawFlashcardsView(flashcards, battery, wifi.isConnected());
                    } else if (hard) {
                        audio.playPause();
                        flashcards.nextCard(false);
                        ui.drawFlashcardsView(flashcards, battery, wifi.isConnected());
                    }
                }
            } 
            // --- ENTRADA DE TEXTO PARA FLASHCARD (PREGUNTA) ---
            else if (currentView == VIEW_KEYBOARD_CARD_Q) {
                KeyAction kAction = keyboard.handleTouch(touchX, touchY);
                if (kAction == KEY_ACTION_CHAR_TYPED || kAction == KEY_ACTION_DELETE) {
                    audio.playClick();
                } else if (kAction == KEY_ACTION_BACK) {
                    audio.playClick();
                    currentView = VIEW_FLASH;
                    refreshCurrentView();
                } else if (kAction == KEY_ACTION_SUBMIT) {
                    audio.playClick();
                    newCardQuestion = keyboard.getText();
                    currentView = VIEW_KEYBOARD_CARD_A;
                    keyboard.reset("Respuesta de la ficha:", false, "");
                    keyboard.drawFull();
                }
            }
            // --- ENTRADA DE TEXTO PARA FLASHCARD (RESPUESTA) ---
            else if (currentView == VIEW_KEYBOARD_CARD_A) {
                KeyAction kAction = keyboard.handleTouch(touchX, touchY);
                if (kAction == KEY_ACTION_CHAR_TYPED || kAction == KEY_ACTION_DELETE) {
                    audio.playClick();
                } else if (kAction == KEY_ACTION_BACK) {
                    audio.playClick();
                    currentView = VIEW_FLASH;
                    refreshCurrentView();
                } else if (kAction == KEY_ACTION_SUBMIT) {
                    audio.playWorkFinished();
                    flashcards.addCard(newCardQuestion, keyboard.getText(), "Personal");
                    currentView = VIEW_FLASH;
                    refreshCurrentView();
                }
            }
            // --- VISTA 4: APLICACIÓN AGENDA & EXÁMENES ---
            else if (currentView == VIEW_AGENDA) {
                bool addExam = false, nightMode = false, back = false;
                if (ui.handleAgendaTouch(touchX, touchY, &addExam, &nightMode, &back)) {
                    if (back) {
                        audio.playClick();
                        currentView = VIEW_HOME;
                        refreshCurrentView();
                    } else if (addExam) {
                        audio.playClick();
                        currentView = VIEW_KEYBOARD_EXAM;
                        keyboard.reset("Nombre del examen/hito:", false, "");
                        keyboard.drawFull();
                    } else if (nightMode) {
                        audio.playClick();
                        currentView = VIEW_NIGHT_CLOCK;
                        analogWrite(PIN_BACKLIGHT, 25);
                        ui.drawNightClockView(battery, weather);
                    }
                }
            } 
            // --- ENTRADA DE TEXTO PARA EXAMEN ---
            else if (currentView == VIEW_KEYBOARD_EXAM) {
                KeyAction kAction = keyboard.handleTouch(touchX, touchY);
                if (kAction == KEY_ACTION_CHAR_TYPED || kAction == KEY_ACTION_DELETE) {
                    audio.playClick();
                } else if (kAction == KEY_ACTION_BACK) {
                    audio.playClick();
                    currentView = VIEW_AGENDA;
                    refreshCurrentView();
                } else if (kAction == KEY_ACTION_SUBMIT) {
                    audio.playWorkFinished();
                    calendar.addDeadline(keyboard.getText(), 28, 11, 2026, 10);
                    currentView = VIEW_AGENDA;
                    refreshCurrentView();
                }
            }
            // --- VISTA CLIMA OPEN-METEO ---
            else if (currentView == VIEW_WEATHER) {
                bool ref = false, back = false;
                if (ui.handleWeatherTouch(touchX, touchY, &ref, &back)) {
                    if (back) {
                        audio.playClick();
                        currentView = VIEW_HOME;
                        refreshCurrentView();
                    } else if (ref) {
                        audio.playClick();
                        weather.update(true);
                        ui.drawWeatherView(weather, battery, wifi.isConnected());
                    }
                }
            }
            // --- VISTA RESPIRACIÓN RELAX BOX ---
            else if (currentView == VIEW_BREATHING) {
                if (ui.handleBreathingTouch(touchX, touchY)) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    led.setMode(pomodoro.isRunning() ? (pomodoro.isFocus() ? LED_MODE_FOCUS : LED_MODE_BREAK) : LED_MODE_IDLE);
                    refreshCurrentView();
                }
            }
            // --- VISTA LÁMPARA RGB DE ESCRITORIO ---
            else if (currentView == VIEW_RGB_LAMP) {
                bool back = false;
                int modeSel = ui.handleRgbLampTouch(touchX, touchY, &back);
                if (back) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    refreshCurrentView();
                } else if (modeSel >= 0) {
                    audio.playClick();
                    lampMode = modeSel;
                    if (lampMode == 0) led.setRaw(false, false, false);
                    else if (lampMode == 1) led.setRaw(true, true, false);  // Blanco Cálido (R+G)
                    else if (lampMode == 2) led.setRaw(false, true, true);  // Cian Enfoque (G+B)
                    else if (lampMode == 3) led.setRaw(false, false, true); // Relax Azul (B)
                    else if (lampMode == 4) led.setRaw(true, true, false);  // Vela ámbar
                    ui.drawRgbLampView(lampMode, battery);
                }
            }
            // --- VISTA BATERÍA 5000 mAh ---
            else if (currentView == VIEW_BATTERY) {
                if (ui.handleBatteryTouch(touchX, touchY)) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    refreshCurrentView();
                }
            }
            // --- VISTA ESTADÍSTICAS & HISTORIAL MICROSD ---
            else if (currentView == VIEW_STATS) {
                if (ui.handleStatsTouch(touchX, touchY)) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    refreshCurrentView();
                }
            }
            // --- VISTA AJUSTES DEL SISTEMA ---
            else if (currentView == VIEW_SETTINGS) {
                bool toggleAuto = false, briDown = false, briUp = false;
                bool toggleMute = false, timeoutToggle = false, nightClock = false, openWifi = false, back = false;
                ui.handleSettingsTouch(touchX, touchY, &toggleAuto, &briDown, &briUp, 
                                       &toggleMute, &timeoutToggle, &nightClock, &openWifi, &back);

                if (back) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    refreshCurrentView();
                } else if (toggleAuto) {
                    autoBrightness = !autoBrightness;
                    audio.playClick();
                    ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness, audio.isMuted(), timeoutMinutes);
                } else if (briDown) {
                    autoBrightness = false;
                    currentBrightness = max(40, currentBrightness - 25);
                    analogWrite(PIN_BACKLIGHT, currentBrightness);
                    audio.playClick();
                    ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness, audio.isMuted(), timeoutMinutes);
                } else if (briUp) {
                    autoBrightness = false;
                    currentBrightness = min(255, currentBrightness + 25);
                    analogWrite(PIN_BACKLIGHT, currentBrightness);
                    audio.playClick();
                    ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness, audio.isMuted(), timeoutMinutes);
                } else if (toggleMute) {
                    audio.toggleMute();
                    if (!audio.isMuted()) audio.playClick();
                    ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness, audio.isMuted(), timeoutMinutes);
                } else if (nightClock) {
                    audio.playClick();
                    currentView = VIEW_NIGHT_CLOCK;
                    analogWrite(PIN_BACKLIGHT, 25);
                    ui.drawNightClockView(battery, weather);
                } else if (openWifi) {
                    audio.playClick();
                    currentView = VIEW_WIFI_SCAN;
                    ui.drawWiFiScanView(wifi, battery, true);
                    wifi.scanNetworks();
                    scanPage = 0;
                    ui.drawWiFiScanView(wifi, battery, false, scanPage);
                }
            }
            // --- VISTA INFO DEL SISTEMA ---
            else if (currentView == VIEW_INFO) {
                if (ui.handleInfoTouch(touchX, touchY)) {
                    audio.playClick();
                    currentView = VIEW_HOME;
                    refreshCurrentView();
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
                    ui.drawWiFiScanView(wifi, battery, true);
                    wifi.scanNetworks();
                    scanPage = 0;
                    ui.drawWiFiScanView(wifi, battery, false, scanPage);
                } else if (nextPage) {
                    audio.playClick();
                    int totalP = (wifi.getNetworkCount() + 3) / 4;
                    if (totalP > 0) {
                        scanPage = (scanPage + 1) % totalP;
                    }
                    ui.drawWiFiScanView(wifi, battery, false, scanPage);
                } else if (selectedIdx >= 0) {
                    audio.playClick();
                    selectedSSID = wifi.getNetwork(selectedIdx).ssid;
                    currentView = VIEW_WIFI_KEYBOARD;
                    keyboard.reset("Clave: " + selectedSSID, true, wifi.getDefaultPassword());
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
                    ui.drawWiFiScanView(wifi, battery, false, scanPage);
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
                    ui.drawWiFiScanView(wifi, battery, false, scanPage);
                } else {
                    currentView = VIEW_SETTINGS;
                    refreshCurrentView();
                }
            }
        }
    }

    // 7. Actualizaciones periódicas de visualización según la vista activa
    if (currentView == VIEW_HOME) {
        time_t now = time(nullptr);
        struct tm *t = localtime(&now);
        if (t->tm_min != lastHomeMin) {
            lastHomeMin = t->tm_min;
            ui.drawHomeScreen(pomodoro, taskMgr, battery, weather, wifi.isConnected(), audio.isMuted(), autoBrightness, homeTab);
        }
    } else if (currentView == VIEW_POMODORO) {
        ui.updatePomodoroTimer(pomodoro, battery, wifi.isConnected());
    } else if (currentView == VIEW_AGENDA) {
        if (millis() - lastClockUpdate >= 1000) {
            lastClockUpdate = millis();
            ui.updateAgendaClock(calendar, weather);
        }
    } else if (currentView == VIEW_NIGHT_CLOCK) {
        if (millis() - lastClockUpdate >= 1000) {
            lastClockUpdate = millis();
            ui.updateNightClock(battery, weather);
        }
    }

    // 8. Sensor LDR y ajuste suave de retroiluminación cada 400 ms si autoBrightness está activo
    if (autoBrightness && !screenDimmed && currentView != VIEW_NIGHT_CLOCK && (millis() - lastLdrCheck >= 400)) {
        lastLdrCheck = millis();
        currentLdr = analogRead(PIN_LDR);

        targetBrightness = map(currentLdr, 4000, 500, 70, 255);
        targetBrightness = constrain(targetBrightness, 70, 255);

        if (abs(targetBrightness - currentBrightness) > 8) {
            currentBrightness += (targetBrightness > currentBrightness) ? 5 : -5;
            analogWrite(PIN_BACKLIGHT, currentBrightness);
        }
    }
}
