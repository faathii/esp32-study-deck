#include "UIManager.h"
#include <time.h>

// ============================================================================
// SISTEMA DE DISEÑO: CYBER-STUDIO DECK OS v2 (AERO-CYBER MINIMALIST 16-BIT)
// ============================================================================
// Fondo negro absoluto (efecto OLED en panel IPS/TFT para contraste infinito)
#define COLOR_BG 0x0000
#define COLOR_STATUS_BAR 0x0000
#define COLOR_SEPARATOR 0x18C3 // Línea de separación sutil grafito (24, 24, 24)

// Superficies elevadas y tarjetas (Dark Carbon Glass)
#define COLOR_CARD 0x0842 // Carbón profundo para islas de contenido (8, 8, 8)
#define COLOR_CARD_HL                                                          \
  0x10A3 // Superficie interactiva / ranura seleccionada (16, 20, 24)
#define COLOR_CARD_ACTIVE 0x1925 // Pizarra activa con relieve (24, 36, 40)
#define COLOR_BORDER 0x2145      // Borde fino de 1px tipo titanio (33, 40, 41)
#define COLOR_BORDER_HL 0xFFFF   // Borde destacado blanco nítido

// Tipografía y jerarquía visual
#define COLOR_TEXT_PRIMARY                                                     \
  0xFFFF // Blanco puro (máximo contraste para dígitos y títulos)
#define COLOR_TEXT_SECONDARY                                                   \
  0xC67A // Plata titanio (198, 206, 210) para subtítulos y datos
#define COLOR_TEXT_MUTED                                                       \
  0x52AA // Grafito oscuro (82, 85, 82) para unidades y leyendas

// Acentos cromáticos semánticos
#define COLOR_ACCENT_CYAN                                                      \
  0x07FF // Cian eléctrico (Tiempo, Wi-Fi, selección activa)
#define COLOR_ACCENT_CORAL                                                     \
  0xFA60 // Coral tomate brillante (Enfoque Pomodoro, alertas)
#define COLOR_ACCENT_MINT                                                      \
  0x2FEA // Verde menta zen (Descanso, relax, estado óptimo)
#define COLOR_ACCENT_AMBER                                                     \
  0xFFC0 // Ámbar cálido / Oro (Lámpara de noche, Sol, avisos)
#define COLOR_ACCENT_ROYAL                                                     \
  0xA2BF // Amatista / Púrpura UNED (Calendario académico, hitos)
#define COLOR_ACCENT_EMERALD                                                   \
  0x07E0 // Verde esmeralda (Batería 100%, carga activa)

// ============================================================================
// CONSTRUCTOR Y CONTROLADOR DE HARDWARE
// ============================================================================
UIManager::UIManager(TFT_eSPI &tft) : _tft(tft) {}

void UIManager::begin() {
  _tft.begin();
  _tft.setRotation(SCREEN_ROTATION);
  _tft.fillScreen(COLOR_BG);
}

// ============================================================================
// COMPONENTE CORE: LEFT CYBER RAIL (DOCK DE NAVEGACIÓN DIRECTA x: 0..46)
// ============================================================================
void UIManager::drawLeftRail(int activeTab, const BatteryManager &battery,
                             bool wifiConnected) {
  // Fondo de barra lateral y separador vertical de 1px
  _tft.fillRect(0, 0, 46, 240, COLOR_BG);
  _tft.drawFastVLine(45, 0, 240, COLOR_BORDER);
  _tft.setTextDatum(TL_DATUM);

  // 4 Ranuras táctiles verticales ampliadas (60 px de alto cada una para
  // ergonomía total)
  const char *slotLabels[4] = {"DESK", "FOCUS", "CAL", "SYS"};

  for (int i = 0; i < 4; i++) {
    int y0 = i * 60;
    bool active = (activeTab == i);

    if (active) {
      // Indicador vertical iluminado (neon bar) en el borde izquierdo
      uint16_t barCol = (i == 1) ? COLOR_ACCENT_CORAL : COLOR_ACCENT_CYAN;
      _tft.fillRect(0, y0 + 6, 3, 48, barCol);
      // Fondo sutilmente elevado para la pestaña activa
      _tft.fillRoundRect(5, y0 + 5, 37, 50, 4, COLOR_CARD_HL);
      _tft.drawRoundRect(5, y0 + 5, 37, 50, 4, COLOR_BORDER);
    }

    int cx = 23;
    int cy = y0 + 22;

    // Glifos vectoriales específicos de cada aplicación del Dock
    switch (i) {
    case 0: // DESK / STANDBY (Icono Casa / Estación)
      _tft.fillTriangle(cx, cy - 8, cx - 9, cy, cx + 9, cy,
                        active ? COLOR_ACCENT_AMBER : COLOR_TEXT_SECONDARY);
      _tft.fillRect(cx - 7, cy, 14, 8,
                    active ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED);
      _tft.fillRect(cx - 2, cy + 3, 4, 5, COLOR_BG); // Puerta
      break;

    case 1: // FOCUS / POMODORO (Cronómetro de precisión)
      _tft.drawCircle(cx, cy, 8,
                      active ? COLOR_ACCENT_CORAL : COLOR_TEXT_SECONDARY);
      _tft.drawCircle(cx, cy, 7,
                      active ? COLOR_ACCENT_CORAL : COLOR_TEXT_SECONDARY);
      _tft.drawFastVLine(cx, cy - 10, 3,
                         active ? COLOR_TEXT_PRIMARY
                                : COLOR_TEXT_MUTED); // Botón corona
      _tft.drawLine(cx, cy, cx + 4, cy - 3,
                    active ? COLOR_ACCENT_CORAL : COLOR_TEXT_PRIMARY); // Aguja
      break;

    case 2: // CAL / AGENDA (Cuaderno anillado UNED)
      _tft.fillRoundRect(cx - 8, cy - 7, 16, 15, 2,
                         active ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY);
      _tft.fillRect(cx - 8, cy - 7, 16, 4,
                    COLOR_ACCENT_ROYAL);                       // Barra superior
      _tft.drawPixel(cx - 5, cy - 8, COLOR_BG);                // Anilla 1
      _tft.drawPixel(cx + 4, cy - 8, COLOR_BG);                // Anilla 2
      _tft.fillRect(cx - 2, cy + 1, 4, 4, COLOR_ACCENT_CORAL); // Hito HOY
      break;

    case 3: // SYS / CONTROL CENTER (Sliders de precisión)
      _tft.drawFastHLine(cx - 8, cy - 4, 16,
                         active ? COLOR_ACCENT_CYAN : COLOR_TEXT_MUTED);
      _tft.fillCircle(cx - 2, cy - 4, 2,
                      active ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY);
      _tft.drawFastHLine(cx - 8, cy + 3, 16,
                         active ? COLOR_ACCENT_CYAN : COLOR_TEXT_MUTED);
      _tft.fillCircle(cx + 3, cy + 3, 2,
                      active ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY);
      break;
    }

    // Texto de etiqueta debajo del icono
    _tft.setTextColor(active ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED,
                      active ? COLOR_CARD_HL : COLOR_BG);
    _tft.drawCentreString(slotLabels[i], cx, y0 + 44, 1);
  }
}

// ============================================================================
// BARRA SUPERIOR UNIFICADA (LEGACY FALLBACK)
// ============================================================================
void UIManager::drawUnifiedStatusBar(const BatteryManager &battery,
                                     bool wifiConnected, bool audioMuted,
                                     bool autoBrightness, int streakCount,
                                     const char *appTitle) {
  // Compatibilidad: Dibuja un mini-header en caso de invocación directa
  _tft.fillRect(0, 0, SCREEN_WIDTH, 22, COLOR_STATUS_BAR);
  _tft.setTextDatum(TL_DATUM);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_STATUS_BAR);
  if (appTitle != nullptr) {
    _tft.drawString(appTitle, 8, 4, 2);
  } else {
    _tft.drawString("STUDY DECK", 8, 4, 2);
  }
  _tft.drawFastHLine(0, 22, SCREEN_WIDTH, COLOR_SEPARATOR);
}

// Micro-iconos vectoriales auxiliares
void UIManager::drawAppColorIcon(int cx, int cy, int appType) {
  switch (appType) {
  case 1: // Tomate Pomodoro
    _tft.fillCircle(cx, cy + 2, 11, COLOR_ACCENT_CORAL);
    _tft.drawCircle(cx - 4, cy - 1, 4, 0xFDC8);
    _tft.fillRect(cx - 1, cy - 10, 3, 5, 0x1CE0);
    _tft.fillTriangle(cx, cy - 7, cx - 7, cy - 10, cx - 2, cy - 5, 0x1CE0);
    _tft.fillTriangle(cx, cy - 7, cx + 7, cy - 10, cx + 2, cy - 5, 0x1CE0);
    break;

  case 3: // Sol + Nube Clima
    _tft.fillCircle(cx - 3, cy - 3, 8, COLOR_ACCENT_AMBER);
    _tft.drawFastVLine(cx - 3, cy - 14, 3, COLOR_ACCENT_AMBER);
    _tft.drawFastHLine(cx - 14, cy - 3, 3, COLOR_ACCENT_AMBER);
    _tft.drawLine(cx - 10, cy - 10, cx - 8, cy - 8, COLOR_ACCENT_AMBER);
    _tft.fillCircle(cx + 4, cy + 4, 6, 0xCE7F);
    _tft.fillCircle(cx - 1, cy + 5, 5, 0xCE7F);
    _tft.fillRoundRect(cx - 5, cy + 5, 15, 6, 2, 0xCE7F);
    _tft.drawPixel(cx - 1, cy + 12, COLOR_ACCENT_CYAN);
    _tft.drawPixel(cx + 5, cy + 12, COLOR_ACCENT_CYAN);
    break;

  case 7: // Batería
    _tft.drawRoundRect(cx - 11, cy - 6, 20, 13, 2, COLOR_TEXT_PRIMARY);
    _tft.fillRect(cx + 9, cy - 2, 3, 5, 0xC618);
    _tft.fillRect(cx - 9, cy - 4, 14, 9, COLOR_ACCENT_EMERALD);
    _tft.drawLine(cx - 3, cy - 3, cx - 1, cy, COLOR_ACCENT_AMBER);
    _tft.drawLine(cx - 1, cy, cx - 4, cy + 3, COLOR_ACCENT_AMBER);
    break;

  default:
    _tft.fillCircle(cx, cy, 10, COLOR_CARD_HL);
    _tft.drawCircle(cx, cy, 10, COLOR_BORDER);
    break;
  }
}

void UIManager::drawAppCard(int x, int y, int w, int h, uint16_t bgColor,
                            int appType, const char *title,
                            const char *subtitle) {
  _tft.fillRoundRect(x, y, w, h, 6, bgColor);
  _tft.drawRoundRect(x, y, w, h, 6, COLOR_BORDER);
  drawAppColorIcon(x + w / 2, y + 25, appType);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, bgColor);
  _tft.drawCentreString(title, x + w / 2, y + 46, 2);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, bgColor);
  _tft.drawCentreString(subtitle, x + w / 2, y + 64, 1);
}

// ============================================================================
// 1. PANTALLA PREDEFINIDA: ESTACIÓN CLIMA & RADAR DE ESCRITORIO (STANDBY)
//    SOPORTE PARA 5 ESTILOS SELECCIONABLES:
//    0 = Minimalista, 1 = Cyberdeck, 2 = Analógico, 3 = Dashboard UNED, 4 = Zen Clima
// ============================================================================
void UIManager::drawWeatherView(const WeatherManager &weather,
                                CalendarManager &calendar,
                                const BatteryManager &battery,
                                bool wifiConnected) {
  switch (_deskStyle) {
  case 0:
    drawDeskMinimal(weather, calendar, battery, wifiConnected);
    break;
  case 1:
    drawDeskCyberdeck(weather, calendar, battery, wifiConnected);
    break;
  case 2:
    drawDeskAnalog(weather, calendar, battery, wifiConnected);
    break;
  case 3:
    drawDeskDashboard(weather, calendar, battery, wifiConnected);
    break;
  case 4:
  default:
    drawDeskFlipClock(weather, calendar, battery, wifiConnected);
    break;
  }
}

void UIManager::updateWeatherViewClock(const WeatherManager &weather,
                                       CalendarManager &calendar) {
  switch (_deskStyle) {
  case 0:
    updateDeskMinimalClock(weather, calendar);
    break;
  case 1:
    updateDeskCyberdeckClock(weather, calendar);
    break;
  case 2:
    updateDeskAnalogClock(weather, calendar);
    break;
  case 3:
    updateDeskDashboardClock(weather, calendar);
    break;
  case 4:
  default:
    updateDeskFlipClock(weather, calendar);
    break;
  }
}

// ----------------------------------------------------------------------------
// ESTILO 0: MINIMALISTA (Bauhaus / Alto Contraste)
// ----------------------------------------------------------------------------
void UIManager::drawDeskMinimal(const WeatherManager &weather,
                                CalendarManager &calendar,
                                const BatteryManager &battery,
                                bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(0, battery, wifiConnected);

  _tft.setTextDatum(TL_DATUM);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  _tft.drawString("STUDY // MINIMAL", 60, 8, 1);

  char stBuf[32];
  if (battery.hasBatterySense() && battery.getPercentage() >= 0) {
    snprintf(stBuf, sizeof(stBuf), "%s  %d%%", wifiConnected ? "WIFI" : "OFF",
             battery.getPercentage());
  } else {
    snprintf(stBuf, sizeof(stBuf), "%s  USB 5V",
             wifiConnected ? "WIFI" : "OFF");
  }
  _tft.setTextDatum(TR_DATUM);
  _tft.setTextColor(COLOR_ACCENT_EMERALD, COLOR_BG);
  _tft.drawString(stBuf, 310, 8, 1);
  _tft.setTextDatum(TL_DATUM);

  _tft.drawFastHLine(60, 22, 250, COLOR_BORDER);

  updateDeskMinimalClock(weather, calendar);

  // Tarjeta 1: Clima (x: 58..178, w: 120, y: 154..232, h: 78)
  _tft.fillRoundRect(58, 154, 120, 78, 6, COLOR_CARD);
  _tft.drawRoundRect(58, 154, 120, 78, 6, COLOR_BORDER);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawString("AVILA", 68, 162, 1);

  char tBuf[16];
  if (weather.hasData()) {
    snprintf(tBuf, sizeof(tBuf), "%.1f", weather.getTemperature());
  } else {
    snprintf(tBuf, sizeof(tBuf), "--.-");
  }
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  int tw = _tft.drawString(tBuf, 68, 178, 4);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(" C", 68 + tw + 2, 180, 2);

  char condBuf[24];
  snprintf(condBuf, sizeof(condBuf), "%s %d%%",
           weather.hasData() ? weather.getConditionText().c_str() : "Despejado",
           weather.getHumidity());
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(condBuf, 68, 210, 1);

  // Tarjeta 2: Tarea UNED (x: 186..312, w: 126, y: 154..232, h: 78)
  _tft.fillRoundRect(186, 154, 126, 78, 6, COLOR_CARD);
  _tft.drawRoundRect(186, 154, 126, 78, 6, COLOR_BORDER);

  CalendarEvent todaySession;
  bool hasSession = calendar.getTodayMainSession(todaySession);
  bool ongoing = calendar.isStudySessionOngoing();

  _tft.setTextColor(COLOR_ACCENT_CORAL, COLOR_CARD);
  _tft.drawString(ongoing ? "EN CURSO" : (hasSession ? "HOY UNED" : "EXAMEN"),
                  196, 162, 1);

  String displayTitle = "";
  String displayTime = "";
  if (hasSession || ongoing) {
    displayTitle = todaySession.title;
    displayTime = todaySession.timeRange;
  } else {
    displayTime = calendar.getNextDeadlineCountdown(displayTitle);
  }

  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(displayTitle.substring(0, 14), 196, 178, 2);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(displayTime, 196, 206, 2);
}

void UIManager::updateDeskMinimalClock(const WeatherManager & /*weather*/,
                                       CalendarManager & /*calendar*/) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  char timeBuf[16];
  char dateBuf[48];

  if (t->tm_year > 120) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t->tm_hour, t->tm_min);
    const char *dias[7] = {"Domingo", "Lunes", "Martes", "Miercoles",
                           "Jueves",  "Viernes", "Sabado"};
    const char *meses[12] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                             "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
    snprintf(dateBuf, sizeof(dateBuf), "%s, %d %s %d", dias[t->tm_wday],
             t->tm_mday, meses[t->tm_mon], t->tm_year + 1900);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "12:00");
    snprintf(dateBuf, sizeof(dateBuf), "Sincronizando hora...");
  }

  _tft.fillRect(60, 30, 246, 80, COLOR_BG);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
  _tft.drawCentreString(timeBuf, 183, 34, 8);

  _tft.fillRect(60, 118, 246, 22, COLOR_BG);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
  _tft.drawCentreString(dateBuf, 183, 120, 2);
}

// ----------------------------------------------------------------------------
// ESTILO 1: CYBERDECK (HUD Táctico Neón)
// ----------------------------------------------------------------------------
void UIManager::drawDeskCyberdeck(const WeatherManager &weather,
                                  CalendarManager &calendar,
                                  const BatteryManager &battery,
                                  bool wifiConnected) {
  _tft.fillScreen(0x0000);
  drawLeftRail(0, battery, wifiConnected);

  // Corchetes angulares en esquinas HUD
  _tft.drawFastHLine(54, 4, 20, COLOR_ACCENT_CYAN);
  _tft.drawFastVLine(54, 4, 14, COLOR_ACCENT_CYAN);
  _tft.drawFastHLine(294, 4, 20, COLOR_ACCENT_CYAN);
  _tft.drawFastVLine(314, 4, 14, COLOR_ACCENT_CYAN);
  _tft.drawFastHLine(54, 236, 20, COLOR_ACCENT_CYAN);
  _tft.drawFastVLine(54, 222, 14, COLOR_ACCENT_CYAN);
  _tft.drawFastHLine(294, 236, 20, COLOR_ACCENT_CYAN);
  _tft.drawFastVLine(314, 222, 14, COLOR_ACCENT_CYAN);

  _tft.setTextColor(COLOR_ACCENT_EMERALD, 0x0000);
  _tft.drawString("// HUD.CYBERDECK: ONLINE", 62, 8, 1);
  _tft.setTextColor(COLOR_ACCENT_CYAN, 0x0000);
  _tft.drawString(wifiConnected ? "NET:5G_AVILA" : "NET:OFFLINE", 230, 8, 1);

  // Módulo Central: CHRONO POD (x: 56..312, y: 22..116, h: 94)
  _tft.fillRoundRect(56, 22, 256, 94, 4, 0x0842);
  _tft.drawRoundRect(56, 22, 256, 94, 4, COLOR_ACCENT_CYAN);
  _tft.setTextColor(COLOR_ACCENT_CYAN, 0x0842);
  _tft.drawString("// QUANTUM_CLOCK.CET", 66, 28, 1);

  updateDeskCyberdeckClock(weather, calendar);

  // Módulo Atmosférico Split Izquierdo (x: 56..178, w: 122, y: 122..232, h: 110)
  _tft.fillRoundRect(56, 122, 122, 110, 4, 0x0842);
  _tft.drawRoundRect(56, 122, 122, 110, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_ACCENT_AMBER, 0x0842);
  _tft.drawString("// ATMOS.POD", 64, 128, 1);

  char tBuf[16];
  if (weather.hasData()) {
    snprintf(tBuf, sizeof(tBuf), "%.1f", weather.getTemperature());
  } else {
    snprintf(tBuf, sizeof(tBuf), "--.-");
  }
  _tft.setTextColor(COLOR_TEXT_PRIMARY, 0x0842);
  int tw = _tft.drawString(tBuf, 64, 144, 4);
  _tft.setTextColor(COLOR_ACCENT_AMBER, 0x0842);
  _tft.drawString(" C", 64 + tw + 2, 148, 2);

  int hum = weather.getHumidity();
  _tft.setTextColor(COLOR_TEXT_SECONDARY, 0x0842);
  _tft.drawString("HR: " + String(hum) + "%", 64, 178, 1);
  _tft.drawRect(64, 192, 104, 8, COLOR_BORDER);
  int fillW = constrain((hum * 102) / 100, 0, 102);
  _tft.fillRect(65, 193, fillW, 6, COLOR_ACCENT_CYAN);

  _tft.setTextColor(COLOR_ACCENT_EMERALD, 0x0842);
  _tft.drawString(weather.hasData() ? weather.getConditionText().c_str()
                                    : "STANDBY",
                  64, 212, 1);

  // Módulo Táctico UNED Split Derecho (x: 186..312, w: 126, y: 122..232, h: 110)
  _tft.fillRoundRect(186, 122, 126, 110, 4, 0x0842);
  _tft.drawRoundRect(186, 122, 126, 110, 4, COLOR_ACCENT_CORAL);
  _tft.setTextColor(COLOR_ACCENT_CORAL, 0x0842);
  _tft.drawString("// MISSION.UNED", 194, 128, 1);

  CalendarEvent todaySession;
  bool hasSession = calendar.getTodayMainSession(todaySession);
  bool ongoing = calendar.isStudySessionOngoing();

  String targetTitle = "TUTORIA";
  String targetTime = "18:15";
  if (hasSession || ongoing) {
    targetTitle = todaySession.title;
    targetTime = todaySession.timeRange;
  } else {
    targetTime = calendar.getNextDeadlineCountdown(targetTitle);
  }

  _tft.setTextColor(COLOR_TEXT_PRIMARY, 0x0842);
  _tft.drawString(targetTitle.substring(0, 14), 194, 146, 2);
  _tft.setTextColor(COLOR_ACCENT_AMBER, 0x0842);
  _tft.drawString(targetTime, 194, 174, 2);
  _tft.setTextColor(COLOR_TEXT_MUTED, 0x0842);
  _tft.drawString(ongoing ? ">> ACTIVO <<" : "T-MINUS TARGET", 194, 212, 1);
}

void UIManager::updateDeskCyberdeckClock(const WeatherManager & /*weather*/,
                                         CalendarManager & /*calendar*/) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  char timeBuf[16];
  char dateBuf[48];

  if (t->tm_year > 120) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t->tm_hour, t->tm_min);
    snprintf(dateBuf, sizeof(dateBuf), "DATE: %04d.%02d.%02d // S:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_sec);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "12:00");
    snprintf(dateBuf, sizeof(dateBuf), "DATE: SYNCING...");
  }

  _tft.fillRect(66, 40, 140, 50, 0x0842);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, 0x0842);
  _tft.drawString(timeBuf, 68, 42, 7);

  int sec = (t->tm_year > 120) ? t->tm_sec : 0;
  int activeBlocks = (sec * 18) / 60;
  for (int b = 0; b < 18; b++) {
    int bx = 212 + (b % 6) * 15;
    int by = 46 + (b / 6) * 14;
    uint16_t bCol = (b <= activeBlocks) ? COLOR_ACCENT_CYAN : 0x18E3;
    _tft.fillRect(bx, by, 12, 10, bCol);
  }

  _tft.fillRect(66, 96, 230, 14, 0x0842);
  _tft.setTextColor(COLOR_ACCENT_CYAN, 0x0842);
  _tft.drawString(dateBuf, 66, 96, 1);
}

// ----------------------------------------------------------------------------
// ESTILO 2: RELOJ ANALÓGICO (Heritage Chronometer Dial)
// ----------------------------------------------------------------------------
void UIManager::drawDeskAnalog(const WeatherManager &weather,
                               CalendarManager &calendar,
                               const BatteryManager &battery,
                               bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(0, battery, wifiConnected);

  // Cabecera Heritage
  _tft.setTextDatum(TL_DATUM);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  _tft.drawString("CHRONOMETER // HERITAGE", 56, 7, 1);
  _tft.setTextDatum(TR_DATUM);
  _tft.setTextColor(COLOR_ACCENT_EMERALD, COLOR_BG);
  _tft.drawString(wifiConnected ? "WIFI  USB 5V" : "OFF  USB 5V", 310, 7, 1);
  _tft.setTextDatum(TL_DATUM);
  _tft.drawFastHLine(54, 18, 258, COLOR_BORDER);

  // Esfera del reloj: Centro (122, 126), Radio 56
  int cx = 122, cy = 126, r = 56;
  _tft.fillCircle(cx, cy, r, COLOR_CARD);
  _tft.drawCircle(cx, cy, r, 0x4208); // Bisel metálico
  _tft.drawCircle(cx, cy, r - 1, COLOR_BORDER);
  _tft.drawCircle(cx, cy, r - 2, COLOR_ACCENT_CYAN);

  // 12 Números cardinales (12, 3, 6, 9)
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("12", cx, cy - r + 5, 2);
  _tft.drawCentreString("6", cx, cy + r - 17, 2);
  _tft.drawString("3", cx + r - 14, cy - 7, 2);
  _tft.drawString("9", cx - r + 5, cy - 7, 2);

  // Marcadores de horas intermedias (1, 2, 4, 5, 7, 8, 10, 11) como puntos dorados
  for (int h = 1; h < 12; h++) {
    if (h % 3 != 0) {
      float angle = h * 30.0f * 0.01745329f;
      int px = cx + (int)((r - 9) * sin(angle));
      int py = cy - (int)((r - 9) * cos(angle));
      _tft.fillCircle(px, py, 2, COLOR_ACCENT_AMBER);
    }
  }

  // Agujas y actualización dinámica
  updateDeskAnalogClock(weather, calendar);

  // Sub-dial 1: Clima Ávila (y: 28..124, h: 96)
  _tft.fillRoundRect(188, 22, 124, 98, 6, COLOR_CARD);
  _tft.drawRoundRect(188, 22, 124, 98, 6, COLOR_BORDER);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawString("AVILA MET", 196, 28, 1);

  char tBuf[16];
  if (weather.hasData()) {
    snprintf(tBuf, sizeof(tBuf), "%.1f", weather.getTemperature());
  } else {
    snprintf(tBuf, sizeof(tBuf), "--.-");
  }
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  int tw = _tft.drawString(tBuf, 196, 42, 4);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(" C", 196 + tw + 2, 46, 2);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(weather.hasData() ? weather.getConditionText() : "Despejado",
                  196, 72, 1);
  _tft.drawString(String(weather.getHumidity()) + "% Humedad", 196, 92, 1);

  // Sub-dial 2: Agenda UNED (y: 126..232, h: 106)
  _tft.fillRoundRect(188, 126, 124, 106, 6, COLOR_CARD);
  _tft.drawRoundRect(188, 126, 124, 106, 6, COLOR_ACCENT_CORAL);
  _tft.setTextColor(COLOR_ACCENT_CORAL, COLOR_CARD);
  _tft.drawString("OBJETIVO UNED", 196, 134, 1);

  CalendarEvent todaySession;
  bool hasSession = calendar.getTodayMainSession(todaySession);
  bool ongoing = calendar.isStudySessionOngoing();

  String aTitle = "TUTORIA";
  String aTime = "18:15";
  if (hasSession || ongoing) {
    aTitle = todaySession.title;
    aTime = todaySession.timeRange;
  } else {
    aTime = calendar.getNextDeadlineCountdown(aTitle);
  }

  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(aTitle.substring(0, 13), 196, 150, 2);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(aTime, 196, 178, 2);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawString(ongoing ? "EN CURSO" : "PROGRAMADO", 196, 206, 1);
}

void UIManager::updateDeskAnalogClock(const WeatherManager & /*weather*/,
                                      CalendarManager & /*calendar*/) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  int cx = 122, cy = 126, r = 56;

  // Limpiar solo la zona interna donde giran las agujas sin borrar los números 12, 3, 6, 9
  _tft.fillCircle(cx, cy, r - 18, COLOR_CARD);

  // Ventana de fecha (Date Complication)
  char dayBuf[8];
  snprintf(dayBuf, sizeof(dayBuf), "%d", t->tm_mday);
  _tft.fillRoundRect(cx - 10, cy + 14, 20, 12, 2, COLOR_CARD_HL);
  _tft.drawRoundRect(cx - 10, cy + 14, 20, 12, 2, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString(dayBuf, cx, cy + 16, 1);

  float h_angle =
      ((t->tm_hour % 12) + t->tm_min / 60.0f) * 30.0f * 0.01745329f;
  float m_angle = (t->tm_min + t->tm_sec / 60.0f) * 6.0f * 0.01745329f;
  float s_angle = t->tm_sec * 6.0f * 0.01745329f;

  // Aguja Horaria (Baton de 22px con grosor)
  int hx = cx + (int)(22 * sin(h_angle));
  int hy = cy - (int)(22 * cos(h_angle));
  _tft.drawLine(cx, cy, hx, hy, COLOR_TEXT_PRIMARY);
  _tft.drawLine(cx + 1, cy, hx + 1, hy, COLOR_TEXT_PRIMARY);
  _tft.drawLine(cx - 1, cy, hx - 1, hy, COLOR_TEXT_PRIMARY);
  _tft.drawLine(cx, cy + 1, hx, hy + 1, COLOR_TEXT_PRIMARY);

  // Aguja Minutera (Sword Hand de 32px)
  int mx = cx + (int)(32 * sin(m_angle));
  int my = cy - (int)(32 * cos(m_angle));
  _tft.drawLine(cx, cy, mx, my, COLOR_ACCENT_CYAN);
  _tft.drawLine(cx + 1, cy, mx + 1, my, COLOR_ACCENT_CYAN);
  _tft.drawLine(cx, cy + 1, mx, my + 1, COLOR_ACCENT_CYAN);

  // Aguja Segundera con contrapeso circular trasero
  int sx = cx + (int)(35 * sin(s_angle));
  int sy = cy - (int)(35 * cos(s_angle));
  int tailX = cx - (int)(10 * sin(s_angle));
  int tailY = cy + (int)(10 * cos(s_angle));
  _tft.drawLine(tailX, tailY, sx, sy, COLOR_ACCENT_CORAL);
  _tft.fillCircle(tailX, tailY, 2, COLOR_ACCENT_CORAL);

  // Eje central dorado
  _tft.fillCircle(cx, cy, 3, COLOR_ACCENT_AMBER);
  _tft.drawCircle(cx, cy, 3, 0x0000);
}

// ----------------------------------------------------------------------------
// ESTILO 3: DASHBOARD UNED (Focus & Academic Station)
// ----------------------------------------------------------------------------
void UIManager::drawDeskDashboard(const WeatherManager &weather,
                                  CalendarManager &calendar,
                                  const BatteryManager &battery,
                                  bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(0, battery, wifiConnected);

  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
  _tft.drawString("UNED AVILA // CENTRO ASOCIADO", 56, 8, 1);
  _tft.setTextColor(COLOR_ACCENT_EMERALD, COLOR_BG);
  _tft.drawString(wifiConnected ? "WIFI  USB 5V" : "OFF  USB 5V", 232, 8, 1);
  _tft.drawFastHLine(52, 22, 262, COLOR_BORDER);

  // Tarjeta 1: Reloj & Clima Aislados (y: 26..94, h: 68, x: 52..314)
  _tft.fillRoundRect(52, 26, 262, 68, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 26, 262, 68, 6, COLOR_BORDER);

  // Cápsula Meteorológica Separada (x: 196..308, y: 32..88, w: 112, h: 56)
  _tft.fillRoundRect(196, 32, 112, 56, 4, COLOR_CARD_HL);
  _tft.drawRoundRect(196, 32, 112, 56, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD_HL);
  _tft.drawString("AVILA", 204, 36, 1);

  char tBuf[16];
  if (weather.hasData()) {
    snprintf(tBuf, sizeof(tBuf), "%.1f", weather.getTemperature());
  } else {
    snprintf(tBuf, sizeof(tBuf), "--.-");
  }
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  int tw = _tft.drawString(tBuf, 204, 48, 4);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD_HL);
  _tft.drawString(" C", 204 + tw + 2, 50, 2);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD_HL);
  _tft.drawString(weather.hasData() ? weather.getConditionText() : "Despejado",
                  204, 72, 1);

  updateDeskDashboardClock(weather, calendar);

  // Tarjeta 2 Hero: Centro de Mando Académico UNED (y: 100..234, h: 134, x: 52..314)
  _tft.fillRoundRect(52, 100, 262, 134, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 100, 262, 134, 6, COLOR_ACCENT_AMBER);

  CalendarEvent todaySession;
  bool hasSession = calendar.getTodayMainSession(todaySession);
  bool ongoing = calendar.isStudySessionOngoing();

  _tft.fillCircle(64, 114, 4,
                  ongoing ? COLOR_ACCENT_CORAL : COLOR_ACCENT_CYAN);
  _tft.setTextColor(ongoing ? COLOR_ACCENT_CORAL : COLOR_ACCENT_AMBER,
                    COLOR_CARD);
  _tft.drawString(ongoing ? "SESION ACTIVA EN CURSO"
                          : (hasSession ? "PROXIMA SESION HOY"
                                        : "OBJETIVO EXAMEN"),
                  74, 108, 2);

  String examTitle;
  String countdown = calendar.getNextDeadlineCountdown(examTitle);

  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(hasSession ? todaySession.title.substring(0, 24)
                             : examTitle.substring(0, 24),
                  62, 134, 2);

  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(hasSession ? todaySession.timeRange : "Cuenta Atras:", 62,
                  160, 2);

  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawString(hasSession ? "Inicio en horario" : countdown, 180, 160, 2);

  _tft.fillRoundRect(62, 192, 242, 34, 4, COLOR_ACCENT_EMERALD);
  _tft.setTextColor(0x0000, COLOR_ACCENT_EMERALD);
  _tft.drawCentreString("▶ INICIAR ENFOQUE POMODORO", 183, 200, 2);
}

void UIManager::updateDeskDashboardClock(const WeatherManager & /*weather*/,
                                         CalendarManager & /*calendar*/) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  char timeBuf[16];
  char dateBuf[48];

  if (t->tm_year > 120) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t->tm_hour, t->tm_min);
    const char *dias[7] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
    const char *meses[12] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                             "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
    snprintf(dateBuf, sizeof(dateBuf), "%s, %d %s %d", dias[t->tm_wday],
             t->tm_mday, meses[t->tm_mon], t->tm_year + 1900);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "12:00");
    snprintf(dateBuf, sizeof(dateBuf), "Sincronizando...");
  }

  _tft.fillRect(58, 30, 134, 40, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(timeBuf, 60, 32, 6);

  _tft.fillRect(58, 70, 134, 18, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(dateBuf, 60, 72, 1);
}

// ----------------------------------------------------------------------------
// ESTILO 4: RETRO FLIP CLOCK (Mecánico de Aletas / Solari Split-Flap)
// ----------------------------------------------------------------------------
void UIManager::drawDeskFlipClock(const WeatherManager &weather,
                                  CalendarManager &calendar,
                                  const BatteryManager &battery,
                                  bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(0, battery, wifiConnected);

  // Cabecera Flip Clock
  _tft.setTextDatum(TL_DATUM);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  _tft.drawString("SOLARI // MECHANICAL FLIP", 56, 8, 1);
  _tft.setTextDatum(TR_DATUM);
  _tft.setTextColor(COLOR_ACCENT_EMERALD, COLOR_BG);
  _tft.drawString(wifiConnected ? "WIFI  USB 5V" : "OFF  USB 5V", 310, 8, 1);
  _tft.setTextDatum(TL_DATUM);
  _tft.drawFastHLine(54, 20, 258, COLOR_BORDER);

  // Dos Aletas de Reloj Mecánico Flap (Horas y Minutos)
  updateDeskFlipClock(weather, calendar);

  // Tarjeta Inferior Izquierda: Clima Ávila (x: 56..178, w: 122, y: 136..232, h: 96)
  _tft.fillRoundRect(56, 136, 122, 96, 5, COLOR_CARD);
  _tft.drawRoundRect(56, 136, 122, 96, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawString("AVILA // CLIMA", 64, 142, 1);

  char tBuf[16];
  if (weather.hasData()) {
    snprintf(tBuf, sizeof(tBuf), "%.1f", weather.getTemperature());
  } else {
    snprintf(tBuf, sizeof(tBuf), "--.-");
  }
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  int tw = _tft.drawString(tBuf, 64, 156, 4);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(" C", 64 + tw + 2, 160, 2);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(weather.hasData() ? weather.getConditionText() : "Despejado",
                  64, 186, 1);
  char humBuf[32];
  snprintf(humBuf, sizeof(humBuf), "HR: %d%%  -  Optimo", weather.getHumidity());
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString(humBuf, 64, 204, 1);

  // Tarjeta Inferior Derecha: Tarea UNED (x: 188..310, w: 122, y: 136..232, h: 96)
  _tft.fillRoundRect(188, 136, 122, 96, 5, COLOR_CARD);
  _tft.drawRoundRect(188, 136, 122, 96, 5, COLOR_BORDER);

  CalendarEvent todaySession;
  bool hasSession = calendar.getTodayMainSession(todaySession);
  bool ongoing = calendar.isStudySessionOngoing();

  _tft.setTextColor(COLOR_ACCENT_CORAL, COLOR_CARD);
  _tft.drawString(ongoing ? "EN CURSO" : (hasSession ? "HOY UNED" : "PROXIMA UNED"),
                  196, 142, 1);

  String fTitle = "TUTORIA";
  String fTime = "18:15";
  if (hasSession || ongoing) {
    fTitle = todaySession.title;
    fTime = todaySession.timeRange;
  } else {
    fTime = calendar.getNextDeadlineCountdown(fTitle);
  }

  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(fTitle.substring(0, 13), 196, 158, 2);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(fTime, 196, 184, 2);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("Planificado Cal.", 196, 210, 1);
}

void UIManager::updateDeskFlipClock(const WeatherManager & /*weather*/,
                                    CalendarManager & /*calendar*/) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  char hBuf[8];
  char mBuf[8];

  if (t->tm_year > 120) {
    snprintf(hBuf, sizeof(hBuf), "%02d", t->tm_hour);
    snprintf(mBuf, sizeof(mBuf), "%02d", t->tm_min);
  } else {
    snprintf(hBuf, sizeof(hBuf), "12");
    snprintf(mBuf, sizeof(mBuf), "00");
  }

  // --- Aleta Izquierda (Horas) ---
  int hx = 56, hy = 26, hw = 122, hh = 102;
  _tft.fillRoundRect(hx, hy, hw, hh, 6, 0x1082); // Gris grafito mecánico profundo
  _tft.drawRoundRect(hx, hy, hw, hh, 6, 0x3186); // Borde exterior

  _tft.setTextDatum(TL_DATUM);
  _tft.setTextColor(COLOR_TEXT_MUTED, 0x1082);
  _tft.drawString("HOURS", hx + 8, hy + 6, 1);

  _tft.setTextColor(COLOR_TEXT_PRIMARY, 0x1082);
  _tft.drawCentreString(hBuf, hx + hw / 2, hy + 26, 6);

  // Ranura mecánica horizontal (Split Slit)
  int midY = hy + hh / 2;
  _tft.drawFastHLine(hx, midY - 1, hw, 0x0000);
  _tft.drawFastHLine(hx, midY, hw, 0x0000);
  _tft.drawFastHLine(hx, midY + 1, hw, 0x2104);

  // Pivotes / bisagras metálicas laterales
  _tft.fillRect(hx, midY - 4, 3, 8, COLOR_ACCENT_AMBER);
  _tft.fillRect(hx + hw - 3, midY - 4, 3, 8, COLOR_ACCENT_AMBER);

  // --- Aleta Derecha (Minutos) ---
  int mx = 188, my = 26, mw = 122, mh = 102;
  _tft.fillRoundRect(mx, my, mw, mh, 6, 0x1082);
  _tft.drawRoundRect(mx, my, mw, mh, 6, 0x3186);

  _tft.setTextDatum(TL_DATUM);
  _tft.setTextColor(COLOR_TEXT_MUTED, 0x1082);
  _tft.drawString("MINUTES", mx + 8, my + 6, 1);

  _tft.setTextColor(COLOR_TEXT_PRIMARY, 0x1082);
  _tft.drawCentreString(mBuf, mx + mw / 2, my + 26, 6);

  // Ranura mecánica horizontal
  _tft.drawFastHLine(mx, midY - 1, mw, 0x0000);
  _tft.drawFastHLine(mx, midY, mw, 0x0000);
  _tft.drawFastHLine(mx, midY + 1, mw, 0x2104);

  // Pivotes / bisagras metálicas laterales
  _tft.fillRect(mx, midY - 4, 3, 8, COLOR_ACCENT_AMBER);
  _tft.fillRect(mx + mw - 3, midY - 4, 3, 8, COLOR_ACCENT_AMBER);
}

bool UIManager::handleWeatherTouch(int /*x*/, int /*y*/, bool *outOpenMenu) {
  *outOpenMenu = true;
  return true;
}

// ============================================================================
// 2. MENÚ LANZADOR (COMPATIBILIDAD CON VISTAS LEGACY)
// ============================================================================
void UIManager::drawHomeScreen(const PomodoroEngine &engine,
                               const BatteryManager &battery,
                               const WeatherManager &weather,
                               bool wifiConnected, bool audioMuted,
                               bool autoBrightness, int homeTab) {
  // Redirige elegantemente a Standby con el Left Rail
  drawWeatherView(weather, *(CalendarManager *)nullptr, battery, wifiConnected);
}

int UIManager::handleHomeTouch(int x, int y, int &homeTab) { return -20; }

// ============================================================================
// 3. APLICACIÓN POMODORO CHRONO DECK (RADICALMENTE REDISEÑADA)
// ============================================================================
void UIManager::drawPomodoroView(const PomodoroEngine &engine,
                                 const CalendarManager &calendar,
                                 const BatteryManager &battery,
                                 bool wifiConnected, bool audioMuted,
                                 bool autoBrightness) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(1, battery, wifiConnected); // Slot 1 = FOCUS

  // 1. Franja Superior Académica (x: 52..314, y: 4..24)
  _tft.setTextDatum(TL_DATUM);
  _tft.fillRoundRect(52, 4, 46, 16, 2, COLOR_ACCENT_ROYAL);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_ACCENT_ROYAL);
  _tft.drawCentreString("UNED", 75, 7, 1);

  CalendarEvent session;
  bool hasSession = calendar.getTodayMainSession(session);
  String subName = hasSession ? session.title : "Modo Enfoque Libre";
  if (subName.length() > 24)
    subName = subName.substring(0, 22) + "..";
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
  _tft.drawString(subName, 104, 6, 2);

  _tft.drawFastHLine(52, 24, 262, COLOR_BORDER);

  // 2. Tarjeta Central del Cronómetro (x: 52..214, y: 28..178, w: 162)
  _tft.fillRoundRect(52, 28, 162, 150, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 28, 162, 150, 6, COLOR_BORDER);

  drawCountdown(engine);
  drawProgressBar(engine);

  // 3. Columna Vertical de Presets de Tiempo (x: 220..314, y: 28..178, w: 94)
  drawModeButtons(engine);

  // 4. Botonera Inferior Hero de Control Táctil (x: 52..314, y: 184..234)
  drawActionButtons(engine);
}

void UIManager::drawModeButtons(const PomodoroEngine &engine) {
  uint16_t currentFocus = engine.getFocusMinutes();
  int x0 = 220;
  int w = 94;

  const int presets[3] = {25, 50, 90};
  const char *pLabels[3] = {"25 MIN", "50 MIN", "90 MIN"};

  for (int i = 0; i < 3; i++) {
    int y = 28 + i * 51;
    bool active = (currentFocus == presets[i]);

    _tft.fillRoundRect(x0, y, w, 46, 4,
                       active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawRoundRect(x0, y, w, 46, 4,
                       active ? COLOR_TEXT_PRIMARY : COLOR_BORDER);

    _tft.setTextColor(active ? COLOR_BG : COLOR_TEXT_SECONDARY,
                      active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawCentreString(pLabels[i], x0 + w / 2, y + 10, 2);
    _tft.setTextColor(active ? 0x31A6 : COLOR_TEXT_MUTED,
                      active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawCentreString(i == 0 ? "Normal" : (i == 1 ? "Intenso" : "Examen"),
                          x0 + w / 2, y + 28, 1);
  }
}

void UIManager::drawCountdown(const PomodoroEngine &engine) {
  uint32_t s = engine.getRemainingSeconds();
  uint16_t m = s / 60;
  uint8_t sec = s % 60;

  char timeBuf[12];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", m, sec);

  // Insignia superior de fase
  const char *stateStr = "EN REPOSO";
  uint16_t sCol = COLOR_TEXT_MUTED;
  if (engine.isFocus()) {
    stateStr = engine.isPaused() ? "PAUSADO" : "ENFOQUE";
    sCol = engine.isPaused() ? COLOR_ACCENT_AMBER : COLOR_ACCENT_CORAL;
  } else if (engine.isBreak()) {
    stateStr = engine.isPaused() ? "PAUSA" : "DESCANSO";
    sCol = engine.isPaused() ? COLOR_ACCENT_AMBER : COLOR_ACCENT_MINT;
  }

  _tft.fillRect(58, 36, 150, 20, COLOR_CARD);
  _tft.fillRoundRect(62, 38, 142, 18, 3, COLOR_BG);
  _tft.drawRoundRect(62, 38, 142, 18, 3, sCol);
  _tft.setTextColor(sCol, COLOR_BG);
  _tft.drawCentreString(stateStr, 133, 41, 1);

  // Dígitos gigantes de cronómetro 7-segment
  _tft.fillRect(58, 64, 150, 56, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(timeBuf, 133, 68, 6); // Font 6: Extra grande y nítido

  // Porcentaje y estado de sesión
  _tft.fillRect(58, 126, 150, 18, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  int pct = (int)(engine.getProgress() * 100);
  char pBuf[24];
  snprintf(pBuf, sizeof(pBuf), "%d%% COMPLETADO", pct);
  _tft.drawCentreString(pBuf, 133, 128, 1);
}

void UIManager::drawProgressBar(const PomodoroEngine &engine) {
  int barX = 62;
  int barY = 152;
  int barW = 142;
  int barH = 8;

  _tft.fillRoundRect(barX, barY, barW, barH, 2, COLOR_BG);
  _tft.drawRoundRect(barX, barY, barW, barH, 2, COLOR_BORDER);

  float prog = engine.getProgress();
  int fillW = (int)(prog * (barW - 2));
  if (fillW > 0) {
    uint16_t pCol = engine.isBreak() ? COLOR_ACCENT_MINT : COLOR_ACCENT_CORAL;
    _tft.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 2, pCol);
  }
}

void UIManager::drawActionButtons(const PomodoroEngine &engine) {
  int btnY = 184;
  int btnH = 50;

  // Botón Principal Hero: INICIAR / PAUSAR (x: 52..214, w: 162)
  bool running = engine.isRunning();
  uint16_t mainBg = running ? COLOR_CARD : COLOR_ACCENT_EMERALD;
  uint16_t mainText = running ? COLOR_TEXT_PRIMARY : 0x0000;

  _tft.fillRoundRect(52, btnY, 162, btnH, 5, mainBg);
  _tft.drawRoundRect(52, btnY, 162, btnH, 5,
                     running ? COLOR_TEXT_PRIMARY : COLOR_ACCENT_EMERALD);
  _tft.setTextColor(mainText, mainBg);
  _tft.drawCentreString(running ? "❚❚ PAUSAR" : "▶ INICIAR", 133, btnY + 16, 2);

  // Botón Auxiliar Reset (x: 220..264, w: 44)
  _tft.fillRoundRect(220, btnY, 44, btnH, 4, COLOR_CARD);
  _tft.drawRoundRect(220, btnY, 44, btnH, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString("RES", 242, btnY + 16, 1);

  // Botón Auxiliar Skip (x: 270..314, w: 44)
  _tft.fillRoundRect(270, btnY, 44, btnH, 4, COLOR_CARD);
  _tft.drawRoundRect(270, btnY, 44, btnH, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString("SKIP", 292, btnY + 16, 1);
}

void UIManager::updatePomodoroTimer(const PomodoroEngine &engine,
                                    const CalendarManager & /*calendar*/,
                                    const BatteryManager & /*battery*/,
                                    bool /*wifiConnected*/) {
  drawCountdown(engine);
  drawProgressBar(engine);
}

UIButtonAction UIManager::handlePomodoroTouch(int x, int y) {
  // Presets verticales (x: 220..314)
  if (x >= 220 && x <= 314) {
    if (y >= 28 && y <= 74)
      return UI_ACTION_SELECT_25;
    if (y >= 78 && y <= 126)
      return UI_ACTION_SELECT_50;
    if (y >= 130 && y <= 178)
      return UI_ACTION_SELECT_90;
  }

  // Botonera de control inferior (y: 184..238)
  if (y >= 184 && y <= 238) {
    if (x >= 52 && x <= 214)
      return UI_ACTION_PLAY_PAUSE;
    if (x >= 220 && x <= 264)
      return UI_ACTION_RESET;
    if (x >= 270 && x <= 314)
      return UI_ACTION_SKIP;
  }

  return UI_ACTION_NONE;
}

// ============================================================================
// 4. APLICACIÓN AGENDA ACADÉMICA UNED (REDiseño TOTAL)
// ============================================================================
void UIManager::drawAgendaView(CalendarManager &calendar,
                               const BatteryManager &battery, int scrollOffset,
                               int selectedIndex) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(2, battery, true); // Slot 2 = CAL

  // Obtener items de la agenda
  AgendaListItem items[24];
  int totalItems = calendar.getAgendaItems(items, 24);

  // Asegurar límites de scrollOffset
  if (scrollOffset < 0)
    scrollOffset = 0;
  if (totalItems > 3 && scrollOffset > totalItems - 3) {
    scrollOffset = totalItems - 3;
  } else if (totalItems <= 3) {
    scrollOffset = 0;
  }

  // 1. Barra de Encabezado Superior (x: 52..314, y: 4..28)
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
  _tft.drawString("AGENDA UNED", 54, 7, 2);

  // Contador de elementos: "1-3 de 7"
  if (totalItems > 0) {
    char countBuf[24];
    int startIdx = scrollOffset + 1;
    int endIdx = min(scrollOffset + 3, totalItems);
    snprintf(countBuf, sizeof(countBuf), "%d-%d de %d", startIdx, endIdx,
             totalItems);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawString(countBuf, 168, 8, 1);
  }

  // Botón Scroll Arriba [ ▲ ] (x: 232..268, y: 3..27)
  bool canScrollUp = (scrollOffset > 0);
  _tft.fillRoundRect(232, 3, 36, 24, 3,
                     canScrollUp ? COLOR_CARD_HL : COLOR_CARD);
  _tft.drawRoundRect(232, 3, 36, 24, 3,
                     canScrollUp ? COLOR_ACCENT_CYAN : COLOR_BORDER);
  _tft.setTextColor(canScrollUp ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED,
                    canScrollUp ? COLOR_CARD_HL : COLOR_CARD);
  _tft.drawCentreString("^", 250, 6, 2);

  // Botón Scroll Abajo [ ▼ ] (x: 274..310, y: 3..27)
  bool canScrollDown = (scrollOffset + 3 < totalItems);
  _tft.fillRoundRect(274, 3, 36, 24, 3,
                     canScrollDown ? COLOR_CARD_HL : COLOR_CARD);
  _tft.drawRoundRect(274, 3, 36, 24, 3,
                     canScrollDown ? COLOR_ACCENT_CYAN : COLOR_BORDER);
  _tft.setTextColor(canScrollDown ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED,
                    canScrollDown ? COLOR_CARD_HL : COLOR_CARD);
  _tft.drawCentreString("v", 292, 6, 2);

  // 2. Renderizado de 3 Tarjetas Visuales Grandes (y: 32..228)
  if (totalItems == 0) {
    _tft.fillRoundRect(52, 60, 262, 100, 6, COLOR_CARD);
    _tft.drawRoundRect(52, 60, 262, 100, 6, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("Sin eventos pendientes", 183, 90, 2);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawCentreString("Sincroniza Google Calendar", 183, 115, 1);
  } else {
    for (int i = 0; i < 3; i++) {
      int itemIdx = scrollOffset + i;
      int cardY = 32 + i * 66;

      if (itemIdx < totalItems) {
        const AgendaListItem &item = items[itemIdx];

        // Fondo de la tarjeta y borde
        _tft.fillRoundRect(52, cardY, 262, 62, 5, COLOR_CARD);
        _tft.drawRoundRect(52, cardY, 262, 62, 5, COLOR_BORDER);

        // Barra lateral indicadora de color (x: 54..58)
        _tft.fillRoundRect(54, cardY + 4, 4, 54, 2, item.color);

        // Fila 1: Fecha / Badge + Cuenta Atras
        _tft.setTextColor(item.color, COLOR_CARD);
        _tft.drawString(item.dateStr, 64, cardY + 6, 1);

        _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
        _tft.drawRightString(item.countdown, 304, cardY + 6, 1);

        // Fila 2: Título en letra clara
        String dispTitle = item.title;
        if (dispTitle.length() > 21) {
          dispTitle = dispTitle.substring(0, 19) + "..";
        }
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawString(dispTitle, 64, cardY + 20, 2);

        // Fila 3: Horario y Ubicación
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        String sub = item.timeStr;
        if (sub.length() > 32)
          sub = sub.substring(0, 30) + "..";
        _tft.drawString(sub, 64, cardY + 44, 1);
      }
    }
  }

  // 3. Modal de Detalle de Evento (si selectedIndex >= 0)
  if (selectedIndex >= 0 && selectedIndex < totalItems) {
    const AgendaListItem &sel = items[selectedIndex];

    // Modal centrado en x: 50..314 (ancho 264), y: 10..232 (alto 222)
    _tft.fillRoundRect(50, 10, 264, 222, 6, COLOR_BG);
    _tft.drawRoundRect(50, 10, 264, 222, 6, sel.color);
    _tft.drawRoundRect(51, 11, 262, 220, 5, COLOR_BORDER);

    // Cabecera del modal: Tipo y botón [X]
    _tft.fillRoundRect(52, 12, 260, 26, 4, COLOR_CARD_HL);
    _tft.setTextColor(sel.color, COLOR_CARD_HL);
    const char *tagStr =
        (sel.type == 0)
            ? "HITO UNED / PEC"
            : ((sel.type == 1) ? "TUTORIA ACADEMICA" : "SESION DE ESTUDIO");
    _tft.drawString(tagStr, 62, 18, 1);

    // Botón Cerrar [ X ] (x: 284..308, y: 14..34)
    _tft.fillRoundRect(284, 14, 24, 20, 3, COLOR_CARD);
    _tft.drawRoundRect(284, 14, 24, 20, 3, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("X", 296, 16, 2);

    // Título Completo de la Materia / Evento (en 1 o 2 líneas)
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
    if (sel.title.length() <= 20) {
      _tft.drawString(sel.title, 60, 46, 2);
    } else {
      int spaceIdx = sel.title.lastIndexOf(' ', 19);
      if (spaceIdx <= 0)
        spaceIdx = 19;
      String line1 = sel.title.substring(0, spaceIdx);
      String line2 = sel.title.substring(spaceIdx + 1);
      if (line2.length() > 22)
        line2 = line2.substring(0, 20) + "..";
      _tft.drawString(line1, 60, 42, 2);
      _tft.drawString(line2, 60, 60, 2);
    }

    // Información detallada
    int infoY = 82;
    _tft.drawFastHLine(60, infoY, 244, COLOR_BORDER);
    infoY += 6;

    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawString("Fecha:", 60, infoY, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
    _tft.drawString(sel.dateStr, 110, infoY, 1);
    infoY += 16;

    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawString("Horario:", 60, infoY, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
    _tft.drawString(sel.timeStr, 110, infoY, 1);
    infoY += 16;

    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawString("Ubicacion:", 60, infoY, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
    String loc = sel.location;
    if (loc.length() > 22)
      loc = loc.substring(0, 20) + "..";
    _tft.drawString(loc, 120, infoY, 1);
    infoY += 16;

    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawString("Cuenta atras:", 60, infoY, 1);
    _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_BG);
    _tft.drawString(sel.countdown, 138, infoY, 1);

    // Botón de Acción Principal: [ ▶ ESTUDIAR AHORA ] (y: 178..220)
    _tft.fillRoundRect(60, 178, 244, 42, 6, COLOR_ACCENT_EMERALD);
    _tft.drawRoundRect(60, 178, 244, 42, 6, COLOR_TEXT_PRIMARY);
    _tft.setTextColor(COLOR_BG, COLOR_ACCENT_EMERALD);
    _tft.drawCentreString("ESTUDIAR AHORA", 182, 190, 2);
  }
}

void UIManager::updateAgendaClock(CalendarManager & /*calendar*/,
                                  const WeatherManager & /*weather*/) {}

bool UIManager::handleAgendaTouch(int x, int y, CalendarManager &calendar,
                                  int &scrollOffset, int &selectedIndex,
                                  bool *outStartFocus, String *outFocusTitle) {
  if (outStartFocus)
    *outStartFocus = false;
  if (outFocusTitle)
    *outFocusTitle = "";

  AgendaListItem items[24];
  int totalItems = calendar.getAgendaItems(items, 24);

  // 1. Si el modal esta abierto:
  if (selectedIndex >= 0 && selectedIndex < totalItems) {
    // A. Boton de cierre [ X ] (x: 275..314, y: 10..40)
    if (x >= 275 && x <= 314 && y >= 10 && y <= 40) {
      selectedIndex = -1;
      return true;
    }

    // B. Boton [ ESTUDIAR AHORA ] (x: 60..304, y: 175..225)
    if (x >= 60 && x <= 304 && y >= 175 && y <= 225) {
      if (outStartFocus)
        *outStartFocus = true;
      if (outFocusTitle)
        *outFocusTitle = items[selectedIndex].title;
      selectedIndex = -1;
      return true;
    }

    // Tocar fuera del modal tambien lo cierra
    if (x < 50 || x > 314 || y < 10 || y > 232) {
      selectedIndex = -1;
      return true;
    }

    return false;
  }

  // 2. Si estamos en la lista general:
  // A. Botones de Scroll Arriba y Abajo (y: 0..30)
  if (y >= 0 && y <= 30) {
    // Scroll Arriba [ ^ ] (x: 228..268)
    if (x >= 228 && x <= 268) {
      if (scrollOffset > 0) {
        scrollOffset--;
        return true;
      }
      return false;
    }
    // Scroll Abajo [ v ] (x: 270..314)
    if (x >= 270 && x <= 314) {
      if (scrollOffset + 3 < totalItems) {
        scrollOffset++;
        return true;
      }
      return false;
    }
  }

  // B. Pulsacion sobre una de las 3 tarjetas (x: 52..314)
  if (x >= 52 && x <= 314) {
    // Tarjeta 0: y: 32..94
    if (y >= 32 && y <= 94) {
      int clicked = scrollOffset + 0;
      if (clicked < totalItems) {
        selectedIndex = clicked;
        return true;
      }
    }
    // Tarjeta 1: y: 98..160
    else if (y >= 98 && y <= 160) {
      int clicked = scrollOffset + 1;
      if (clicked < totalItems) {
        selectedIndex = clicked;
        return true;
      }
    }
    // Tarjeta 2: y: 164..226
    else if (y >= 164 && y <= 226) {
      int clicked = scrollOffset + 2;
      if (clicked < totalItems) {
        selectedIndex = clicked;
        return true;
      }
    }
  }

  return false;
}

// ============================================================================
// 5. APLICACIÓN DEDICADA: RELOJ NOCTURNO
// ============================================================================
void UIManager::drawNightClockView(const BatteryManager &battery,
                                   const WeatherManager &weather) {
  _tft.fillScreen(COLOR_BG);
  updateNightClock(battery, weather);
}

void UIManager::updateNightClock(const BatteryManager &battery,
                                 const WeatherManager &weather) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  char timeBuf[16];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t->tm_hour, t->tm_min);

  _tft.fillRect(20, 60, 280, 80, COLOR_BG);
  _tft.setTextColor(0x7800, COLOR_BG); // Rojo tenue anti-fatiga
  _tft.drawCentreString(timeBuf, 160, 68, 7);

  char wBuf[32];
  if (weather.hasData()) {
    snprintf(wBuf, sizeof(wBuf), "%.1f C  |  Bateria: %d%%",
             weather.getTemperature(), battery.getPercentage());
  } else {
    snprintf(wBuf, sizeof(wBuf), "Bateria: %d%%", battery.getPercentage());
  }
  _tft.setTextColor(0x4000, COLOR_BG);
  _tft.drawCentreString(wBuf, 160, 160, 2);
  _tft.drawCentreString("Toca la pantalla para salir", 160, 210, 1);
}

bool UIManager::handleNightClockTouch(int /*x*/, int /*y*/) { return true; }

// ============================================================================
// 6. APLICACIÓN DEDICADA: LÁMPARA RGB & RELAX CONSOLE (Slot 3)
// ============================================================================
void UIManager::drawRgbLampView(int currentMode,
                                const BatteryManager &battery) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(3, battery, true); // Slot 3 = LAMP

  const char *modes[5] = {"0. APAGADO", "1. CALIDO 3000K", "2. CIAN ESTUDIO",
                          "3. AZUL NOCTURNO", "4. EFECTO VELA"};
  const uint16_t swatchColors[5] = {
      0x2945,            // 0. APAGADO (grafito oscuro)
      0xFFF0,            // 1. BLANCO CALIDO
      COLOR_ACCENT_CYAN, // 2. CIAN ESTUDIO
      0x223F,            // 3. AZUL NOCTURNO
      COLOR_ACCENT_CORAL // 4. EFECTO VELA
  };

  // 1. Orbe Central de Halo Simulado (x: 52..156, y: 10..180)
  _tft.fillRoundRect(52, 10, 104, 172, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 10, 104, 172, 6, COLOR_BORDER);

  int cx = 104;
  int cy = 86;
  uint16_t activeCol = swatchColors[currentMode % 5];

  // Simulación de resplandor ambiental
  _tft.drawCircle(cx, cy, 38, activeCol);
  _tft.drawCircle(cx, cy, 32, activeCol);
  _tft.fillCircle(cx, cy, 26, activeCol);
  _tft.fillCircle(cx, cy, 14, COLOR_BG);

  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("AURA LED", cx, 144, 2);

  // Botón para saltar a Relax Box
  _tft.fillRoundRect(52, 188, 104, 46, 4, COLOR_CARD_HL);
  _tft.drawRoundRect(52, 188, 104, 46, 4, COLOR_ACCENT_MINT);
  _tft.setTextColor(COLOR_ACCENT_MINT, COLOR_CARD_HL);
  _tft.drawCentreString("RELAX 🫁", cx, 202, 2);

  // 2. Columna Derecha con los 5 Modos Táctiles (x: 162..314, y: 10..234)
  for (int i = 0; i < 5; i++) {
    int y = 10 + i * 45;
    bool active = (currentMode == i);

    _tft.fillRoundRect(162, y, 152, 40, 4, active ? COLOR_CARD_HL : COLOR_CARD);
    _tft.drawRoundRect(162, y, 152, 40, 4,
                       active ? COLOR_TEXT_PRIMARY : COLOR_BORDER);

    // Muestra de color (swatch circular)
    _tft.fillCircle(178, y + 20, 8, swatchColors[i]);
    if (active) {
      _tft.drawCircle(178, y + 20, 10, COLOR_TEXT_PRIMARY);
    }

    _tft.setTextColor(active ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY,
                      active ? COLOR_CARD_HL : COLOR_CARD);
    _tft.drawString(modes[i], 194, y + 12, 1);
  }
}

int UIManager::handleRgbLampTouch(int x, int y, bool *outBack) {
  *outBack = false;

  // Botón Relax Box (x: 52..156, y: 188..234)
  if (x >= 52 && x <= 156 && y >= 188 && y <= 234) {
    return 10; // Ir a Relax Box
  }

  // Selección de los 5 modos a la derecha (x: 162..314)
  if (x >= 162 && x <= 314) {
    for (int i = 0; i < 5; i++) {
      int rowY = 10 + i * 45;
      if (y >= rowY && y <= rowY + 40) {
        return i;
      }
    }
  }

  return -1;
}

// ============================================================================
// 7. APLICACIÓN DEDICADA: RESPIRACIÓN RELAX BOX
// ============================================================================
void UIManager::drawBreathingView(int phase, int secondsLeft, int cycle,
                                  const BatteryManager &battery) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(3, battery, true);

  const char *phaseNames[4] = {"INHALA", "MANTEN", "EXHALA", "PAUSA"};
  const uint16_t phaseColors[4] = {
      COLOR_ACCENT_MINT, // Inhala
      COLOR_ACCENT_CYAN, // Mantén
      0x435F,            // Exhala
      COLOR_TEXT_MUTED   // Pausa
  };
  uint16_t col = phaseColors[phase % 4];

  _tft.setTextColor(col, COLOR_BG);
  _tft.drawCentreString(phaseNames[phase % 4], 183, 20, 4);

  int cx = 183, cy = 105;
  _tft.fillCircle(cx, cy, 46, COLOR_CARD);
  _tft.drawCircle(cx, cy, 46, col);
  _tft.drawCircle(cx, cy, 47, col);
  _tft.drawCircle(cx, cy, 48, col);

  char sBuf[8];
  snprintf(sBuf, sizeof(sBuf), "%d", secondsLeft);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(sBuf, cx, cy - 14, 6);

  char cBuf[32];
  snprintf(cBuf, sizeof(cBuf), "Ciclo %d / 4  |  Tecnica 4-4-4-4", cycle);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
  _tft.drawCentreString(cBuf, cx, 165, 2);

  // Botón Salir
  _tft.fillRoundRect(80, 192, 206, 38, 4, COLOR_CARD);
  _tft.drawRoundRect(80, 192, 206, 38, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("FINALIZAR RESPIRACION", 183, 202, 2);
}

bool UIManager::handleBreathingTouch(int x, int y) {
  if (x >= 52 && y >= 180)
    return true;
  return false;
}

// ============================================================================
// 8. CONTROL CENTER / AJUSTES DEL SISTEMA (Slot 4)
// ============================================================================
void UIManager::drawSettingsView(const WiFiConfig &wifi,
                                 const BatteryManager &battery, int brightness,
                                 bool autoBrightness, bool soundMuted,
                                 int timeoutMinutes, int deskStyle) {
  _deskStyle = (deskStyle >= 0) ? (deskStyle % 5) : _deskStyle;
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(4, battery, wifi.isConnected()); // Slot 4 = SYS

  // 1. Tarjeta Brillo PWM (x: 52..178, y: 10..106)
  _tft.fillRoundRect(52, 10, 126, 96, 5, COLOR_CARD);
  _tft.drawRoundRect(52, 10, 126, 96, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("BRILLO TFT", 62, 18, 2);

  char bBuf[16];
  snprintf(bBuf, sizeof(bBuf), "%d%%", (brightness * 100) / 255);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(bBuf, 62, 38, 4);

  // Botones [ - ] [ + ]
  _tft.fillRoundRect(60, 68, 50, 30, 3, COLOR_CARD_HL);
  _tft.drawRoundRect(60, 68, 50, 30, 3, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString("-", 85, 72, 4);

  _tft.fillRoundRect(118, 68, 50, 30, 3, COLOR_CARD_HL);
  _tft.drawRoundRect(118, 68, 50, 30, 3, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString("+", 143, 72, 4);

  // 2. Tarjeta Audio SC8002B (x: 188..314, y: 10..106)
  _tft.fillRoundRect(188, 10, 126, 96, 5, COLOR_CARD);
  _tft.drawRoundRect(188, 10, 126, 96, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("ALARMA 3s", 198, 18, 2);

  _tft.fillRoundRect(196, 50, 110, 44, 4,
                     soundMuted ? COLOR_CARD_HL : COLOR_ACCENT_EMERALD);
  _tft.drawRoundRect(196, 50, 110, 44, 4,
                     soundMuted ? COLOR_BORDER : COLOR_ACCENT_EMERALD);
  _tft.setTextColor(soundMuted ? COLOR_TEXT_PRIMARY : 0x0000,
                    soundMuted ? COLOR_CARD_HL : COLOR_ACCENT_EMERALD);
  _tft.drawCentreString(soundMuted ? "SILENCIADO" : "SONIDO ON", 251, 64, 2);

  // 3. Tarjeta Reposo / Standby (x: 52..178, y: 114..180)
  _tft.fillRoundRect(52, 114, 126, 66, 5, COLOR_CARD);
  _tft.drawRoundRect(52, 114, 126, 66, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("REPOSO AUTO", 62, 122, 1);

  char tBuf[16];
  snprintf(tBuf, sizeof(tBuf),
           timeoutMinutes == 0 ? "DESACTIVADO" : "%d MINUTOS", timeoutMinutes);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawString(tBuf, 62, 142, 2);

  // 4. Tarjeta Wi-Fi 2.4 GHz (x: 188..314, y: 114..180)
  _tft.fillRoundRect(188, 114, 126, 66, 5, COLOR_CARD);
  _tft.drawRoundRect(188, 114, 126, 66, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("RED WI-FI", 198, 122, 1);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawString(wifi.isConnected() ? wifi.getSSID().substring(0, 12).c_str()
                                     : "DESCONECTADO",
                  198, 142, 1);

  // 5. Botonera Inferior: Telemetría Batería & Hardware (x: 52..314, y:
  // 186..234)
  _tft.fillRoundRect(52, 186, 126, 46, 4, COLOR_CARD);
  _tft.drawRoundRect(52, 186, 126, 46, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("BATERIA 5Ah", 115, 196, 2);
  _tft.setTextColor(COLOR_ACCENT_EMERALD, COLOR_CARD);
  _tft.drawCentreString(
      battery.getVoltage() < 0.5f ? "USB 5.0V ⚡" : "3.7V LiPo", 115, 214, 1);

  // 6. Tarjeta Selector de Estilo / Tema Standby (x: 188..314, y: 186..234)
  _tft.fillRoundRect(188, 186, 126, 46, 4, COLOR_CARD_HL);
  _tft.drawRoundRect(188, 186, 126, 46, 4, COLOR_ACCENT_CYAN);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString("ESTILO DESK", 251, 190, 1);
  const char *styleNames[5] = {"0. MINIMAL", "1. CYBERDECK", "2. ANALOGICO",
                               "3. DASHBOARD", "4. FLIP CLOCK"};
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD_HL);
  _tft.drawCentreString(styleNames[_deskStyle % 5], 251, 204, 2);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD_HL);
  _tft.drawCentreString("Toca para cambiar", 251, 220, 1);
}

int UIManager::handleSettingsTouch(int x, int y, bool *outToggleAuto,
                                   bool *outBriDown, bool *outBriUp,
                                   bool *outToggleMute, bool *outTimeoutToggle,
                                   bool *outNightClock, bool *outOpenWifi,
                                   bool *outBack) {
  *outToggleAuto = false;
  *outBriDown = false;
  *outBriUp = false;
  *outToggleMute = false;
  *outTimeoutToggle = false;
  *outNightClock = false;
  *outOpenWifi = false;
  *outBack = false;

  // Brillo [-] y [+]
  if (x >= 52 && x <= 178 && y >= 10 && y <= 106) {
    if (x <= 114)
      *outBriDown = true;
    else
      *outBriUp = true;
    return 1;
  }

  // Audio Toggle
  if (x >= 188 && x <= 314 && y >= 10 && y <= 106) {
    *outToggleMute = true;
    return 2;
  }

  // Timeout Reposo
  if (x >= 52 && x <= 178 && y >= 114 && y <= 180) {
    *outTimeoutToggle = true;
    return 3;
  }

  // Wi-Fi Config
  if (x >= 188 && x <= 314 && y >= 114 && y <= 180) {
    *outOpenWifi = true;
    return 4;
  }

  return 0;
}

// ============================================================================
// 9. VISTA TELEMETRÍA DE BATERÍA 5000 mAh
// ============================================================================
void UIManager::drawBatteryView(const BatteryManager &battery,
                                bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(4, battery, wifiConnected);

  int pct = battery.getPercentage();
  float v = battery.getVoltage();
  bool charging = battery.isCharging();
  float hrs = battery.getEstimatedHoursLeft();

  _tft.fillRoundRect(52, 10, 262, 140, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 10, 262, 140, 6, COLOR_BORDER);

  drawAppColorIcon(84, 52, 7);

  char pctStr[16];
  snprintf(pctStr, sizeof(pctStr), "%d", pct);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  int numW = _tft.drawString(pctStr, 114, 30, 6);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("%", 114 + numW + 3, 38, 4);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(charging ? "Cargando LiPo..." : "Descarga normal", 114, 74,
                  2);

  // Barra analógica
  int barW = 230;
  _tft.fillRoundRect(68, 96, barW, 10, 3, COLOR_BG);
  _tft.drawRoundRect(68, 96, barW, 10, 3, COLOR_BORDER);
  int fW = map(constrain(pct, 0, 100), 0, 100, 0, barW - 2);
  if (fW > 0) {
    _tft.fillRoundRect(69, 97, fW, 8, 2, COLOR_ACCENT_EMERALD);
  }

  char infoBuf[48];
  if (v < 0.5f) {
    snprintf(infoBuf, sizeof(infoBuf), "Alimentacion USB (5.0V)  |  18.5 Wh");
  } else {
    snprintf(infoBuf, sizeof(infoBuf), "%.2f V  |  ~%.1f h restantes", v, hrs);
  }
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString(infoBuf, 183, 120, 2);

  _tft.fillRoundRect(52, 160, 262, 68, 5, COLOR_CARD);
  _tft.drawRoundRect(52, 160, 262, 68, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawCentreString("Cargador TP4056 USB-C Integrado", 183, 175, 2);
  _tft.drawCentreString("Proteccion de Litio DW01 | 18.5 Wh", 183, 198, 1);
}

bool UIManager::handleBatteryTouch(int /*x*/, int /*y*/) { return true; }

// ============================================================================
// 10. VISTAS ESTADÍSTICAS, INFO & WI-FI (CANVAS DERECHO x: 52..314)
// ============================================================================
void UIManager::drawStatsView(StorageManager &storage,
                              const PomodoroEngine &pomodoro,
                              const BatteryManager &battery,
                              bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(4, battery, wifiConnected);

  int sessionsToday = pomodoro.getCompletedSessionsToday();
  int focusMinToday = sessionsToday * pomodoro.getFocusMinutes();
  int totalLogged = storage.getTotalLoggedSessions();

  _tft.fillRoundRect(52, 10, 126, 76, 5, COLOR_CARD);
  _tft.drawRoundRect(52, 10, 126, 76, 5, COLOR_BORDER);
  _tft.fillCircle(64, 22, 3, COLOR_ACCENT_CORAL);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("SESIONES HOY", 72, 18, 1);
  char sBuf[12];
  snprintf(sBuf, sizeof(sBuf), "%d", sessionsToday);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(sBuf, 64, 34, 6);

  _tft.fillRoundRect(188, 10, 126, 76, 5, COLOR_CARD);
  _tft.drawRoundRect(188, 10, 126, 76, 5, COLOR_BORDER);
  _tft.fillCircle(200, 22, 3, COLOR_ACCENT_CYAN);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("TIEMPO TOTAL", 208, 18, 1);
  char mBuf[16];
  snprintf(mBuf, sizeof(mBuf), "%dh %02dm", focusMinToday / 60,
           focusMinToday % 60);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(mBuf, 200, 40, 4);

  _tft.fillRoundRect(52, 96, 262, 130, 5, COLOR_CARD);
  _tft.drawRoundRect(52, 96, 262, 130, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("HISTORIAL MICROSD", 64, 108, 1);

  char lBuf[64];
  snprintf(lBuf, sizeof(lBuf), "Total acumulado: %d sesiones", totalLogged);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(lBuf, 64, 130, 2);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Archivo: /sessions.csv", 64, 160, 2);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("Compatible con hojas de calculo Excel/Python", 64, 188, 1);
}

bool UIManager::handleStatsTouch(int /*x*/, int /*y*/) { return true; }

void UIManager::drawInfoView(const BatteryManager &battery,
                             StorageManager &storage,
                             const PomodoroEngine & /*pomodoro*/,
                             bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(4, battery, wifiConnected);

  _tft.fillRoundRect(52, 10, 262, 218, 5, COLOR_CARD);
  _tft.drawRoundRect(52, 10, 262, 218, 5, COLOR_BORDER);

  int y = 18;
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString("CYBER-STUDIO DECK OS v2", 64, y, 2);
  y += 22;

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Placa: Guition ESP32-2432S024C", 64, y, 1);
  y += 18;
  _tft.drawString("Pantalla: 2.4\" TFT 320x240 ILI9341", 64, y, 1);
  y += 18;
  _tft.drawString("Tactil: CST820 Capacitivo I2C", 64, y, 1);
  y += 18;
  _tft.drawString("Bateria: LiPo 5000 mAh 3.7V", 64, y, 1);
  y += 18;
  _tft.drawString("Audio: SC8002B (Alarma 3s)", 64, y, 1);
  y += 18;
  _tft.drawString("Almacenamiento: MicroSD FAT32", 64, y, 1);
  y += 22;
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawString("Grado en Ingenieria Informatica", 64, y, 2);
  y += 18;
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("UNED Centro Asociado Avila", 64, y, 1);
}

bool UIManager::handleInfoTouch(int /*x*/, int /*y*/) { return true; }

void UIManager::drawWiFiScanView(const WiFiConfig &wifi,
                                 const BatteryManager &battery, bool scanning,
                                 int page) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(4, battery, wifi.isConnected());

  if (scanning) {
    _tft.fillRoundRect(60, 80, 244, 70, 5, COLOR_CARD);
    _tft.drawRoundRect(60, 80, 244, 70, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("Buscando redes 2.4 GHz...", 182, 100, 2);
    return;
  }

  int totalNets = wifi.getNetworkCount();
  const int netsPerPage = 4;
  int startIndex = page * netsPerPage;

  if (totalNets == 0) {
    _tft.fillRoundRect(60, 80, 244, 70, 5, COLOR_CARD);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("No se encontraron redes", 182, 100, 2);
  } else {
    int showCount = min(netsPerPage, totalNets - startIndex);
    for (int i = 0; i < showCount; i++) {
      int idx = startIndex + i;
      DiscoveredNetwork net = wifi.getNetwork(idx);
      String ssid = net.ssid;
      int32_t rssi = net.rssi;
      bool open = !net.isEncrypted;

      int y = 10 + i * 40;
      _tft.fillRoundRect(52, y, 262, 36, 4, COLOR_CARD);
      _tft.drawRoundRect(52, y, 262, 36, 4, COLOR_BORDER);

      if (ssid.length() > 20)
        ssid = ssid.substring(0, 18) + "..";
      _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
      _tft.drawString(ssid, 60, y + 10, 2);

      char inf[16];
      snprintf(inf, sizeof(inf), "%d dBm", (int)rssi);
      _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
      _tft.drawRightString(inf, 304, y + 12, 1);
    }
  }

  // Botón reescanear
  _tft.fillRoundRect(52, 186, 262, 44, 4, COLOR_CARD_HL);
  _tft.drawRoundRect(52, 186, 262, 44, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString("REESCANEAR REDES", 183, 198, 2);
}

int UIManager::handleWiFiScanTouch(int x, int y, int netCount, int page,
                                   bool *outRescan, bool *outBack,
                                   bool *outNextPage) {
  *outRescan = false;
  *outBack = false;
  *outNextPage = false;

  const int netsPerPage = 4;
  int startIndex = page * netsPerPage;
  for (int i = 0; i < netsPerPage; i++) {
    int rowY = 10 + i * 40;
    if (y >= rowY && y <= rowY + 36 && x >= 52) {
      int chosen = startIndex + i;
      if (chosen < netCount) {
        return chosen;
      }
    }
  }

  if (y >= 180 && x >= 52) {
    *outRescan = true;
    return -1;
  }

  return -1;
}

void UIManager::drawWiFiConnecting(const String &ssid) {
  _tft.fillScreen(COLOR_BG);
  _tft.fillRoundRect(52, 60, 262, 110, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 60, 262, 110, 6, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("Conectando a Wi-Fi...", 183, 80, 2);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawCentreString(ssid, 183, 110, 2);
}

void UIManager::drawWiFiResult(bool success, const String &info) {
  _tft.fillScreen(COLOR_BG);
  _tft.fillRoundRect(52, 60, 262, 110, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 60, 262, 110, 6,
                     success ? COLOR_ACCENT_EMERALD : COLOR_ACCENT_CORAL);
  _tft.setTextColor(success ? COLOR_ACCENT_EMERALD : COLOR_ACCENT_CORAL,
                    COLOR_CARD);
  _tft.drawCentreString(success ? "CONECTADO CON EXITO" : "FALLO DE CONEXION",
                        183, 80, 2);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(info, 183, 115, 2);
}

// ============================================================================
// NUEVAS VISTAS DE DISCIPLINA Y AUTOMATIZACIÓN (CORE STUDYDECK)
// ============================================================================

void UIManager::drawStudyPromptView(const String &subject, int countdownSec,
                                    const BatteryManager &battery,
                                    bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(1, battery, wifiConnected); // Slot 1 (FOCUS) activo

  // Contenedor principal de Alerta de Estudio (x: 52..314, y: 10..230)
  _tft.fillRoundRect(52, 10, 262, 220, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 10, 262, 220, 6, COLOR_ACCENT_AMBER);
  _tft.drawRoundRect(53, 11, 260, 218, 5, COLOR_ACCENT_AMBER);

  // Badge superior
  _tft.fillRoundRect(62, 18, 130, 20, 3, 0x3800);
  _tft.drawRoundRect(62, 18, 130, 20, 3, COLOR_ACCENT_CORAL);
  _tft.setTextColor(COLOR_ACCENT_AMBER, 0x3800);
  _tft.drawCentreString("ALERTA ACADEMICA", 127, 22, 1);

  // Título
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("HORA DE ESTUDIAR", 62, 44, 4);

  // Tarjeta de Asignatura
  _tft.fillRoundRect(62, 78, 242, 44, 4, COLOR_CARD_HL);
  _tft.drawRoundRect(62, 78, 242, 44, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD_HL);
  _tft.drawString("SESION PROGRAMADA UNED:", 70, 84, 1);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD_HL);
  String sub = subject.length() > 0 ? subject : "Sistemas Informaticos";
  if (sub.length() > 22)
    sub = sub.substring(0, 20) + "..";
  _tft.drawString(sub, 70, 100, 2);

  // Contador de cortesía
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("Arrancando automaticamente en:", 62, 130, 1);

  char countBuf[16];
  snprintf(countBuf, sizeof(countBuf), "%d seg", countdownSec);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(countBuf, 62, 146, 4);

  // Botón 1: EMPEZAR YA
  _tft.fillRoundRect(62, 180, 126, 42, 5, 0x2444);
  _tft.drawRoundRect(62, 180, 126, 42, 5, COLOR_ACCENT_EMERALD);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, 0x2444);
  _tft.drawCentreString("EMPEZAR YA", 125, 192, 2);

  // Botón 2: POSPONER 5M
  _tft.fillRoundRect(194, 180, 110, 42, 5, COLOR_CARD_HL);
  _tft.drawRoundRect(194, 180, 110, 42, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD_HL);
  _tft.drawCentreString("POSPONER 5m", 249, 192, 2);
}

void UIManager::updateStudyPromptCountdown(int countdownSec) {
  _tft.fillRect(62, 146, 120, 28, COLOR_CARD);
  char countBuf[16];
  snprintf(countBuf, sizeof(countBuf), "%d seg", countdownSec);
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(countBuf, 62, 146, 4);
}

UIButtonAction UIManager::handleStudyPromptTouch(int x, int y) {
  if (x >= 62 && x <= 188 && y >= 180 && y <= 222) {
    return UI_ACTION_PROMPT_START;
  }
  if (x >= 194 && x <= 304 && y >= 180 && y <= 222) {
    return UI_ACTION_PROMPT_SNOOZE;
  }
  return UI_ACTION_NONE;
}

void UIManager::drawDailyCheckpointView(int checkpointIndex,
                                        CalendarManager &calendar,
                                        const BatteryManager &battery,
                                        bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(2, battery, wifiConnected); // Slot 2 (CAL) activo

  // Contenedor principal de Checkpoint (x: 52..314, y: 10..230)
  _tft.fillRoundRect(52, 10, 262, 220, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 10, 262, 220, 6, COLOR_ACCENT_CYAN);

  const char *chkTimes[5] = {"09:00", "12:00", "15:30", "19:30", "22:00"};
  const char *chkNames[5] = {"PLANIFICACION MANANA", "CHECK INTERMEDIO",
                             "ARRANQUE DE TARDE", "CIERRE DE JORNADA",
                             "RADAR NOCTURNO UNED"};

  const char *timeStr = (checkpointIndex >= 0 && checkpointIndex < 5)
                            ? chkTimes[checkpointIndex]
                            : "AVISO";
  const char *nameStr = (checkpointIndex >= 0 && checkpointIndex < 5)
                            ? chkNames[checkpointIndex]
                            : "CHECKPOINT DIARIO";

  _tft.fillRoundRect(62, 18, 120, 18, 3, 0x01E8);
  _tft.setTextColor(COLOR_ACCENT_CYAN, 0x01E8);
  char badgeBuf[32];
  snprintf(badgeBuf, sizeof(badgeBuf), "CHECKPOINT %s", timeStr);
  _tft.drawCentreString(badgeBuf, 122, 22, 1);

  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(nameStr, 62, 42, 2);

  // Tarjeta de Hito UNED
  String examTitle;
  String countdown = calendar.getNextDeadlineCountdown(examTitle);
  _tft.fillRoundRect(62, 68, 242, 54, 4, COLOR_CARD_HL);
  _tft.drawRoundRect(62, 68, 242, 54, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD_HL);
  _tft.drawString("PROXIMO EXAMEN / PEC UNED:", 70, 74, 1);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawString(examTitle.substring(0, 22), 70, 88, 2);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD_HL);
  _tft.drawString(countdown, 70, 106, 2);

  // Tarjeta de Estado de Estudio Hoy
  _tft.fillRoundRect(62, 128, 242, 46, 4, COLOR_CARD_HL);
  _tft.drawRoundRect(62, 128, 242, 46, 4, COLOR_BORDER);
  CalendarEvent todaySes;
  if (calendar.getTodayMainSession(todaySes)) {
    _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD_HL);
    _tft.drawString("SESION HOY: " + todaySes.timeRange, 70, 134, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
    _tft.drawString(todaySes.title.substring(0, 22), 70, 150, 2);
  } else {
    _tft.setTextColor(COLOR_ACCENT_EMERALD, COLOR_CARD_HL);
    _tft.drawString("HOY: ESTUDIO AUTONOMO", 70, 134, 1);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD_HL);
    _tft.drawString("Mantente al dia con las lecturas", 70, 150, 2);
  }

  // Botón inferior CONTINUAR
  _tft.fillRoundRect(62, 182, 242, 40, 5, 0x1A2F);
  _tft.drawRoundRect(62, 182, 242, 40, 5, COLOR_ACCENT_CYAN);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, 0x1A2F);
  _tft.drawCentreString("CONTINUAR (O TOQUE)", 183, 194, 2);
}

bool UIManager::handleDailyCheckpointTouch(int x, int y) {
  if (x >= 48) {
    return true; // Dismiss
  }
  return false;
}
