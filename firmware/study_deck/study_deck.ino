#include "AudioNotifier.h"
#include "BatteryManager.h"
#include "CST820.h"
#include "CalendarManager.h"
#include "HardwareConfig.h"
#include "KeyboardUI.h"
#include "LedIndicator.h"
#include "PomodoroEngine.h"
#include "ScreenServer.h"
#include "StorageManager.h"
#include "UIManager.h"
#include "WeatherManager.h"
#include "WiFiConfig.h"
#include <Arduino.h>
#include <TFT_eSPI.h>
#include <time.h>

// Periféricos y controladores de hardware
TFT_eSPI tft = TFT_eSPI();
CST820 touch(I2C_SDA_PIN, I2C_SCL_PIN, TP_RST_PIN, TP_INT_PIN);
AudioNotifier audio;
LedIndicator led;
PomodoroEngine pomodoro;
UIManager ui(tft);
WiFiConfig wifi;
KeyboardUI keyboard(tft);
BatteryManager battery(PIN_BATTERY);
CalendarManager calendar;
StorageManager storage;
WeatherManager weather;
ScreenServer screenServer(tft);

// Estado de la interfaz: Pantalla inicial predefinida = CLIMA & RELOJ DESK
AppView currentView = VIEW_WEATHER;
int homeTab = 0; // 0 = Estudio & Desk (6 apps), 1 = Sistema & Utils (4 apps)
int scanPage = 0;
String selectedSSID = "";
String typedPassword = "";
unsigned long resultStartTime = 0;
bool connectionSuccess = false;

// Estado de Lámpara RGB y Respiración Relax
int lampMode =
    0; // 0 = Apagado, 1 = Blanco Cálido, 2 = Cian, 3 = Azul, 4 = Vela
int breathingPhase = 0;
int breathingSec = 4;
int breathingCycle = 1;
unsigned long lastBreathingTick = 0;
unsigned long lastCandleTick = 0;
bool candleFlickerState = false;

// Configuración de pantalla y ahorro de energía
unsigned long lastTouchTime = 0;
unsigned long lastClockUpdate = 0;
int currentBrightness = 220;
bool autoBrightness = true;
int timeoutMinutes = 2; // 1, 2, 5 o 0 (desactivado)
bool screenDimmed = false;

// Rastreador de alarma para evitar repetición en el mismo minuto
int lastAlarmMinute = -1;

void refreshCurrentView() {
  switch (currentView) {
  case VIEW_WEATHER:
    ui.drawWeatherView(weather, calendar, battery, wifi.isConnected());
    break;
  case VIEW_HOME:
    ui.drawHomeScreen(pomodoro, battery, weather, wifi.isConnected(),
                      audio.isMuted(), autoBrightness, homeTab);
    break;
  case VIEW_POMODORO:
    ui.drawPomodoroView(pomodoro, calendar, battery, wifi.isConnected(),
                        audio.isMuted(), autoBrightness);
    break;
  case VIEW_AGENDA:
    ui.drawAgendaView(calendar, wifi, battery, weather);
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
    ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness,
                        audio.isMuted(), timeoutMinutes);
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

void handleTouchPoint(int touchX, int touchY);

void navigateToView(int viewId) {
  if (viewId >= 0 && viewId <= 15) {
    currentView = (AppView)viewId;
    if (currentView == VIEW_NIGHT_CLOCK) {
      analogWrite(PIN_BACKLIGHT, 25);
    } else {
      analogWrite(PIN_BACKLIGHT, currentBrightness);
    }
    refreshCurrentView();
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n==============================================");
  Serial.println("  ESTACION DE ESTUDIO INTELIGENTE - STUDY DECK");
  Serial.println("  Hardware: Guition ESP32-2432S024C (2.4\")");
  Serial.println("  Edicion Academica UNED Avila 2026/2027");
  Serial.println("==============================================\n");

  // 1. Inicializar retroiluminación
  pinMode(PIN_BACKLIGHT, OUTPUT);
  analogWrite(PIN_BACKLIGHT, currentBrightness);

  // 2. Inicializar sensor LDR y Monitor de Batería 5000 mAh
  pinMode(PIN_LDR, INPUT);
  battery.begin();

  // 3. Inicializar audio y LED RGB
  audio.begin();
  led.begin();
  led.setMode(LED_MODE_IDLE);

  // 4. Inicializar MicroSD, Pomodoro, Calendario y Clima
  storage.begin();
  pomodoro.begin();
  calendar.begin();
  weather.begin();

  // 5. Inicializar Wi-Fi, Sincronización NTP (Hora España CET/CEST) y Servidor Remoto
  configTzTime("CET-1CEST,M3.5.0,M10.5.0/3", "pool.ntp.org", "time.google.com");
  wifi.begin();
  screenServer.begin(handleTouchPoint, navigateToView, refreshCurrentView);

  // 6. Inicializar pantalla táctil y mostrar la vista predefinida (Clima &
  // Reloj Desk)
  ui.begin();
  touch.begin();

  lastTouchTime = millis();
  currentView = VIEW_WEATHER;
  refreshCurrentView();
}

void loop() {
  // 0. Atender peticiones del servidor de capturas y control HTTP
  screenServer.handleClient();

  // 1. Motores lógicos principales
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
  calendar.update(wifi.isConnected());

  // 2. Automatización del Estudio Vinculada al Calendario y Reloj (Alarma de
  // 3s)
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  if (t->tm_year > 120 && t->tm_min != lastAlarmMinute) {
    // A. Inicio de sesión agendada de estudio
    CalendarEvent startEv;
    if (calendar.checkSessionStartTrigger(startEv)) {
      lastAlarmMinute = t->tm_min;
      Serial.printf("[StudyDeck] INICIO DE SESION DE ESTUDIO: %s\n",
                    startEv.title.c_str());
      audio.playStudyAlarm3s();
      currentView = VIEW_POMODORO;
      screenDimmed = false;
      analogWrite(PIN_BACKLIGHT, currentBrightness);
      refreshCurrentView();
    }
    // B. Fin de horario de estudio agendado
    else if (calendar.checkSessionEndTrigger()) {
      lastAlarmMinute = t->tm_min;
      Serial.println(
          "[StudyDeck] FIN DE HORARIO DE ESTUDIO. Regresando a Clima.");
      audio.playStudyAlarm3s();
      currentView = VIEW_WEATHER;
      screenDimmed = false;
      analogWrite(PIN_BACKLIGHT, currentBrightness);
      refreshCurrentView();
    }
  }

  // 3. Actualización de reloj digital por segundo en vistas con reloj
  if (millis() - lastClockUpdate >= 1000) {
    lastClockUpdate = millis();
    if (currentView == VIEW_WEATHER) {
      ui.updateWeatherViewClock(weather, calendar);
    } else if (currentView == VIEW_AGENDA) {
      ui.updateAgendaClock(calendar, weather);
    } else if (currentView == VIEW_NIGHT_CLOCK) {
      ui.updateNightClock(battery, weather);
    } else if (currentView == VIEW_POMODORO) {
      ui.updatePomodoroTimer(pomodoro, calendar, battery, wifi.isConnected());
    }
  }

  // 4. Animación de Respiración Box Breathing (4-4-4-4)
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
        if (breathingPhase == 0)
          led.setRaw(false, true, true); // Inhala: Cian suave
        else if (breathingPhase == 1)
          led.setRaw(false, true, true); // Mantén: Cian
        else if (breathingPhase == 2)
          led.setRaw(false, false, true); // Exhala: Azul profundo
        else
          led.setRaw(false, false, false); // Pausa: Apagado
      }
      ui.drawBreathingView(breathingPhase, breathingSec, breathingCycle,
                           battery);
    }
  }

  // 5. Efecto parpadeo suave vela en modo lámpara vela
  if (lampMode == 4 &&
      (currentView == VIEW_RGB_LAMP || currentView == VIEW_WEATHER)) {
    if (millis() - lastCandleTick >= 120) {
      lastCandleTick = millis();
      candleFlickerState = !candleFlickerState;
      if (candleFlickerState) {
        led.setRaw(true, true, false); // Ámbar cálido
      } else {
        led.setRaw(true, false, false);
      }
    }
  }

  // 6. Gestión de conexión Wi-Fi asíncrona
  if (currentView == VIEW_WIFI_CONNECTING) {
    WiFiConnectStatus status = wifi.checkConnectStatus();
    if (status == WIFI_STATUS_CONNECTED) {
      connectionSuccess = true;
      if (selectedSSID.length() > 0) {
        wifi.saveCredentials(selectedSSID, typedPassword);
      }
      configTime(0, 0, "pool.ntp.org", "time.google.com");
      weather.update(true);              // Descarga inmediata de clima de Ávila
      calendar.syncGoogleCalendar(true); // Sincronización Google Calendar
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

  // 7. Retorno a Clima (Standby) por inactividad prolongada
  if (timeoutMinutes > 0 && currentView != VIEW_WEATHER &&
      currentView != VIEW_POMODORO && currentView != VIEW_NIGHT_CLOCK) {
    unsigned long maxIdleMs = (unsigned long)timeoutMinutes * 60000;
    if (millis() - lastTouchTime > maxIdleMs) {
      currentView = VIEW_WEATHER;
      refreshCurrentView();
      lastTouchTime = millis();
    }
  }

  // 8. Lectura táctil capacitiva CST820
  uint16_t rawX = 0, rawY = 0;
  uint8_t gesture = GESTURE_NONE;
  if (touch.getRawTouch(&rawX, &rawY, &gesture)) {
    if (screenDimmed) {
      screenDimmed = false;
      analogWrite(PIN_BACKLIGHT, currentBrightness);
      lastTouchTime = millis();
      return;
    }

    if (millis() - lastTouchTime >= 180) {
      lastTouchTime = millis();

      // Mapeo apaisado (320x240)
      int touchX = rawY;
      int touchY = 240 - rawX;
      if (touchX < 0)
        touchX = 0;
      if (touchX >= 320)
        touchX = 319;
      if (touchY < 0)
        touchY = 0;
      if (touchY >= 240)
        touchY = 239;

      handleTouchPoint(touchX, touchY);
    }
  }
}

void handleTouchPoint(int touchX, int touchY) {
  lastTouchTime = millis();
  if (screenDimmed) {
    screenDimmed = false;
    analogWrite(PIN_BACKLIGHT, currentBrightness);
  }

  // ========================================================================
  // 0. NAVEGACIÓN DIRECTA: LEFT CYBER RAIL (x: 0..46, y: 0..240)
  // ========================================================================
  if (touchX >= 0 && touchX <= 46) {
    audio.playClick();
    if (touchY >= 0 && touchY < 48) {
      currentView = VIEW_WEATHER; // Slot 0: DESK / STANDBY
    } else if (touchY >= 48 && touchY < 96) {
      currentView = VIEW_POMODORO; // Slot 1: FOCUS / POMODORO
    } else if (touchY >= 96 && touchY < 144) {
      currentView = VIEW_AGENDA; // Slot 2: CAL / AGENDA UNED
    } else if (touchY >= 144 && touchY < 192) {
      currentView = VIEW_RGB_LAMP; // Slot 3: LAMP / AURA
    } else if (touchY >= 192 && touchY < 240) {
      currentView = VIEW_SETTINGS; // Slot 4: SYS / CONTROL CENTER
    }
    refreshCurrentView();
    return;
  }

  // ========================================================================
  // 1. ÁREA DE TRABAJO (x: 47..320, y: 0..240)
  // ========================================================================

  // --- VISTA 0: PANTALLA PREDEFINIDA CLIMA & RADAR DESK (STANDBY) ---
  if (currentView == VIEW_WEATHER) {
    audio.playClick();
    // Si toca la tarjeta inferior de radar académico, abre Pomodoro
    if (touchY >= 122) {
      currentView = VIEW_POMODORO;
    } else {
      currentView = VIEW_AGENDA;
    }
    refreshCurrentView();
    return;
  }

      // --- VISTA 1: MENÚ LANZADOR DE APLICACIONES ---
      else if (currentView == VIEW_HOME) {
        int appSelected = ui.handleHomeTouch(touchX, touchY, homeTab);
        if (appSelected == -20) {
          // Volver a Standby / Clima
          audio.playClick();
          currentView = VIEW_WEATHER;
          refreshCurrentView();
        } else if (appSelected == -10) {
          audio.playClick();
          refreshCurrentView();
        } else if (appSelected == 1) { // ⏱️ Pomodoro
          audio.playClick();
          currentView = VIEW_POMODORO;
          refreshCurrentView();
        } else if (appSelected == 4) { // 📅 Agenda UNED
          audio.playClick();
          currentView = VIEW_AGENDA;
          refreshCurrentView();
        } else if (appSelected == 8) { // 🌤️ Clima
          audio.playClick();
          currentView = VIEW_WEATHER;
          refreshCurrentView();
        } else if (appSelected == 9) { // 🌙 Reloj Noche
          audio.playClick();
          currentView = VIEW_NIGHT_CLOCK;
          analogWrite(PIN_BACKLIGHT, 25);
          ui.drawNightClockView(battery, weather);
        } else if (appSelected == 10) { // 🫁 Relax Box
          audio.playClick();
          currentView = VIEW_BREATHING;
          breathingPhase = 0;
          breathingSec = 4;
          breathingCycle = 1;
          led.setRaw(false, true, true);
          ui.drawBreathingView(breathingPhase, breathingSec, breathingCycle,
                               battery);
        } else if (appSelected == 11) { // 💡 Lámpara RGB
          audio.playClick();
          currentView = VIEW_RGB_LAMP;
          refreshCurrentView();
        } else if (appSelected == 12) { // 🔋 Batería 5Ah
          audio.playClick();
          currentView = VIEW_BATTERY;
          refreshCurrentView();
        } else if (appSelected == 7) { // 📊 Stats MicroSD
          audio.playClick();
          currentView = VIEW_STATS;
          refreshCurrentView();
        } else if (appSelected == 5) { // ⚙️ Ajustes
          audio.playClick();
          currentView = VIEW_SETTINGS;
          refreshCurrentView();
        } else if (appSelected == 6) { // ℹ️ Info Deck
          audio.playClick();
          currentView = VIEW_INFO;
          refreshCurrentView();
        }
      }

      // --- VISTA 2: APLICACIÓN POMODORO ---
      else if (currentView == VIEW_POMODORO) {
        UIButtonAction action = ui.handlePomodoroTouch(touchX, touchY);
        switch (action) {
        case UI_ACTION_BACK_HOME:
          audio.playClick();
          currentView = VIEW_HOME;
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

      // --- VISTA 3: APLICACIÓN AGENDA & GOOGLE CALENDAR ---
      else if (currentView == VIEW_AGENDA) {
        if (calendar.getViewMode() == CAL_VIEW_DAY && touchY >= 186) {
          audio.playClick();
          currentView = VIEW_POMODORO;
          refreshCurrentView();
          return;
        }
        bool addExam = false, syncGCal = false, back = false;
        if (ui.handleAgendaTouch(touchX, touchY, calendar, &addExam, &syncGCal,
                                 &back)) {
          audio.playClick();
          refreshCurrentView();
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

      // --- VISTA RELOJ NOCTURNO ---
      else if (currentView == VIEW_NIGHT_CLOCK) {
        audio.playClick();
        currentView = VIEW_WEATHER;
        analogWrite(PIN_BACKLIGHT, currentBrightness);
        refreshCurrentView();
      }

      // --- VISTA RESPIRACIÓN RELAX BOX ---
      else if (currentView == VIEW_BREATHING) {
        if (ui.handleBreathingTouch(touchX, touchY)) {
          audio.playClick();
          led.setMode(LED_MODE_IDLE);
          currentView = VIEW_HOME;
          refreshCurrentView();
        }
      }

      // --- VISTA LÁMPARA RGB ---
      else if (currentView == VIEW_RGB_LAMP) {
        bool back = false;
        int mode = ui.handleRgbLampTouch(touchX, touchY, &back);
        if (mode == 10) {
          audio.playClick();
          currentView = VIEW_BREATHING;
          breathingPhase = 0;
          breathingSec = 4;
          breathingCycle = 1;
          led.setRaw(false, true, true);
          refreshCurrentView();
        } else if (mode >= 0) {
          audio.playClick();
          lampMode = mode;
          if (lampMode == 0)
            led.setRaw(false, false, false);
          else if (lampMode == 1)
            led.setRaw(true, true, true);
          else if (lampMode == 2)
            led.setRaw(false, true, true);
          else if (lampMode == 3)
            led.setRaw(false, false, true);
          else if (lampMode == 4)
            led.setRaw(true, true, false);
          ui.drawRgbLampView(lampMode, battery);
        }
      }

      // --- VISTA BATERÍA ---
      else if (currentView == VIEW_BATTERY) {
        if (ui.handleBatteryTouch(touchX, touchY)) {
          audio.playClick();
          currentView = VIEW_HOME;
          refreshCurrentView();
        }
      }

      // --- VISTA ESTADÍSTICAS ---
      else if (currentView == VIEW_STATS) {
        if (ui.handleStatsTouch(touchX, touchY)) {
          audio.playClick();
          currentView = VIEW_HOME;
          refreshCurrentView();
        }
      }

      // --- VISTA INFORMACIÓN ---
      else if (currentView == VIEW_INFO) {
        if (ui.handleInfoTouch(touchX, touchY)) {
          audio.playClick();
          currentView = VIEW_HOME;
          refreshCurrentView();
        }
      }

      // --- VISTA AJUSTES ---
      else if (currentView == VIEW_SETTINGS) {
        if (touchY >= 186 && touchY <= 234) {
          audio.playClick();
          if (touchX >= 52 && touchX <= 178) {
            currentView = VIEW_BATTERY;
          } else if (touchX >= 188 && touchX <= 314) {
            currentView = VIEW_INFO;
          }
          refreshCurrentView();
          return;
        }

        bool tAuto = false, bDown = false, bUp = false, tMute = false,
             toToggle = false, nClock = false, oWifi = false, back = false;
        ui.handleSettingsTouch(touchX, touchY, &tAuto, &bDown, &bUp, &tMute,
                               &toToggle, &nClock, &oWifi, &back);

        if (back) {
          audio.playClick();
          currentView = VIEW_HOME;
          refreshCurrentView();
        } else if (tAuto) {
          audio.playClick();
          autoBrightness = !autoBrightness;
          ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness,
                              audio.isMuted(), timeoutMinutes);
        } else if (bDown) {
          audio.playClick();
          autoBrightness = false;
          currentBrightness = max(30, currentBrightness - 25);
          analogWrite(PIN_BACKLIGHT, currentBrightness);
          ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness,
                              audio.isMuted(), timeoutMinutes);
        } else if (bUp) {
          audio.playClick();
          autoBrightness = false;
          currentBrightness = min(255, currentBrightness + 25);
          analogWrite(PIN_BACKLIGHT, currentBrightness);
          ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness,
                              audio.isMuted(), timeoutMinutes);
        } else if (tMute) {
          audio.toggleMute();
          audio.playClick();
          ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness,
                              audio.isMuted(), timeoutMinutes);
        } else if (toToggle) {
          audio.playClick();
          if (timeoutMinutes == 1)
            timeoutMinutes = 2;
          else if (timeoutMinutes == 2)
            timeoutMinutes = 5;
          else if (timeoutMinutes == 5)
            timeoutMinutes = 0;
          else
            timeoutMinutes = 1;
          ui.drawSettingsView(wifi, battery, currentBrightness, autoBrightness,
                              audio.isMuted(), timeoutMinutes);
        } else if (oWifi) {
          audio.playClick();
          currentView = VIEW_WIFI_SCAN;
          scanPage = 0;
          ui.drawWiFiScanView(wifi, battery, true, scanPage);
          wifi.scanNetworks();
          ui.drawWiFiScanView(wifi, battery, false, scanPage);
        }
      }

      // --- VISTA ESCANEO WI-FI ---
      else if (currentView == VIEW_WIFI_SCAN) {
        bool rescan = false, back = false, nextPage = false;
        int netSelected =
            ui.handleWiFiScanTouch(touchX, touchY, wifi.getNetworkCount(),
                                   scanPage, &rescan, &back, &nextPage);

        if (back) {
          audio.playClick();
          currentView = VIEW_SETTINGS;
          refreshCurrentView();
        } else if (rescan) {
          audio.playClick();
          ui.drawWiFiScanView(wifi, battery, true, scanPage);
          wifi.scanNetworks();
          ui.drawWiFiScanView(wifi, battery, false, scanPage);
        } else if (nextPage) {
          audio.playClick();
          int totalNets = wifi.getNetworkCount();
          if ((scanPage + 1) * 4 < totalNets)
            scanPage++;
          else
            scanPage = 0;
          ui.drawWiFiScanView(wifi, battery, false, scanPage);
        } else if (netSelected >= 0) {
          audio.playClick();
          DiscoveredNetwork chosenNet = wifi.getNetwork(netSelected);
          selectedSSID = chosenNet.ssid;
          if (!chosenNet.isEncrypted) {
            currentView = VIEW_WIFI_CONNECTING;
            ui.drawWiFiConnecting(selectedSSID);
            wifi.startConnect(selectedSSID, "");
          } else {
            currentView = VIEW_WIFI_KEYBOARD;
            String prefilledPass = "";
            if (wifi.hasSavedCredentials() && selectedSSID == wifi.getSavedSSID()) {
              prefilledPass = wifi.getSavedPassword();
            } else if (selectedSSID == wifi.getDefaultSSID() || wifi.getDefaultSSID().length() == 0 || prefilledPass.length() == 0) {
              prefilledPass = wifi.getDefaultPassword();
            }
            keyboard.reset("Clave para: " + selectedSSID, false, prefilledPass);
            keyboard.drawFull();
          }
        }
      }

      // --- ENTRADA DE CLAVE WI-FI ---
      else if (currentView == VIEW_WIFI_KEYBOARD) {
        KeyAction kAction = keyboard.handleTouch(touchX, touchY);
        if (kAction == KEY_ACTION_CHAR_TYPED || kAction == KEY_ACTION_DELETE) {
          audio.playClick();
        } else if (kAction == KEY_ACTION_BACK) {
          audio.playClick();
          currentView = VIEW_WIFI_SCAN;
          refreshCurrentView();
        } else if (kAction == KEY_ACTION_SUBMIT) {
          audio.playWorkFinished();
          typedPassword = keyboard.getText();
          currentView = VIEW_WIFI_CONNECTING;
          ui.drawWiFiConnecting(selectedSSID);
          wifi.startConnect(selectedSSID, typedPassword);
        }
      }

      // --- RESULTADO WI-FI ---
      else if (currentView == VIEW_WIFI_RESULT) {
        audio.playClick();
        currentView = connectionSuccess ? VIEW_WEATHER : VIEW_WIFI_SCAN;
        refreshCurrentView();
      }
}
