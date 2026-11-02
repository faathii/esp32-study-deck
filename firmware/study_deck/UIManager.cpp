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

  // 4 Ranuras táctiles verticales ampliadas (60 px de alto cada una para ergonomía total)
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
// ============================================================================
void UIManager::drawWeatherView(const WeatherManager &weather,
                                CalendarManager &calendar,
                                const BatteryManager &battery,
                                bool wifiConnected) {
  // Limpiar pantalla y renderizar el Left Cyber Rail (Slot 0 Activo)
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(0, battery, wifiConnected);

  // 1. Barra Superior del Área de Trabajo (x: 52..314, y: 4..24)
  _tft.setTextDatum(TL_DATUM);
  _tft.fillCircle(54, 13, 3, COLOR_ACCENT_CYAN);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
  _tft.drawString("AVILA", 62, 7, 2);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_BG);
  _tft.drawString(weather.hasData() ? weather.getConditionText().c_str()
                                    : "DESPEJADO",
                  112, 8, 1);

  // Estado batería y wifi en la esquina superior derecha
  int batPct = battery.getPercentage();
  bool usb = (battery.getVoltage() < 0.5f);
  char stBuf[32];
  snprintf(stBuf, sizeof(stBuf), "%s %d%%%s", wifiConnected ? "WIFI" : "OFF",
           batPct, usb ? " 5V" : "");
  _tft.setTextDatum(TR_DATUM);
  _tft.setTextColor(usb ? COLOR_ACCENT_EMERALD : COLOR_TEXT_SECONDARY,
                    COLOR_BG);
  _tft.drawString(stBuf, 314, 7, 1);
  _tft.setTextDatum(TL_DATUM);

  _tft.drawFastHLine(52, 24, 262, COLOR_BORDER);

  // 2. Módulo Atmosférico Hero (x: 52..314, y: 28..116, h: 88)
  _tft.fillRoundRect(52, 28, 262, 88, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 28, 262, 88, 6, COLOR_BORDER);

  // Temperatura gigante (Font 7 o Font 6)
  char tBuf[16];
  if (weather.hasData()) {
    snprintf(tBuf, sizeof(tBuf), "%.1f", weather.getTemperature());
  } else {
    snprintf(tBuf, sizeof(tBuf), "--.-");
  }
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  int tW =
      _tft.drawString(tBuf, 62, 38, 6); // Font 6: Números limpios y grandes
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
  _tft.drawString(" C", 62 + tW + 2, 42, 4);

  // Subtítulo meteorológico con datos de Ávila
  char wSub[40];
  if (weather.hasData()) {
    snprintf(wSub, sizeof(wSub), "%d%% HR  |  Viento: 8 km/h",
             weather.getHumidity());
  } else {
    snprintf(wSub, sizeof(wSub), "Conectando estacion meteorologica...");
  }
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(wSub, 62, 90, 1);

  // Ilustración meteorológica a la derecha de la tarjeta
  drawAppColorIcon(270, 68, 3);

  // 3. Radar Académico UNED & Reloj Vivo (x: 52..314, y: 122..234, h: 112)
  _tft.fillRoundRect(52, 122, 262, 112, 6, COLOR_CARD);
  _tft.drawRoundRect(52, 122, 262, 112, 6, COLOR_BORDER);

  // Reloj digital grande y fecha
  updateWeatherViewClock(weather, calendar);

  // Separador interno
  _tft.drawFastHLine(58, 172, 250, COLOR_BORDER);

  // Información de sesión académica o cuenta atrás de examen
  CalendarEvent todaySession;
  bool hasSession = calendar.getTodayMainSession(todaySession);
  bool ongoing = calendar.isStudySessionOngoing();

  if (ongoing) {
    _tft.fillRoundRect(56, 178, 254, 48, 4, 0x3800);
    _tft.drawRoundRect(56, 178, 254, 48, 4, COLOR_ACCENT_CORAL);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, 0x3800);
    _tft.drawString("SESION EN CURSO", 64, 184, 2);
    _tft.setTextColor(COLOR_ACCENT_AMBER, 0x3800);
    _tft.drawString(todaySession.timeRange + " | " +
                        todaySession.title.substring(0, 20),
                    64, 204, 1);
  } else if (hasSession) {
    _tft.fillCircle(64, 188, 3, COLOR_ACCENT_CYAN);
    _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
    _tft.drawString(todaySession.timeRange, 72, 180, 2);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(todaySession.title.substring(0, 26), 72, 200, 1);
  } else {
    String examTitle;
    String countdown = calendar.getNextDeadlineCountdown(examTitle);
    _tft.fillCircle(64, 188, 3, COLOR_ACCENT_CYAN);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawString("PROXIMO EXAMEN UNED", 72, 178, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(examTitle.substring(0, 20), 72, 192, 2);
    _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
    _tft.drawRightString(countdown, 304, 192, 2);
  }
}

void UIManager::updateWeatherViewClock(const WeatherManager & /*weather*/,
                                       CalendarManager & /*calendar*/) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  char timeBuf[16];
  char dateBuf[48];

  if (t->tm_year > 120) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d:%02d", t->tm_hour, t->tm_min,
             t->tm_sec);
    const char *dias[7] = {"Domingo", "Lunes",   "Martes", "Miercoles",
                           "Jueves",  "Viernes", "Sabado"};
    const char *meses[12] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                             "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
    snprintf(dateBuf, sizeof(dateBuf), "%s, %d %s %d", dias[t->tm_wday],
             t->tm_mday, meses[t->tm_mon], t->tm_year + 1900);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "12:00:00");
    snprintf(dateBuf, sizeof(dateBuf), "Sincronizando hora NTP...");
  }

  // Reloj digital limpio centrado en la parte superior del radar
  _tft.fillRect(58, 128, 250, 40, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(timeBuf, 62, 130, 4);

  // Fecha debajo de la hora
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(dateBuf, 62, 154, 1);
  _tft.setTextDatum(TL_DATUM);
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
                               const WiFiConfig & /*wifi*/,
                               const BatteryManager &battery,
                               const WeatherManager & /*weather*/) {
  _tft.fillScreen(COLOR_BG);
  drawLeftRail(2, battery, true); // Slot 2 = CAL

  CalendarViewMode mode = calendar.getViewMode();

  // Conmutador de 3 Modos Superior (x: 52..314, y: 4..26)
  int tabW = 84;
  const char *tabs[3] = {"HOY", "SEMANA", "MES"};
  for (int i = 0; i < 3; i++) {
    int tx = 52 + i * 89;
    bool active = ((int)mode == i);
    _tft.fillRoundRect(tx, 4, tabW, 22, 3,
                       active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawRoundRect(tx, 4, tabW, 22, 3,
                       active ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
    _tft.setTextColor(active ? COLOR_BG : COLOR_TEXT_SECONDARY,
                      active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawCentreString(tabs[i], tx + tabW / 2, 8, 2);
  }

  // --- MODO 1: HOY & HITOS DE EXÁMENES ---
  if (mode == CAL_VIEW_DAY) {
    String examTitle;
    String countdown = calendar.getNextDeadlineCountdown(examTitle);

    // Tarjeta 1: Próximo Hito / Examen UNED (y: 32..90)
    _tft.fillRoundRect(52, 32, 262, 58, 5, COLOR_CARD);
    _tft.drawRoundRect(52, 32, 262, 58, 5, COLOR_BORDER);
    _tft.fillCircle(64, 46, 4, COLOR_ACCENT_CYAN);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawString("PROXIMO EXAMEN / ENTREGA UNED", 74, 40, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(examTitle.substring(0, 20), 64, 58, 2);
    _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
    _tft.drawRightString(countdown, 304, 58, 2);

    // Tarjeta 2: Sesión de Estudio de Hoy (y: 96..180)
    _tft.fillRoundRect(52, 96, 262, 84, 5, COLOR_CARD);
    _tft.drawRoundRect(52, 96, 262, 84, 5, COLOR_BORDER);

    CalendarEvent session;
    bool hasSession = calendar.getTodayMainSession(session);
    if (hasSession) {
      _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
      _tft.drawString("HORARIO DE ESTUDIO DE HOY:", 64, 104, 1);
      _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
      _tft.drawString(session.timeRange, 64, 118, 4);
      _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
      _tft.drawString(session.title, 64, 154, 2);
    } else {
      _tft.setTextColor(COLOR_ACCENT_EMERALD, COLOR_CARD);
      _tft.drawCentreString("Hoy: Tiempo Libre / Repaso", 183, 118, 2);
      _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
      _tft.drawCentreString("Avanza temas o descansa al 100%", 183, 144, 1);
    }

    // Botón de Inicio Rápido a Pomodoro con la Materia (y: 186..234)
    _tft.fillRoundRect(52, 186, 262, 46, 5, COLOR_CARD_HL);
    _tft.drawRoundRect(52, 186, 262, 46, 5, COLOR_ACCENT_CORAL);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
    _tft.drawCentreString("▶ ABRIR ENFOQUE POMODORO", 183, 200, 2);
  }

  // --- MODO 2: SEMANA EN CURSO ---
  else if (mode == CAL_VIEW_WEEK) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    int curD = (t->tm_year > 120) ? t->tm_mday : 1;
    int curM = (t->tm_year > 120) ? (t->tm_mon + 1) : 11;
    int curY = (t->tm_year > 120) ? (t->tm_year + 1900) : 2026;
    int curWDay = (t->tm_year > 120) ? ((t->tm_wday + 6) % 7) : 6;

    const char *dayNames[7] = {"L", "M", "X", "J", "V", "S", "D"};

    for (int i = 0; i < 7; i++) {
      int offset = i - curWDay;
      time_t dTime = now + (offset * 86400);
      struct tm *dTm = localtime(&dTime);
      int dNum = dTm->tm_mday;
      int dMon = dTm->tm_mon + 1;
      int dYear = dTm->tm_year + 1900;
      bool isToday = (dNum == curD && dMon == curM && dYear == curY);

      int rowY = 32 + i * 29;

      // Etiqueta del día
      _tft.fillRoundRect(52, rowY, 32, 26, 2,
                         isToday ? COLOR_TEXT_PRIMARY : COLOR_CARD);
      _tft.drawRoundRect(52, rowY, 32, 26, 2,
                         isToday ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
      _tft.setTextColor(isToday ? COLOR_BG : COLOR_TEXT_SECONDARY,
                        isToday ? COLOR_TEXT_PRIMARY : COLOR_CARD);
      _tft.drawCentreString(dayNames[i], 68, rowY + 5, 2);

      // Franja de sesión
      _tft.fillRoundRect(88, rowY, 226, 26, 2, COLOR_CARD);
      _tft.drawRoundRect(88, rowY, 226, 26, 2,
                         isToday ? COLOR_ACCENT_CYAN : COLOR_BORDER);

      String sum = calendar.getEventSummaryForDate(dNum, dMon, dYear);
      if (sum.length() > 0) {
        if (sum.length() > 28)
          sum = sum.substring(0, 26) + "..";
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawString(sum, 96, rowY + 6, 1);
      } else {
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawString(isToday ? "Hoy sin sesion agendada" : "-- Libre --", 96,
                        rowY + 6, 1);
      }
    }
  }

  // --- MODO 3: VISTA DE MES ENTERO ---
  else if (mode == CAL_VIEW_MONTH) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    int curY = (t->tm_year > 120) ? (t->tm_year + 1900) : 2026;
    int curM = (t->tm_year > 120) ? (t->tm_mon + 1) : 11;
    int curD = (t->tm_year > 120) ? t->tm_mday : 1;

    int dispM = curM + calendar.getMonthOffset();
    int dispY = curY;
    while (dispM > 12) {
      dispM -= 12;
      dispY++;
    }
    while (dispM < 1) {
      dispM += 12;
      dispY--;
    }

    const char *mesesFull[12] = {
        "ENERO", "FEBRERO", "MARZO",      "ABRIL",   "MAYO",      "JUNIO",
        "JULIO", "AGOSTO",  "SEPTIEMBRE", "OCTUBRE", "NOVIEMBRE", "DICIEMBRE"};

    // Botón mes anterior
    _tft.fillRoundRect(52, 32, 28, 20, 3, COLOR_CARD);
    _tft.drawRoundRect(52, 32, 28, 20, 3, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("<", 66, 34, 2);

    // Título Mes y Año
    char titleBuf[32];
    snprintf(titleBuf, sizeof(titleBuf), "%s %d", mesesFull[dispM - 1], dispY);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
    _tft.drawCentreString(titleBuf, 183, 33, 2);

    // Botón mes siguiente
    _tft.fillRoundRect(286, 32, 28, 20, 3, COLOR_CARD);
    _tft.drawRoundRect(286, 32, 28, 20, 3, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString(">", 300, 34, 2);

    // Cabeceras de días de la semana
    const char *wNames[7] = {"L", "M", "X", "J", "V", "S", "D"};
    for (int col = 0; col < 7; col++) {
      int cx = 56 + col * 37 + 18;
      _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
      _tft.drawCentreString(wNames[col], cx, 58, 1);
    }
    _tft.drawFastHLine(52, 68, 262, COLOR_BORDER);

    int daysInM = 31;
    if (dispM == 2) {
      daysInM =
          (dispY % 4 == 0 && (dispY % 100 != 0 || dispY % 400 == 0)) ? 29 : 28;
    } else if (dispM == 4 || dispM == 6 || dispM == 9 || dispM == 11) {
      daysInM = 30;
    }

    struct tm firstDay = {0};
    firstDay.tm_year = dispY - 1900;
    firstDay.tm_mon = dispM - 1;
    firstDay.tm_mday = 1;
    mktime(&firstDay);
    int startWDay = (firstDay.tm_wday + 6) % 7;

    int row = 0;
    int col = startWDay;
    for (int day = 1; day <= daysInM; day++) {
      int cellX = 54 + col * 37;
      int cellY = 72 + row * 26;

      bool isToday = (dispY == curY && dispM == curM && day == curD);
      bool hasEvent = calendar.hasEventOnDate(day, dispM, dispY);
      bool hasExam = calendar.hasDeadlineOnDate(day, dispM, dispY);

      if (isToday) {
        _tft.fillRoundRect(cellX + 4, cellY, 28, 20, 3, COLOR_TEXT_PRIMARY);
        _tft.drawRoundRect(cellX + 3, cellY - 1, 30, 22, 3, COLOR_ACCENT_CYAN);
        _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
      } else {
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
      }

      _tft.drawCentreString(String(day), cellX + 18, cellY + 3, 2);

      if (hasEvent || hasExam) {
        uint16_t dotCol = hasExam ? COLOR_ACCENT_CORAL
                                  : (isToday ? COLOR_BG : COLOR_ACCENT_CYAN);
        _tft.drawFastHLine(cellX + 10, cellY + 18, 16, dotCol);
      }

      col++;
      if (col > 6) {
        col = 0;
        row++;
      }
    }
  }
}

void UIManager::updateAgendaClock(CalendarManager & /*calendar*/,
                                  const WeatherManager & /*weather*/) {}

bool UIManager::handleAgendaTouch(int x, int y, CalendarManager &calendar,
                                  bool *outAddExam, bool *outSyncGCal,
                                  bool *outBack) {
  *outAddExam = false;
  *outSyncGCal = false;
  *outBack = false;

  // Conmutador de 3 Modos (y: 0..30)
  if (y >= 0 && y <= 30 && x >= 52) {
    if (x >= 52 && x <= 138) {
      calendar.setViewMode(CAL_VIEW_DAY);
      return true;
    }
    if (x >= 140 && x <= 226) {
      calendar.setViewMode(CAL_VIEW_WEEK);
      return true;
    }
    if (x >= 228 && x <= 314) {
      calendar.setViewMode(CAL_VIEW_MONTH);
      return true;
    }
  }

  // Navegación en vista de mes
  if (calendar.getViewMode() == CAL_VIEW_MONTH && y >= 30 && y <= 60) {
    if (x >= 52 && x <= 90) {
      calendar.changeMonth(-1);
      return true;
    }
    if (x >= 270 && x <= 314) {
      calendar.changeMonth(1);
      return true;
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
                                 int timeoutMinutes) {
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

  _tft.fillRoundRect(188, 186, 126, 46, 4, COLOR_CARD);
  _tft.drawRoundRect(188, 186, 126, 46, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("INFO GUITION", 251, 196, 2);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawCentreString("v2.0 Cyber-Studio", 251, 214, 1);
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
  if (sub.length() > 22) sub = sub.substring(0, 20) + "..";
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
  const char *chkNames[5] = {
      "PLANIFICACION MANANA", "CHECK INTERMEDIO",
      "ARRANQUE DE TARDE",    "CIERRE DE JORNADA",
      "RADAR NOCTURNO UNED"};

  const char *timeStr = (checkpointIndex >= 0 && checkpointIndex < 5) ? chkTimes[checkpointIndex] : "AVISO";
  const char *nameStr = (checkpointIndex >= 0 && checkpointIndex < 5) ? chkNames[checkpointIndex] : "CHECKPOINT DIARIO";

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
