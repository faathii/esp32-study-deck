#include "UIManager.h"
#include <time.h>

// ============================================================================
// PALETA MINIMALISTA Y SOBRIA (DARK MONOCHROME / SWISS PRECISION AESTHETIC)
// ============================================================================
#define COLOR_BG 0x0000         // Negro absoluto (OLED Pitch Black)
#define COLOR_STATUS_BAR 0x0000 // Barra de estado integrada en negro puro
#define COLOR_SEPARATOR 0x18C3 // Línea de separación sutil grafito (24, 24, 24)

#define COLOR_CARD 0x1082      // Tarjeta grafito mate profundo (16, 16, 16)
#define COLOR_CARD_HL 0x1CE7   // Tarjeta activa / ranura destacada
#define COLOR_BORDER 0x2945    // Borde fino sutil (41, 42, 41)
#define COLOR_BORDER_HL 0xFFFF // Borde destacado blanco nítido

#define COLOR_TEXT_PRIMARY 0xFFFF   // Blanco nítido (alto contraste)
#define COLOR_TEXT_SECONDARY 0xAD55 // Plata / Gris neutro medio (173, 170, 173)
#define COLOR_TEXT_MUTED 0x632C     // Gris grafito secundario (99, 97, 99)

// ============================================================================
// PALETA DE COLORES VIBRANTES PARA ICONOS VECTORIALES
// ============================================================================
#define ICON_RED_TOMATO 0xFA08    // Rojo vibrante para Pomodoro
#define ICON_GREEN_STEM 0x2E65    // Verde hoja para tallo de Pomodoro
#define ICON_BLUE_CALENDAR 0x04BF // Azul cian para Agenda
#define ICON_GOLD_SUN 0xFDE0      // Amarillo oro brillante para Clima
#define ICON_SKY_CLOUD 0xCE7F     // Azul cielo para nube del clima
#define ICON_VIOLET_MOON 0x9B5F   // Violeta azulado para Reloj Nocturno
#define ICON_GOLD_STAR 0xFFE0     // Dorado para estrellas
#define ICON_MINT_ZEN 0x2FEA      // Verde menta para Respiración Relax
#define ICON_AMBER_BULB 0xFFE0    // Ámbar cálido para Lámpara RGB
#define ICON_EMERALD_BAT 0x07E0   // Verde esmeralda para Batería
#define ICON_CYAN_BAR 0x07FF      // Cian para estadísticas
#define ICON_ORANGE_BAR 0xFD20    // Naranja para estadísticas
#define ICON_SILVER_GEAR 0xC618   // Plata metálico para Ajustes
#define ICON_INFO_BLUE 0x241F     // Azul profundo para Info

UIManager::UIManager(TFT_eSPI &tft) : _tft(tft) {}

void UIManager::begin() {
  _tft.begin();
  _tft.setRotation(SCREEN_ROTATION);
  _tft.fillScreen(COLOR_BG);
}

// ============================================================================
// BARRA SUPERIOR UNIFICADA MINIMALISTA (STATUS BAR)
// ============================================================================
void UIManager::drawUnifiedStatusBar(const BatteryManager &battery,
                                     bool wifiConnected, bool audioMuted,
                                     bool autoBrightness, int streakCount,
                                     const char *appTitle) {
  _tft.fillRect(0, 0, SCREEN_WIDTH, 22, COLOR_STATUS_BAR);

  // 1. Izquierda: Título de Aplicación o Botón Atrás
  _tft.setTextDatum(TL_DATUM);
  if (appTitle != nullptr) {
    _tft.fillRoundRect(4, 2, 50, 18, 3, COLOR_CARD);
    _tft.drawRoundRect(4, 2, 50, 18, 3, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("< ATRAS", 29, 5, 1);

    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_STATUS_BAR);
    _tft.drawString(appTitle, 60, 4, 2);
  } else {
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_STATUS_BAR);
    _tft.drawString("STUDY DECK", 8, 4, 2);
  }

  // 2. Derecha: Indicadores de Hardware y Estado con espaciado óptimo
  int batPct = battery.getPercentage();
  bool isCharging = battery.isCharging();
  _tft.drawRoundRect(296, 6, 18, 10, 2, COLOR_TEXT_SECONDARY);
  _tft.fillRect(314, 8, 2, 6, COLOR_TEXT_SECONDARY);
  int fillW = map(constrain(batPct, 0, 100), 0, 100, 0, 14);
  if (fillW > 0) {
    uint16_t bCol =
        isCharging ? 0x07E0 : ((batPct > 20) ? COLOR_TEXT_PRIMARY : 0xF800);
    _tft.fillRect(298, 8, fillW, 6, bCol);
  }

  // Porcentaje Batería
  char batStr[8];
  snprintf(batStr, sizeof(batStr), "%d%%", batPct);
  _tft.setTextDatum(TR_DATUM);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_STATUS_BAR);
  _tft.drawString(batStr, 292, 5, 1);

  // Wi-Fi
  _tft.setTextDatum(TR_DATUM);
  _tft.setTextColor(wifiConnected ? 0x07E0 : COLOR_TEXT_MUTED,
                    COLOR_STATUS_BAR);
  _tft.drawString(wifiConnected ? "WIFI" : "--", 262, 5, 1);

  // Audio Mute
  int flagX = 234;
  if (audioMuted) {
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_STATUS_BAR);
    _tft.drawString("MUTE", flagX, 5, 1);
    flagX -= 28;
  }

  // Brillo automático
  if (autoBrightness) {
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_STATUS_BAR);
    _tft.drawString("AUTO", flagX, 5, 1);
  }

  _tft.drawFastHLine(0, 22, SCREEN_WIDTH, COLOR_SEPARATOR);
  _tft.setTextDatum(TL_DATUM);
}

// ============================================================================
// DIBUJO DE ICONOS VECTORIALES A TODO COLOR (16-BIT RGB565)
// ============================================================================
void UIManager::drawAppColorIcon(int cx, int cy, int appType) {
  switch (appType) {
  case 1: // Pomodoro (Tomate rojo brillante con tallo verde)
    _tft.fillCircle(cx, cy + 2, 11, ICON_RED_TOMATO);
    _tft.fillRect(cx - 1, cy - 11, 3, 5, ICON_GREEN_STEM);
    _tft.fillTriangle(cx, cy - 8, cx - 6, cy - 10, cx - 2, cy - 6,
                      ICON_GREEN_STEM);
    _tft.fillTriangle(cx, cy - 8, cx + 6, cy - 10, cx + 2, cy - 6,
                      ICON_GREEN_STEM);
    _tft.drawCircle(cx - 3, cy - 1, 3, 0xFCE8); // Brillo suave
    break;

  case 2: // Agenda (Calendario con borde, barra azul cian y cuadrícula)
    _tft.fillRoundRect(cx - 11, cy - 9, 22, 22, 3, 0xFFFF);
    _tft.fillRoundRect(cx - 11, cy - 9, 22, 7, 3, ICON_BLUE_CALENDAR);
    _tft.fillRect(cx - 7, cy - 11, 2, 4, 0x0000);
    _tft.fillRect(cx + 5, cy - 11, 2, 4, 0x0000);
    _tft.fillRect(cx - 7, cy + 1, 4, 3, 0x04BF);
    _tft.fillRect(cx - 1, cy + 1, 4, 3, 0x04BF);
    _tft.fillRect(cx + 5, cy + 1, 4, 3, 0x04BF);
    _tft.fillRect(cx - 7, cy + 6, 4, 3, 0x04BF);
    _tft.fillRect(cx - 1, cy + 6, 4, 3, 0xFA08); // Fecha destacada roja
    _tft.fillRect(cx + 5, cy + 6, 4, 3, 0x04BF);
    break;

  case 3: // Clima (Sol dorado radiante con nube suave)
    _tft.fillCircle(cx - 2, cy - 2, 8, ICON_GOLD_SUN);
    // Rayos solares
    _tft.drawFastVLine(cx - 2, cy - 13, 3, ICON_GOLD_SUN);
    _tft.drawFastHLine(cx - 13, cy - 2, 3, ICON_GOLD_SUN);
    _tft.drawLine(cx - 9, cy - 9, cx - 7, cy - 7, ICON_GOLD_SUN);
    // Nube suave
    _tft.fillCircle(cx + 4, cy + 5, 6, ICON_SKY_CLOUD);
    _tft.fillCircle(cx - 1, cy + 6, 5, ICON_SKY_CLOUD);
    _tft.fillRoundRect(cx - 4, cy + 6, 14, 6, 2, ICON_SKY_CLOUD);
    break;

  case 4: // Reloj Nocturno (Luna creciente violeta + estrellas doradas)
    _tft.fillCircle(cx - 1, cy, 10, ICON_VIOLET_MOON);
    _tft.fillCircle(cx + 4, cy - 3, 8, COLOR_CARD); // Recorte para media luna
    // Estrellas doradas
    _tft.drawPixel(cx + 7, cy + 4, ICON_GOLD_STAR);
    _tft.drawPixel(cx + 8, cy + 4, ICON_GOLD_STAR);
    _tft.drawPixel(cx + 7, cy + 5, ICON_GOLD_STAR);
    _tft.drawPixel(cx - 6, cy - 8, ICON_GOLD_STAR);
    break;

  case 5: // Relax Box (Caja zen / flor de loto menta)
    _tft.drawRoundRect(cx - 11, cy - 11, 22, 22, 4, ICON_MINT_ZEN);
    _tft.drawRoundRect(cx - 7, cy - 7, 14, 14, 3, 0x07E0);
    _tft.fillCircle(cx, cy, 3, 0xFFFF);
    break;

  case 6: // Lámpara RGB (Bombilla ámbar con rosca metálica)
    _tft.fillCircle(cx, cy - 3, 8, ICON_AMBER_BULB);
    _tft.fillRect(cx - 4, cy + 3, 8, 5, 0xC618);
    _tft.drawFastHLine(cx - 3, cy + 6, 6, 0x7BEF);
    _tft.drawLine(cx - 11, cy - 9, cx - 8, cy - 6, ICON_AMBER_BULB);
    _tft.drawLine(cx + 11, cy - 9, cx + 8, cy - 6, ICON_AMBER_BULB);
    break;

  case 7: // Batería 5Ah (Batería estilizada verde esmeralda con rayo)
    _tft.drawRoundRect(cx - 11, cy - 6, 20, 13, 2, 0xFFFF);
    _tft.fillRect(cx + 9, cy - 2, 3, 5, 0xC618);
    _tft.fillRect(cx - 9, cy - 4, 14, 9, ICON_EMERALD_BAT);
    _tft.drawLine(cx - 3, cy - 3, cx - 1, cy, 0xFFE0);
    _tft.drawLine(cx - 1, cy, cx - 4, cy + 3, 0xFFE0);
    break;

  case 8: // Stats MicroSD (Histograma de 3 barras multicolores)
    _tft.fillRect(cx - 9, cy + 1, 5, 9, ICON_CYAN_BAR);
    _tft.fillRect(cx - 2, cy - 7, 5, 17, ICON_EMERALD_BAT);
    _tft.fillRect(cx + 5, cy - 3, 5, 13, ICON_ORANGE_BAR);
    _tft.drawFastHLine(cx - 11, cy + 10, 23, 0xFFFF);
    break;

  case 9: // Ajustes (Engranaje plateado con centro azul)
    _tft.fillCircle(cx, cy, 10, ICON_SILVER_GEAR);
    _tft.drawFastVLine(cx, cy - 12, 24, ICON_SILVER_GEAR);
    _tft.drawFastHLine(cx - 12, cy, 24, ICON_SILVER_GEAR);
    _tft.fillCircle(cx, cy, 4, COLOR_CARD);
    _tft.drawCircle(cx, cy, 4, 0x04BF);
    break;

  case 10: // Info Deck (Círculo azul marino con la "i" en blanco)
    _tft.fillCircle(cx, cy, 10, ICON_INFO_BLUE);
    _tft.drawCircle(cx, cy, 10, 0x07FF);
    _tft.fillCircle(cx, cy - 4, 2, 0xFFFF);
    _tft.fillRect(cx - 1, cy - 1, 3, 6, 0xFFFF);
    break;

  default:
    _tft.drawRect(cx - 8, cy - 8, 16, 16, COLOR_BORDER);
    break;
  }
}

void UIManager::drawAppCard(int x, int y, int w, int h, uint16_t bgColor,
                            int appType, const char *title,
                            const char *subtitle) {
  _tft.fillRoundRect(x, y, w, h, 6, bgColor);
  _tft.drawRoundRect(x, y, w, h, 6, COLOR_BORDER);

  // Icono centrado
  drawAppColorIcon(x + w / 2, y + 26, appType);

  // Título
  _tft.setTextColor(COLOR_TEXT_PRIMARY, bgColor);
  _tft.drawCentreString(title, x + w / 2, y + 46, 2);

  // Subtítulo
  _tft.setTextColor(COLOR_TEXT_SECONDARY, bgColor);
  _tft.drawCentreString(subtitle, x + w / 2, y + 64, 1);
}

// ============================================================================
// 1. PANTALLA INICIAL PREDEFINIDA: ESTACIÓN DE CLIMA & RELOJ DESK (STANDBY)
// ============================================================================
void UIManager::drawWeatherView(const WeatherManager &weather,
                                CalendarManager &calendar,
                                const BatteryManager &battery,
                                bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  _tft.setTextDatum(TL_DATUM);
  drawUnifiedStatusBar(battery, wifiConnected, false, true, 0, nullptr);
  _tft.setTextDatum(TL_DATUM);

  // Reloj digital grande y fecha de hoy
  updateWeatherViewClock(weather, calendar);

  // 1. Tarjeta Clima de Ávila (Open-Meteo) (y: 86..150, h: 64)
  _tft.fillRoundRect(12, 86, 296, 64, 6, COLOR_CARD);
  _tft.drawRoundRect(12, 86, 296, 64, 6, COLOR_BORDER);

  // Icono de sol/clima en grande
  drawAppColorIcon(38, 118, 3);

  // Temperatura grande
  char tBuf[16];
  if (weather.hasData()) {
    snprintf(tBuf, sizeof(tBuf), "%.1f C", weather.getTemperature());
  } else {
    snprintf(tBuf, sizeof(tBuf), "--.- C");
  }
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(tBuf, 68, 96, 4);

  // Humedad y Ciudad
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  char hBuf[32];
  if (weather.hasData()) {
    snprintf(hBuf, sizeof(hBuf), "%d%% Humedad  |  %s", weather.getHumidity(),
             weather.getCityName().c_str());
  } else {
    snprintf(hBuf, sizeof(hBuf), "Avila  |  Conectando WiFi");
  }
  _tft.drawString(hBuf, 68, 126, 2);

  // 2. Franja de Estado Académico UNED (y: 156..196, h: 40)
  _tft.fillRoundRect(12, 156, 296, 40, 5, COLOR_CARD);
  _tft.drawRoundRect(12, 156, 296, 40, 5, COLOR_BORDER);

  CalendarEvent todaySession;
  bool hasSession = calendar.getTodayMainSession(todaySession);
  bool ongoing = calendar.isStudySessionOngoing();

  if (ongoing) {
    _tft.fillRoundRect(14, 158, 292, 36, 4, 0x051D);
    _tft.setTextColor(0xFFFF, 0x051D);
    _tft.drawCentreString("SESION DE ESTUDIO EN CURSO", 160, 162, 2);
    _tft.drawCentreString(todaySession.timeRange + " | " +
                              todaySession.title.substring(0, 24),
                          160, 178, 1);
  } else if (hasSession) {
    _tft.setTextColor(0xFDE0, COLOR_CARD);
    _tft.drawString("Hoy:", 20, 162, 2);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(todaySession.timeRange, 58, 162, 2);

    String sum = todaySession.title;
    if (sum.length() > 28)
      sum = sum.substring(0, 26) + "..";
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawString(sum, 20, 178, 1);
  } else {
    String examTitle;
    String countdown = calendar.getNextDeadlineCountdown(examTitle);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawString("Proximo examen:", 20, 162, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(examTitle.substring(0, 24), 20, 174, 2);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawRightString(countdown, 300, 174, 2);
  }

  // 3. Botón de acceso al menú de aplicaciones (y: 202..234, h: 32)
  _tft.fillRoundRect(12, 202, 296, 32, 5, COLOR_CARD_HL);
  _tft.drawRoundRect(12, 202, 296, 32, 5, COLOR_BORDER_HL);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString("TOCAR PANTALLA PARA ABRIR MENU", 160, 210, 2);
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
    const char *meses[12] = {"Enero",      "Febrero", "Marzo",     "Abril",
                             "Mayo",       "Junio",   "Julio",     "Agosto",
                             "Septiembre", "Octubre", "Noviembre", "Diciembre"};
    snprintf(dateBuf, sizeof(dateBuf), "%s, %d de %s %d", dias[t->tm_wday],
             t->tm_mday, meses[t->tm_mon], t->tm_year + 1900);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "--:--:--");
    snprintf(dateBuf, sizeof(dateBuf), "Sincronizando hora con red...");
  }

  // Reloj digital centrado grande
  _tft.fillRect(12, 26, 296, 56, COLOR_BG);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
  _tft.drawCentreString(timeBuf, 160, 30, 4);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
  _tft.drawCentreString(dateBuf, 160, 62, 2);
  _tft.setTextDatum(TL_DATUM);
}

bool UIManager::handleWeatherTouch(int x, int y, bool *outOpenMenu) {
  *outOpenMenu = true;
  return true; // Cualquier toque en pantalla abre el menú de aplicaciones
}

// ============================================================================
// 2. MENÚ LANZADOR DE APLICACIONES (APP LAUNCHER)
// ============================================================================
void UIManager::drawHomeScreen(const PomodoroEngine &engine,
                               const BatteryManager &battery,
                               const WeatherManager &weather,
                               bool wifiConnected, bool audioMuted,
                               bool autoBrightness, int homeTab) {
  _tft.fillScreen(COLOR_BG);

  // Barra superior con botón volver a Clima (Standby)
  drawUnifiedStatusBar(battery, wifiConnected, audioMuted, autoBrightness,
                       engine.getCompletedSessionsToday(), "MENU PRINCIPAL");
  _tft.setTextDatum(TL_DATUM);

  // Selector de Pestañas Superiores (x: 10 y 164, y: 25..46)
  bool tab0 = (homeTab == 0);
  _tft.fillRoundRect(10, 25, 146, 22, 4,
                     tab0 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(10, 25, 146, 22, 4,
                     tab0 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(tab0 ? COLOR_BG : COLOR_TEXT_SECONDARY,
                    tab0 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("1. ESTUDIO & DESK", 83, 29, 2);

  bool tab1 = (homeTab == 1);
  _tft.fillRoundRect(164, 25, 146, 22, 4,
                     tab1 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(164, 25, 146, 22, 4,
                     tab1 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(tab1 ? COLOR_BG : COLOR_TEXT_SECONDARY,
                    tab1 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("2. SISTEMA & UTILS", 237, 29, 2);

  // Subtítulos dinámicos de estado
  char pomoSub[16];
  if (engine.isRunning()) {
    uint32_t s = engine.getRemainingSeconds();
    snprintf(pomoSub, sizeof(pomoSub), "%02d:%02d", s / 60, s % 60);
  } else {
    snprintf(pomoSub, sizeof(pomoSub), "%d min", engine.getFocusMinutes());
  }

  char batSub[20];
  snprintf(batSub, sizeof(batSub), "%d%% (~%dh)", battery.getPercentage(),
           (int)battery.getEstimatedHoursLeft());

  char wSub[20];
  if (weather.hasData()) {
    snprintf(wSub, sizeof(wSub), "%.1f C Avila", weather.getTemperature());
  } else {
    snprintf(wSub, sizeof(wSub), "Avila");
  }

  if (homeTab == 0) {
    // --- PESTAÑA 1: ESTUDIO & DESK (6 APLICACIONES) ---
    // Fila 1 (y: 52, h: 84)
    drawAppCard(10, 52, 94, 84, COLOR_CARD, 1, "Pomodoro", pomoSub);
    drawAppCard(113, 52, 94, 84, COLOR_CARD, 2, "Agenda", "UNED Avila");
    drawAppCard(216, 52, 94, 84, COLOR_CARD, 3, "Clima", wSub);

    // Fila 2 (y: 142, h: 84)
    drawAppCard(10, 142, 94, 84, COLOR_CARD, 4, "Reloj Noche", "Tenue");
    drawAppCard(113, 142, 94, 84, COLOR_CARD, 5, "Relax Box", "4-4-4-4");
    drawAppCard(216, 142, 94, 84, COLOR_CARD, 6, "Lampara", "Luz RGB");
  } else {
    // --- PESTAÑA 2: SISTEMA & UTILIDADES (4 APLICACIONES GRANDES Y ESPACIOSAS)
    // --- Fila 1 (y: 52, h: 84)
    drawAppCard(10, 52, 146, 84, COLOR_CARD, 7, "Bateria 5Ah", batSub);
    drawAppCard(164, 52, 146, 84, COLOR_CARD, 8, "Stats SD", "Historial");

    // Fila 2 (y: 142, h: 84)
    drawAppCard(10, 142, 146, 84, COLOR_CARD, 9, "Ajustes", "WiFi y Brillo");
    drawAppCard(164, 142, 146, 84, COLOR_CARD, 10, "Info Deck", "v1.3.0 UNED");
  }
}

int UIManager::handleHomeTouch(int x, int y, int &homeTab) {
  // 0. Botón volver a Standby/Clima (esquina superior izquierda en la barra de
  // estado)
  if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
    return -20; // Volver a Standby (VIEW_WEATHER)
  }

  // 1. Selector de Pestañas Superiores (y: 22..48)
  if (y >= 22 && y <= 48) {
    if (x >= 8 && x <= 158) {
      homeTab = 0;
      return -10;
    }
    if (x >= 160 && x <= 312) {
      homeTab = 1;
      return -10;
    }
  }

  // 2. Fila 1 (y: 50..138)
  if (y >= 50 && y <= 138) {
    if (homeTab == 0) {
      if (x >= 8 && x <= 106)
        return 1; // Pomodoro
      if (x >= 111 && x <= 209)
        return 4; // Agenda UNED
      if (x >= 214 && x <= 312)
        return 8; // Clima
    } else {
      if (x >= 8 && x <= 158)
        return 12; // Batería
      if (x >= 160 && x <= 312)
        return 7; // Stats SD
    }
  }

  // 3. Fila 2 (y: 140..228)
  if (y >= 140 && y <= 228) {
    if (homeTab == 0) {
      if (x >= 8 && x <= 106)
        return 9; // Reloj Noche
      if (x >= 111 && x <= 209)
        return 10; // Relax Box
      if (x >= 214 && x <= 312)
        return 11; // Lámpara RGB
    } else {
      if (x >= 8 && x <= 158)
        return 5; // Ajustes
      if (x >= 160 && x <= 312)
        return 6; // Info
    }
  }

  return 0;
}

// ============================================================================
// 3. APLICACIÓN POMODORO VINCULADA AL CALENDARIO DE ESTUDIO
// ============================================================================
void UIManager::drawPomodoroView(const PomodoroEngine &engine,
                                 const CalendarManager &calendar,
                                 const BatteryManager &battery,
                                 bool wifiConnected, bool audioMuted,
                                 bool autoBrightness) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, wifiConnected, audioMuted, autoBrightness,
                       engine.getCompletedSessionsToday(), "POMODORO");

  // Franja Superior: Materia del Calendario Académico de Hoy (y: 26..58, h: 32)
  CalendarEvent session;
  bool hasSession = calendar.getTodayMainSession(session);
  _tft.fillRoundRect(12, 26, 296, 32, 4, COLOR_CARD);
  _tft.drawRoundRect(12, 26, 296, 32, 4, COLOR_BORDER);

  if (hasSession) {
    String title = session.title;
    if (title.length() > 24)
      title = title.substring(0, 22) + "..";
    _tft.setTextColor(0xFDE0, COLOR_CARD);
    _tft.drawString(session.timeRange, 18, 30, 2);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(title, 130, 30, 2);
  } else {
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawCentreString(
        "Modo Enfoque Libre (Sin horario de estudio agendado)", 160, 34, 1);
  }

  // Botones de Modo (25 / 50 / 90 min)
  drawModeButtons(engine);

  // Temporizador y barra de progreso
  drawCountdown(engine);
  drawProgressBar(engine);

  // Botones de Acción: Iniciar / Pausa, Reset, Saltar
  drawActionButtons(engine);
}

void UIManager::drawModeButtons(const PomodoroEngine &engine) {
  uint16_t currentFocus = engine.getFocusMinutes();
  int btnW = 92;
  int btnH = 22;
  int startY = 64;

  int x25 = 12;
  bool sel25 = (currentFocus == 25);
  _tft.fillRoundRect(x25, startY, btnW, btnH, 3,
                     sel25 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(x25, startY, btnW, btnH, 3,
                     sel25 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(sel25 ? COLOR_BG : COLOR_TEXT_SECONDARY,
                    sel25 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("25 MIN", x25 + btnW / 2, startY + 4, 2);

  int x50 = 114;
  bool sel50 = (currentFocus == 50);
  _tft.fillRoundRect(x50, startY, btnW, btnH, 3,
                     sel50 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(x50, startY, btnW, btnH, 3,
                     sel50 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(sel50 ? COLOR_BG : COLOR_TEXT_SECONDARY,
                    sel50 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("50 MIN", x50 + btnW / 2, startY + 4, 2);

  int x90 = 216;
  bool sel90 = (currentFocus == 90);
  _tft.fillRoundRect(x90, startY, btnW, btnH, 3,
                     sel90 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(x90, startY, btnW, btnH, 3,
                     sel90 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(sel90 ? COLOR_BG : COLOR_TEXT_SECONDARY,
                    sel90 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("90 MIN", x90 + btnW / 2, startY + 4, 2);
}

void UIManager::drawCountdown(const PomodoroEngine &engine) {
  uint32_t s = engine.getRemainingSeconds();
  uint16_t m = s / 60;
  uint8_t sec = s % 60;

  char timeBuf[12];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", m, sec);

  _tft.fillRect(30, 94, 260, 46, COLOR_BG);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
  _tft.drawCentreString(timeBuf, 160, 96, 7);

  // Estado actual: ENFOQUE / DESCANSO
  const char *stateStr = "EN REPOSO";
  if (engine.isFocus()) {
    stateStr = engine.isPaused() ? "ENFOQUE (PAUSADO)" : "ENFOQUE TOTAL";
  } else if (engine.isBreak()) {
    stateStr = engine.isPaused() ? "DESCANSO (PAUSADO)" : "DESCANSO CORTO";
  }
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
  _tft.drawCentreString(stateStr, 160, 144, 2);
}

void UIManager::drawProgressBar(const PomodoroEngine &engine) {
  int barX = 20;
  int barY = 164;
  int barW = 280;
  int barH = 6;

  _tft.fillRoundRect(barX, barY, barW, barH, 2, COLOR_CARD);
  _tft.drawRoundRect(barX, barY, barW, barH, 2, COLOR_BORDER);

  float prog = engine.getProgress();
  int fillW = (int)(prog * (barW - 2));
  if (fillW > 0) {
    _tft.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 2,
                       COLOR_TEXT_PRIMARY);
  }
}

void UIManager::drawActionButtons(const PomodoroEngine &engine) {
  int btnY = 184;
  int btnH = 44;

  // Botón Principal: INICIAR / PAUSAR
  bool running = engine.isRunning();
  _tft.fillRoundRect(20, btnY, 180, btnH, 5,
                     running ? COLOR_CARD : COLOR_TEXT_PRIMARY);
  _tft.drawRoundRect(20, btnY, 180, btnH, 5, COLOR_TEXT_PRIMARY);
  _tft.setTextColor(running ? COLOR_TEXT_PRIMARY : COLOR_BG,
                    running ? COLOR_CARD : COLOR_TEXT_PRIMARY);
  _tft.drawCentreString(running ? "PAUSAR" : "INICIAR ESTUDIO", 110, btnY + 12,
                        4);

  // Botón Secundario: SALTAR
  _tft.fillRoundRect(210, btnY, 90, btnH, 5, COLOR_CARD);
  _tft.drawRoundRect(210, btnY, 90, btnH, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString("SALTAR", 255, btnY + 14, 2);
}

void UIManager::updatePomodoroTimer(const PomodoroEngine &engine,
                                    const CalendarManager & /*calendar*/,
                                    const BatteryManager & /*battery*/,
                                    bool /*wifiConnected*/) {
  drawCountdown(engine);
  drawProgressBar(engine);
}

UIButtonAction UIManager::handlePomodoroTouch(int x, int y) {
  // 1. Botón Volver Atrás (barra superior)
  if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
    return UI_ACTION_BACK_HOME;
  }

  // 2. Selectores de Modo (y: 60..88)
  if (y >= 60 && y <= 88) {
    if (x >= 12 && x <= 104)
      return UI_ACTION_SELECT_25;
    if (x >= 114 && x <= 206)
      return UI_ACTION_SELECT_50;
    if (x >= 216 && x <= 308)
      return UI_ACTION_SELECT_90;
  }

  // 3. Botón Principal Iniciar/Pausar (x: 20..200, y: 180..232)
  if (x >= 20 && x <= 200 && y >= 180 && y <= 232) {
    return UI_ACTION_PLAY_PAUSE;
  }

  // 4. Botón Saltar (x: 210..300, y: 180..232)
  if (x >= 210 && x <= 300 && y >= 180 && y <= 232) {
    return UI_ACTION_SKIP;
  }

  return UI_ACTION_NONE;
}

// ============================================================================
// 4. APLICACIÓN AGENDA ERGONÓMICA REDISEÑADA PARA PANTALLA DE 2.4"
// ============================================================================
void UIManager::drawAgendaView(CalendarManager &calendar,
                               const WiFiConfig &wifi,
                               const BatteryManager &battery,
                               const WeatherManager &weather) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, wifi.isConnected(), false, true, 0,
                       "CALENDARIO UNED");
  _tft.setTextDatum(TL_DATUM);

  // 1. Selector de Filtros: [ DÍA ] [ SEMANA ] [ MES ] (y: 26..46)
  CalendarViewMode mode = calendar.getViewMode();

  bool selDay = (mode == CAL_VIEW_DAY);
  _tft.fillRoundRect(12, 26, 92, 20, 3,
                     selDay ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(12, 26, 92, 20, 3,
                     selDay ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(selDay ? COLOR_BG : COLOR_TEXT_SECONDARY,
                    selDay ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("DIA", 58, 29, 2);

  bool selWeek = (mode == CAL_VIEW_WEEK);
  _tft.fillRoundRect(114, 26, 92, 20, 3,
                     selWeek ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(114, 26, 92, 20, 3,
                     selWeek ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(selWeek ? COLOR_BG : COLOR_TEXT_SECONDARY,
                    selWeek ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("SEMANA", 160, 29, 2);

  bool selMonth = (mode == CAL_VIEW_MONTH);
  _tft.fillRoundRect(216, 26, 92, 20, 3,
                     selMonth ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(216, 26, 92, 20, 3,
                     selMonth ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(selMonth ? COLOR_BG : COLOR_TEXT_SECONDARY,
                    selMonth ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("MES", 262, 29, 2);

  // ========================================================================
  // MODO 1: VISTA DE DÍA ERGONÓMICA
  // ========================================================================
  if (mode == CAL_VIEW_DAY) {
    updateAgendaClock(calendar, weather);

    // Tarjeta 1: Sesión Principal de Estudio de Hoy (y: 70..132, h: 62)
    CalendarEvent todayEvs[4];
    int evCount = calendar.getTodayEvents(todayEvs, 4);

    _tft.fillRoundRect(12, 70, 296, 62, 5, COLOR_CARD);
    _tft.drawRoundRect(12, 70, 296, 62, 5, COLOR_BORDER);

    if (evCount > 0) {
      CalendarEvent ev = todayEvs[0];
      _tft.setTextColor(0xFDE0, COLOR_CARD);
      _tft.drawString(ev.timeRange, 20, 75, 2);

      if (ev.isOngoing) {
        _tft.fillRoundRect(238, 74, 62, 16, 2, 0x07E0);
        _tft.setTextColor(0x0000, 0x07E0);
        _tft.drawCentreString("EN CURSO", 269, 76, 1);
      }

      String title = ev.title;
      if (title.length() > 26)
        title = title.substring(0, 24) + "..";
      _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
      _tft.drawString(title, 20, 94, 2);

      _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
      _tft.drawString("Sesion de estudio oficial UNED", 20, 114, 1);
    } else {
      _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
      _tft.drawCentreString("Sin sesion agendada para hoy", 160, 86, 2);
      _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
      _tft.drawCentreString("Dia libre de estudio o repaso voluntario", 160,
                            108, 1);
    }

    // Tarjeta 2: Tutoría Semanal UNED (si toca hoy o tutoría más cercana) (y:
    // 138..166, h: 28)
    _tft.fillRoundRect(12, 138, 296, 28, 4, COLOR_CARD);
    _tft.drawRoundRect(12, 138, 296, 28, 4, COLOR_BORDER);

    bool foundTutoria = false;
    for (int i = 0; i < evCount; i++) {
      if (todayEvs[i].title.indexOf("Tutoria") != -1) {
        _tft.setTextColor(0x07FF, COLOR_CARD);
        _tft.drawString("TUTORIA HOY: " + todayEvs[i].timeRange, 20, 144, 2);
        foundTutoria = true;
        break;
      }
    }
    if (!foundTutoria) {
      _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
      _tft.drawString("Tutorias: Mar 20:00 (Sist. Dig) | Mie 18:15 (Prog)", 20,
                      147, 1);
    }

    // Tarjeta 3: Próximo Examen Oficial / Entrega con Cuenta Atrás (y:
    // 172..200, h: 28)
    String examTitle;
    String examCountdown = calendar.getNextDeadlineCountdown(examTitle);

    _tft.fillRoundRect(12, 172, 296, 28, 4, COLOR_CARD);
    _tft.drawRoundRect(12, 172, 296, 28, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawString("Examen:", 18, 178, 1);

    if (examTitle.length() > 20)
      examTitle = examTitle.substring(0, 18) + "..";
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(examTitle, 66, 176, 2);

    _tft.setTextColor(0xFDE0, COLOR_CARD);
    _tft.drawRightString(examCountdown, 300, 176, 2);

    // Botonera Inferior (y: 206..236, h: 30)
    _tft.fillRoundRect(12, 206, 142, 30, 4, COLOR_CARD_HL);
    _tft.drawRoundRect(12, 206, 142, 30, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
    _tft.drawCentreString("< VOLVER AL MENU", 83, 213, 2);

    _tft.fillRoundRect(166, 206, 142, 30, 4, COLOR_CARD);
    _tft.drawRoundRect(166, 206, 142, 30, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString("SINC GCAL", 237, 213, 2);
  }
  // ========================================================================
  // MODO 2: VISTA DE SEMANA ERGONÓMICA
  // ========================================================================
  else if (mode == CAL_VIEW_WEEK) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    int curY = (t->tm_year > 120) ? (t->tm_year + 1900) : 2026;
    int curM = (t->tm_year > 120) ? (t->tm_mon + 1) : 10;
    int curD = (t->tm_year > 120) ? t->tm_mday : 31;
    int todayWDay =
        (t->tm_year > 120) ? ((t->tm_wday + 6) % 7) : 5; // 0=Lun, 6=Dom

    time_t monTime = (t->tm_year > 120) ? (now - (todayWDay * 86400)) : now;
    struct tm *monTm = localtime(&monTime);
    int monDay = monTm->tm_mday;
    int monMonth = monTm->tm_mon + 1;

    time_t sunTime = monTime + (6 * 86400);
    struct tm *sunTm = localtime(&sunTime);
    int sunDay = sunTm->tm_mday;
    int sunMonth = sunTm->tm_mon + 1;

    const char *mesesShort[12] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                                  "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
    char wHead[48];
    snprintf(wHead, sizeof(wHead), "Semana: %02d %s - %02d %s", monDay,
             mesesShort[monMonth - 1], sunDay, mesesShort[sunMonth - 1]);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
    _tft.drawString(wHead, 16, 50, 2);

    const char *dayNames[7] = {"LUN", "MAR", "MIE", "JUE", "VIE", "SAB", "DOM"};
    for (int i = 0; i < 7; i++) {
      time_t dayTime = monTime + (i * 86400);
      struct tm *dTm = localtime(&dayTime);
      int dNum = dTm->tm_mday;
      int dMon = dTm->tm_mon + 1;
      int dYear = dTm->tm_year + 1900;
      bool isToday = (dNum == curD && dMon == curM && dYear == curY);

      int rowY = 66 + i * 20;

      _tft.fillRoundRect(12, rowY, 52, 18, 2,
                         isToday ? COLOR_TEXT_PRIMARY : COLOR_CARD);
      _tft.drawRoundRect(12, rowY, 52, 18, 2,
                         isToday ? COLOR_TEXT_PRIMARY : COLOR_BORDER);

      char dStr[12];
      snprintf(dStr, sizeof(dStr), "%s %02d", dayNames[i], dNum);
      _tft.setTextColor(isToday ? COLOR_BG : COLOR_TEXT_SECONDARY,
                        isToday ? COLOR_TEXT_PRIMARY : COLOR_CARD);
      _tft.drawCentreString(dStr, 38, rowY + 2, 1);

      _tft.fillRoundRect(68, rowY, 240, 18, 2, COLOR_CARD);
      _tft.drawRoundRect(68, rowY, 240, 18, 2,
                         isToday ? COLOR_TEXT_PRIMARY : COLOR_BORDER);

      String sum = calendar.getEventSummaryForDate(dNum, dMon, dYear);
      if (sum.length() > 0) {
        if (sum.length() > 30)
          sum = sum.substring(0, 28) + "..";
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawString(sum, 74, rowY + 2, 1);
      } else {
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawString(isToday ? "Hoy sin sesion agendada" : "-- Libre --", 74,
                        rowY + 2, 1);
      }
    }

    // Botón inferior Volver (y: 210..236)
    _tft.fillRoundRect(12, 210, 296, 26, 4, COLOR_CARD);
    _tft.drawRoundRect(12, 210, 296, 26, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString("< VOLVER AL MENU", 160, 214, 2);
  }
  // ========================================================================
  // MODO 3: VISTA DE MES ENTERO
  // ========================================================================
  else if (mode == CAL_VIEW_MONTH) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    int curY = (t->tm_year > 120) ? (t->tm_year + 1900) : 2026;
    int curM = (t->tm_year > 120) ? (t->tm_mon + 1) : 10;
    int curD = (t->tm_year > 120) ? t->tm_mday : 31;

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

    _tft.fillRoundRect(12, 48, 26, 16, 3, COLOR_CARD);
    _tft.drawRoundRect(12, 48, 26, 16, 3, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("<", 25, 49, 2);

    char titleBuf[32];
    snprintf(titleBuf, sizeof(titleBuf), "%s %d", mesesFull[dispM - 1], dispY);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
    _tft.drawCentreString(titleBuf, 160, 48, 2);

    _tft.fillRoundRect(282, 48, 26, 16, 3, COLOR_CARD);
    _tft.drawRoundRect(282, 48, 26, 16, 3, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString(">", 295, 49, 2);

    const char *wNames[7] = {"L", "M", "X", "J", "V", "S", "D"};
    for (int col = 0; col < 7; col++) {
      int cx = 13 + col * 42 + 21;
      _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
      _tft.drawCentreString(wNames[col], cx, 66, 1);
    }
    _tft.drawFastHLine(12, 76, 296, COLOR_BORDER);

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
      int cellX = 13 + col * 42;
      int cellY = 78 + row * 16;

      bool isToday = (dispY == curY && dispM == curM && day == curD);
      bool hasEvent = calendar.hasEventOnDate(day, dispM, dispY);
      bool hasExam = calendar.hasDeadlineOnDate(day, dispM, dispY);

      if (isToday) {
        _tft.fillRoundRect(cellX + 7, cellY - 1, 28, 16, 4, COLOR_TEXT_PRIMARY);
        _tft.drawRoundRect(cellX + 6, cellY - 2, 30, 18, 4, 0x07FF);
        _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
      } else {
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
      }

      _tft.drawCentreString(String(day), cellX + 21, cellY + 1, 1);

      if (hasEvent || hasExam) {
        uint16_t dotCol = hasExam ? 0xFA08 : (isToday ? COLOR_BG : 0x07FF);
        _tft.drawFastHLine(cellX + 16, cellY + 13, 10, dotCol);
      }

      col++;
      if (col > 6) {
        col = 0;
        row++;
      }
    }

    // Panel inferior con resumen de eventos del mes / botón volver
    _tft.fillRoundRect(12, 196, 296, 36, 4, COLOR_CARD);
    _tft.drawRoundRect(12, 196, 296, 36, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("< VOLVER AL MENU", 160, 206, 2);
  }
}

void UIManager::updateAgendaClock(CalendarManager &calendar,
                                  const WeatherManager & /*weather*/) {
  if (calendar.getViewMode() != CAL_VIEW_DAY)
    return;

  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  char headerStr[64];
  if (t->tm_year > 120) {
    const char *dias[7] = {"Dom", "Lun", "Mar", "Mie", "Jue", "Vie", "Sab"};
    const char *meses[12] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                             "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
    snprintf(headerStr, sizeof(headerStr), "%02d:%02d:%02d  |  %s, %d %s",
             t->tm_hour, t->tm_min, t->tm_sec, dias[t->tm_wday], t->tm_mday,
             meses[t->tm_mon]);
  } else {
    snprintf(headerStr, sizeof(headerStr), "--:--:--  |  Hora pendiente");
  }

  _tft.fillRect(12, 50, 195, 18, COLOR_BG);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
  _tft.drawString(headerStr, 14, 52, 2);
}

bool UIManager::handleAgendaTouch(int x, int y, CalendarManager &calendar,
                                  bool *outAddExam, bool *outSyncGCal,
                                  bool *outBack) {
  *outAddExam = false;
  *outSyncGCal = false;
  *outBack = false;

  // Botón Volver de la barra de estado
  if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
    *outBack = true;
    return true;
  }

  // Pestañas de filtros superiores (y: 26..48)
  if (y >= 26 && y <= 48) {
    if (x >= 12 && x <= 104) {
      calendar.setViewMode(CAL_VIEW_DAY);
      return true;
    } else if (x >= 114 && x <= 206) {
      calendar.setViewMode(CAL_VIEW_WEEK);
      return true;
    } else if (x >= 216 && x <= 308) {
      calendar.setViewMode(CAL_VIEW_MONTH);
      return true;
    }
  }

  // En vista MES: botones de navegación de mes [<] y [>]
  if (calendar.getViewMode() == CAL_VIEW_MONTH) {
    if (y >= 44 && y <= 68) {
      if (x >= 10 && x <= 45) {
        calendar.changeMonth(-1);
        return true;
      } else if (x >= 275 && x <= 310) {
        calendar.changeMonth(1);
        return true;
      }
    }
    if (y >= 190 && y <= 236) {
      *outBack = true;
      return true;
    }
  }

  // En vista SEMANA: botón volver
  if (calendar.getViewMode() == CAL_VIEW_WEEK) {
    if (y >= 204 && y <= 238) {
      *outBack = true;
      return true;
    }
  }

  // En vista DÍA: botones inferiores
  if (calendar.getViewMode() == CAL_VIEW_DAY) {
    if (y >= 200 && y <= 238) {
      if (x >= 12 && x <= 158) {
        *outBack = true;
        return true;
      } else if (x >= 162 && x <= 312) {
        *outSyncGCal = true;
        return true;
      }
    }
  }

  return false;
}

// ============================================================================
// 5. APLICACIÓN DEDICADA: RELOJ NOCTURNO (NIGHT DESK CLOCK)
// ============================================================================
void UIManager::drawNightClockView(const BatteryManager &battery,
                                   const WeatherManager &weather) {
  _tft.fillScreen(COLOR_BG);

  int batPct = battery.getPercentage();
  char bStr[16];
  snprintf(bStr, sizeof(bStr), "%d%%", batPct);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  _tft.drawRightString(bStr, 310, 8, 1);

  updateNightClock(battery, weather);

  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  _tft.drawCentreString("Modo Tenue Activo  |  Toca para salir", 160, 218, 1);
}

void UIManager::updateNightClock(const BatteryManager & /*battery*/,
                                 const WeatherManager &weather) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  char timeBuf[16];
  char dateBuf[48];

  if (t->tm_year > 120) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t->tm_hour, t->tm_min);
    const char *dias[7] = {"Domingo", "Lunes",   "Martes", "Miercoles",
                           "Jueves",  "Viernes", "Sabado"};
    const char *meses[12] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun",
                             "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
    snprintf(dateBuf, sizeof(dateBuf), "%s, %d %s", dias[t->tm_wday],
             t->tm_mday, meses[t->tm_mon]);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "--:--");
    snprintf(dateBuf, sizeof(dateBuf), "Buscando hora...");
  }

  _tft.fillRect(20, 60, 280, 80, COLOR_BG);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
  _tft.drawCentreString(timeBuf, 160, 64, 7);

  _tft.fillRect(20, 148, 280, 30, COLOR_BG);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
  _tft.drawCentreString(dateBuf, 160, 150, 4);

  if (weather.hasData()) {
    char wBuf[32];
    snprintf(wBuf, sizeof(wBuf), "%.1f C  |  Avila", weather.getTemperature());
    _tft.fillRect(20, 184, 280, 20, COLOR_BG);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
    _tft.drawCentreString(wBuf, 160, 184, 2);
  }
}

bool UIManager::handleNightClockTouch(int /*x*/, int /*y*/) {
  return true; // Cualquier toque sale de reloj nocturno
}

// ============================================================================
// 6. APLICACIÓN DEDICADA: RESPIRACIÓN CONSCIENTE (BOX BREATHING)
// ============================================================================
void UIManager::drawBreathingView(int phase, int secondsLeft, int cycle,
                                  const BatteryManager &battery) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, false, false, false, 0, "RELAX BOX");

  const char *phaseNames[4] = {"INHALA", "MANTEN", "EXHALA", "PAUSA"};
  const uint16_t phaseColors[4] = {
      0x2FEA, // Inhala: Verde menta suave
      0x07FF, // Mantén: Cian enfoque
      0x435F, // Exhala: Azul relajante
      0x7BEF  // Pausa: Grafito suave
  };
  uint16_t currentPhaseColor = phaseColors[phase % 4];

  _tft.setTextColor(currentPhaseColor, COLOR_BG);
  _tft.drawCentreString(phaseNames[phase % 4], 160, 34, 4);

  int cx = 160, cy = 115, sz = 44;
  _tft.drawRoundRect(cx - sz, cy - sz, sz * 2, sz * 2, 8, COLOR_BORDER);
  _tft.fillCircle(cx, cy, 26, COLOR_CARD);
  
  // Anillo de 3 px de grosor con el color dinámico de la fase
  _tft.drawCircle(cx, cy, 26, currentPhaseColor);
  _tft.drawCircle(cx, cy, 27, currentPhaseColor);
  _tft.drawCircle(cx, cy, 28, currentPhaseColor);

  char secBuf[8];
  snprintf(secBuf, sizeof(secBuf), "%d", secondsLeft);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(secBuf, cx, cy - 10, 4);

  char cycBuf[32];
  snprintf(cycBuf, sizeof(cycBuf), "Ciclo %d / 4  |  Tecnica 4-4-4-4", cycle);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
  _tft.drawCentreString(cycBuf, 160, 172, 2);

  _tft.fillRoundRect(20, 196, 280, 36, 5, COLOR_CARD);
  _tft.drawRoundRect(20, 196, 280, 36, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("FINALIZAR RESPIRACION", 160, 206, 2);
}

bool UIManager::handleBreathingTouch(int x, int y) {
  if (x >= 4 && x <= 64 && y >= 0 && y <= 24)
    return true;
  if (y >= 190 && y <= 236)
    return true;
  return false;
}

// ============================================================================
// 7. APLICACIÓN DEDICADA: LÁMPARA RGB DE ESCRITORIO
// ============================================================================
void UIManager::drawRgbLampView(int currentMode,
                                const BatteryManager &battery) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, false, false, false, 0, "LAMPARA RGB");

  const char *modes[5] = {"0. APAGADO", "1. BLANCO CALIDO", "2. CIAN ENFOQUE",
                          "3. AZUL NOCTURNO", "4. EFECTO VELA"};
  const uint16_t swatchColors[5] = {
      0x2945, // 0. APAGADO (grafito oscuro)
      0xFFF0, // 1. BLANCO CALIDO (marfil/ámbar suave)
      0x07FF, // 2. CIAN ENFOQUE (cian brillante)
      0x223F, // 3. AZUL NOCTURNO (azul profundo zafiro)
      0xFD20  // 4. EFECTO VELA (ámbar fuego)
  };

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
  _tft.drawCentreString("Selecciona el tono de iluminacion ambiental:", 160, 28,
                        2);

  for (int i = 0; i < 5; i++) {
    int y = 48 + i * 35;
    bool active = (currentMode == i);

    _tft.fillRoundRect(16, y, 288, 30, 4, active ? COLOR_CARD_HL : COLOR_CARD);
    _tft.drawRoundRect(16, y, 288, 30, 4,
                       active ? COLOR_TEXT_PRIMARY : COLOR_BORDER);

    // Muestra de color (swatch) con anillo exterior si está activo
    _tft.fillCircle(36, y + 15, 7, swatchColors[i]);
    if (active) {
      _tft.drawCircle(36, y + 15, 9, COLOR_TEXT_PRIMARY);
      _tft.drawCircle(36, y + 15, 10, COLOR_TEXT_PRIMARY);
    } else {
      _tft.drawCircle(36, y + 15, 8, COLOR_BORDER);
    }

    _tft.setTextColor(active ? COLOR_TEXT_PRIMARY : COLOR_TEXT_SECONDARY,
                      active ? COLOR_CARD_HL : COLOR_CARD);
    _tft.drawString(modes[i], 56, y + 7, 2);
  }
}

int UIManager::handleRgbLampTouch(int x, int y, bool *outBack) {
  *outBack = false;
  if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
    *outBack = true;
    return -1;
  }

  for (int i = 0; i < 5; i++) {
    int rowY = 48 + i * 35;
    if (y >= rowY && y <= rowY + 30) {
      return i;
    }
  }
  return -1;
}

// ============================================================================
// 8. APLICACIÓN DEDICADA: TELEMETRÍA BATERÍA 5000 mAh
// ============================================================================
void UIManager::drawBatteryView(const BatteryManager &battery,
                                bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, wifiConnected, false, true, 0,
                       "BATERIA 5000 mAh");

  int pct = battery.getPercentage();
  float v = battery.getVoltage();
  bool charging = battery.isCharging();
  float hrs = battery.getEstimatedHoursLeft();

  _tft.fillRoundRect(16, 32, 288, 140, 6, COLOR_CARD);
  _tft.drawRoundRect(16, 32, 288, 140, 6, COLOR_BORDER);

  drawAppColorIcon(50, 75, 7);

  char pctStr[16];
  snprintf(pctStr, sizeof(pctStr), "%d", pct);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  int numW = _tft.drawString(pctStr, 80, 52, 6);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("%", 80 + numW + 3, 62, 4);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(charging ? "Cargando LiPo..." : "Descarga normal", 80, 96, 2);

  int barW = 256;
  _tft.fillRoundRect(32, 118, barW, 10, 3, COLOR_BG);
  _tft.drawRoundRect(32, 118, barW, 10, 3, COLOR_BORDER);
  int fW = map(constrain(pct, 0, 100), 0, 100, 0, barW - 2);
  if (fW > 0) {
    _tft.fillRoundRect(33, 119, fW, 8, 2, 0x07E0);
  }

  char infoBuf[48];
  if (v < 0.5f) {
    snprintf(infoBuf, sizeof(infoBuf), "Alimentacion USB (5.0V)  |  18.5 Wh");
  } else {
    snprintf(infoBuf, sizeof(infoBuf), "%.2f V  |  ~%.1f h restantes  |  18.5 Wh",
             v, hrs);
  }
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString(infoBuf, 160, 144, 2);

  _tft.fillRoundRect(16, 184, 288, 44, 5, COLOR_CARD);
  _tft.drawRoundRect(16, 184, 288, 44, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawCentreString("Cargador USB-C integrado | Proteccion LiPo DW01",
                        160, 200, 1);
}

bool UIManager::handleBatteryTouch(int x, int y) {
  if (x >= 4 && x <= 64 && y >= 0 && y <= 24)
    return true;
  return false;
}

// ============================================================================
// 9. APLICACIÓN DEDICADA: ESTADÍSTICAS & HISTORIAL MICROSD
// ============================================================================
void UIManager::drawStatsView(StorageManager &storage,
                              const PomodoroEngine &pomodoro,
                              const BatteryManager &battery,
                              bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, wifiConnected, false, true, 0, "STATS ESTUDIO");

  int sessionsToday = pomodoro.getCompletedSessionsToday();
  int focusMinToday = sessionsToday * pomodoro.getFocusMinutes();
  int totalLogged = storage.getTotalLoggedSessions();

  _tft.fillRoundRect(16, 32, 138, 70, 5, COLOR_CARD);
  _tft.drawRoundRect(16, 32, 138, 70, 5, COLOR_BORDER);
  _tft.fillCircle(24, 42, 3, 0xFA08); // Acento rojo tomate Pomodoro
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("SESIONES HOY", 32, 38, 1);
  char sBuf[12];
  snprintf(sBuf, sizeof(sBuf), "%d", sessionsToday);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(sBuf, 24, 52, 6);

  _tft.fillRoundRect(166, 32, 138, 70, 5, COLOR_CARD);
  _tft.drawRoundRect(166, 32, 138, 70, 5, COLOR_BORDER);
  _tft.fillCircle(174, 42, 3, 0x07FF); // Acento cian enfoque
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("TIEMPO TOTAL", 182, 38, 1);
  char mBuf[16];
  snprintf(mBuf, sizeof(mBuf), "%dh %02dm", focusMinToday / 60,
           focusMinToday % 60);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(mBuf, 174, 58, 4);

  _tft.fillRoundRect(16, 110, 288, 76, 5, COLOR_CARD);
  _tft.drawRoundRect(16, 110, 288, 76, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("REGISTRO EN TARJETA MICROSD", 24, 116, 1);

  char lBuf[64];
  snprintf(lBuf, sizeof(lBuf), "Total acumulado: %d sesiones guardadas",
           totalLogged);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(lBuf, 24, 134, 2);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Archivo: /sessions.csv (Excel / Python)", 24, 158, 1);

  _tft.fillRoundRect(16, 194, 288, 36, 5, COLOR_CARD);
  _tft.drawRoundRect(16, 194, 288, 36, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("< VOLVER AL MENU", 160, 204, 2);
}

bool UIManager::handleStatsTouch(int x, int y) {
  if (x >= 4 && x <= 64 && y >= 0 && y <= 24)
    return true;
  if (y >= 190 && y <= 236)
    return true;
  return false;
}

// ============================================================================
// 10. APLICACIÓN AJUSTES DEL SISTEMA
// ============================================================================
void UIManager::drawSettingsView(const WiFiConfig &wifi,
                                 const BatteryManager &battery, int brightness,
                                 bool autoBrightness, bool soundMuted,
                                 int timeoutMinutes) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, wifi.isConnected(), soundMuted, autoBrightness,
                       0, "AJUSTES");

  // 1. Brillo de Pantalla (y: 28..70)
  _tft.fillRoundRect(12, 28, 296, 44, 4, COLOR_CARD);
  _tft.drawRoundRect(12, 28, 296, 44, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Brillo TFT:", 20, 34, 2);

  _tft.fillRoundRect(106, 34, 52, 30, 3,
                     autoBrightness ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(106, 34, 52, 30, 3, COLOR_BORDER);
  _tft.setTextColor(autoBrightness ? COLOR_BG : COLOR_TEXT_SECONDARY,
                    autoBrightness ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("AUTO", 132, 42, 2);

  _tft.fillRoundRect(166, 34, 38, 30, 3, COLOR_CARD);
  _tft.drawRoundRect(166, 34, 38, 30, 3, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("-", 185, 38, 4);

  char briStr[8];
  snprintf(briStr, sizeof(briStr), "%d", (brightness * 100) / 255);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(briStr, 230, 42, 2);

  _tft.fillRoundRect(256, 34, 38, 30, 3, COLOR_CARD);
  _tft.drawRoundRect(256, 34, 38, 30, 3, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("+", 275, 38, 4);

  // 2. Audio Notificador (y: 78..116)
  _tft.fillRoundRect(12, 78, 296, 40, 4, COLOR_CARD);
  _tft.drawRoundRect(12, 78, 296, 40, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Zumbador / Audio:", 20, 88, 2);

  _tft.fillRoundRect(200, 83, 94, 28, 3,
                     soundMuted ? COLOR_CARD : COLOR_TEXT_PRIMARY);
  _tft.drawRoundRect(200, 83, 94, 28, 3, COLOR_BORDER);
  _tft.setTextColor(soundMuted ? COLOR_TEXT_MUTED : COLOR_BG,
                    soundMuted ? COLOR_CARD : COLOR_TEXT_PRIMARY);
  _tft.drawCentreString(soundMuted ? "MUTED" : "ACTIVO", 247, 89, 2);

  // 3. Apagado automático de pantalla (y: 124..162)
  _tft.fillRoundRect(12, 124, 296, 40, 4, COLOR_CARD);
  _tft.drawRoundRect(12, 124, 296, 40, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Reposo pantalla:", 20, 134, 2);

  char toStr[16];
  if (timeoutMinutes == 0)
    snprintf(toStr, sizeof(toStr), "Nunca");
  else
    snprintf(toStr, sizeof(toStr), "%d min", timeoutMinutes);
  _tft.fillRoundRect(200, 129, 94, 28, 3, COLOR_CARD);
  _tft.drawRoundRect(200, 129, 94, 28, 3, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(toStr, 247, 135, 2);

  // 4. Red Wi-Fi (y: 170..208)
  _tft.fillRoundRect(12, 170, 296, 40, 4, COLOR_CARD);
  _tft.drawRoundRect(12, 170, 296, 40, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Conexion Wi-Fi:", 20, 180, 2);

  _tft.fillRoundRect(200, 175, 94, 28, 3,
                     wifi.isConnected() ? 0x051D : COLOR_CARD_HL);
  _tft.drawRoundRect(200, 175, 94, 28, 3, COLOR_BORDER);
  _tft.setTextColor(0xFFFF, wifi.isConnected() ? 0x051D : COLOR_CARD_HL);
  _tft.drawCentreString(wifi.isConnected() ? "CONFIGURAR" : "CONECTAR", 247,
                        181, 2);

  // Botón Salir
  _tft.fillRoundRect(12, 214, 296, 22, 3, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawCentreString("< Volver al Menu Principal", 160, 217, 1);
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

  if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
    *outBack = true;
    return 0;
  }

  if (y >= 210 && y <= 240) {
    *outBack = true;
    return 0;
  }

  if (y >= 30 && y <= 72) {
    if (x >= 106 && x <= 160)
      *outToggleAuto = true;
    else if (x >= 164 && x <= 204)
      *outBriDown = true;
    else if (x >= 254 && x <= 296)
      *outBriUp = true;
    return 1;
  }

  if (y >= 78 && y <= 120 && x >= 195 && x <= 300) {
    *outToggleMute = true;
    return 2;
  }

  if (y >= 124 && y <= 166 && x >= 195 && x <= 300) {
    *outTimeoutToggle = true;
    return 3;
  }

  if (y >= 170 && y <= 212 && x >= 195 && x <= 300) {
    *outOpenWifi = true;
    return 4;
  }

  return 0;
}

// ============================================================================
// 11. APLICACIÓN INFO DEL SISTEMA & HARDWARE
// ============================================================================
void UIManager::drawInfoView(const BatteryManager &battery,
                             StorageManager &storage,
                             const PomodoroEngine & /*pomodoro*/,
                             bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, wifiConnected, false, true, 0,
                       "INFO STUDY DECK");

  _tft.fillRoundRect(14, 28, 292, 174, 5, COLOR_CARD);
  _tft.drawRoundRect(14, 28, 292, 174, 5, COLOR_BORDER);

  int y = 34;
  _tft.setTextColor(0xFDE0, COLOR_CARD);
  _tft.drawString("STUDY DECK - EDICION ACADEMICA UNED", 24, y, 2);
  y += 20;

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Hardware: Guition ESP32-2432S024C (2.4\")", 24, y, 2);
  y += 18;
  _tft.drawString("Tactil: CST820 Capacitivo I2C (320x240)", 24, y, 2);
  y += 18;
  _tft.drawString("Bateria: LiPo 5000 mAh 3.7V (18.5 Wh)", 24, y, 2);
  y += 18;
  _tft.drawString("Audio: SC8002B (Alarma Estudio 3s)", 24, y, 2);
  y += 18;
  _tft.drawString("MicroSD: " + String(storage.isAvailable()
                                           ? "Detectada (FAT32)"
                                           : "No detectada"),
                  24, y, 2);
  y += 18;
  _tft.drawString("Firmware: v1.3.0 UNED Avila Edition", 24, y, 2);
  y += 18;
  _tft.drawString("Grado en Ingenieria Informatica", 24, y, 2);

  _tft.fillRoundRect(14, 206, 292, 28, 4, COLOR_CARD);
  _tft.drawRoundRect(14, 206, 292, 28, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("< VOLVER AL MENU PRINCIPAL", 160, 212, 2);
}

bool UIManager::handleInfoTouch(int x, int y) {
  if (x >= 4 && x <= 64 && y >= 0 && y <= 24)
    return true;
  if (y >= 200 && y <= 238)
    return true;
  return false;
}

// ============================================================================
// SUBVISTAS DE CONFIGURACIÓN WI-FI
// ============================================================================
void UIManager::drawWiFiScanView(const WiFiConfig &wifi,
                                 const BatteryManager &battery, bool scanning,
                                 int page) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, wifi.isConnected(), false, true, 0,
                       "REDES WI-FI");

  if (scanning) {
    _tft.fillRoundRect(20, 80, 280, 70, 5, COLOR_CARD);
    _tft.drawRoundRect(20, 80, 280, 70, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("Buscando redes Wi-Fi 2.4 GHz...", 160, 100, 2);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString("Espere unos segundos...", 160, 122, 1);
    return;
  }

  int totalNets = wifi.getNetworkCount();
  int netsPerPage = 4;
  int startIndex = page * netsPerPage;

  if (totalNets == 0) {
    _tft.fillRoundRect(20, 80, 280, 70, 5, COLOR_CARD);
    _tft.drawRoundRect(20, 80, 280, 70, 5, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("No se encontraron redes Wi-Fi", 160, 100, 2);
  } else {
    int showCount = min(netsPerPage, totalNets - startIndex);
    for (int i = 0; i < showCount; i++) {
      int idx = startIndex + i;
      DiscoveredNetwork net = wifi.getNetwork(idx);
      String ssid = net.ssid;
      int32_t rssi = net.rssi;
      bool open = !net.isEncrypted;

      int y = 28 + i * 36;
      _tft.fillRoundRect(12, y, 296, 32, 4, COLOR_CARD);
      _tft.drawRoundRect(12, y, 296, 32, 4, COLOR_BORDER);

      if (ssid.length() > 22)
        ssid = ssid.substring(0, 20) + "..";
      _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
      _tft.drawString(ssid, 20, y + 8, 2);

      char inf[20];
      snprintf(inf, sizeof(inf), "%s %d dBm", open ? "[ABI]" : "[SEG]",
               (int)rssi);
      _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
      _tft.drawRightString(inf, 300, y + 10, 1);
    }
  }

  // Botonera inferior
  _tft.fillRoundRect(12, 178, 142, 34, 4, COLOR_CARD);
  _tft.drawRoundRect(12, 178, 142, 34, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("REESCANEAR", 83, 187, 2);

  bool hasNext = (startIndex + netsPerPage < totalNets);
  _tft.fillRoundRect(166, 178, 142, 34, 4,
                     hasNext ? COLOR_CARD_HL : COLOR_CARD);
  _tft.drawRoundRect(166, 178, 142, 34, 4, COLOR_BORDER);
  _tft.setTextColor(hasNext ? COLOR_TEXT_PRIMARY : COLOR_TEXT_MUTED,
                    hasNext ? COLOR_CARD_HL : COLOR_CARD);
  _tft.drawCentreString("MAS REDES >", 237, 187, 2);

  _tft.fillRoundRect(12, 216, 296, 20, 3, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawCentreString("< Volver a Ajustes", 160, 218, 1);
}

int UIManager::handleWiFiScanTouch(int x, int y, int netCount, int page,
                                   bool *outRescan, bool *outBack,
                                   bool *outNextPage) {
  *outRescan = false;
  *outBack = false;
  *outNextPage = false;

  if (x >= 4 && x <= 64 && y >= 0 && y <= 24) {
    *outBack = true;
    return -1;
  }
  if (y >= 214 && y <= 240) {
    *outBack = true;
    return -1;
  }

  if (y >= 174 && y <= 212) {
    if (x >= 12 && x <= 154) {
      *outRescan = true;
      return -1;
    } else if (x >= 166 && x <= 308) {
      *outNextPage = true;
      return -1;
    }
  }

  int startIndex = page * 4;
  for (int i = 0; i < 4; i++) {
    int cardY = 28 + i * 36;
    if (y >= cardY && y <= cardY + 32) {
      int netIdx = startIndex + i;
      if (netIdx < netCount) {
        return netIdx;
      }
    }
  }
  return -1;
}

void UIManager::drawWiFiConnecting(const String &ssid) {
  _tft.fillScreen(COLOR_BG);
  _tft.fillRoundRect(20, 60, 280, 120, 6, COLOR_CARD);
  _tft.drawRoundRect(20, 60, 280, 120, 6, COLOR_BORDER);

  _tft.setTextColor(0xFDE0, COLOR_CARD);
  _tft.drawCentreString("CONECTANDO A WI-FI", 160, 80, 4);

  String s = ssid;
  if (s.length() > 22)
    s = s.substring(0, 20) + "..";
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(s, 160, 116, 2);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString("Negociando direccion IP...", 160, 144, 1);
}

void UIManager::drawWiFiResult(bool success, const String &info) {
  _tft.fillScreen(COLOR_BG);
  _tft.fillRoundRect(20, 60, 280, 120, 6, COLOR_CARD);
  _tft.drawRoundRect(20, 60, 280, 120, 6, success ? 0x07E0 : 0xF800);

  _tft.setTextColor(success ? 0x07E0 : 0xF800, COLOR_CARD);
  _tft.drawCentreString(success ? "CONEXION EXITOSA" : "ERROR DE CONEXION", 160,
                        80, 4);

  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(info, 160, 120, 2);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString("Red lista para sincronizacion", 160, 148, 1);
}
