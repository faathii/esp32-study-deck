#include "UIManager.h"
#include <time.h>

// ============================================================================
// SISTEMA DE DISEÑO: CYBER-STUDIO DECK OS v2 (AERO-CYBER MINIMALIST 16-BIT)
// ============================================================================
// Fondo negro absoluto (efecto OLED en panel IPS/TFT para contraste infinito)
#define COLOR_BG              0x0000 
#define COLOR_STATUS_BAR      0x0000 
#define COLOR_SEPARATOR       0x18C3 // Línea de separación sutil grafito (24, 24, 24)

// Superficies elevadas y tarjetas (Dark Carbon Glass)
#define COLOR_CARD            0x0842 // Carbón profundo para islas de contenido (8, 8, 8)
#define COLOR_CARD_HL         0x10A3 // Superficie interactiva / ranura seleccionada (16, 20, 24)
#define COLOR_CARD_ACTIVE     0x1925 // Pizarra activa con relieve (24, 36, 40)
#define COLOR_BORDER          0x2145 // Borde fino de 1px tipo titanio (33, 40, 41)
#define COLOR_BORDER_HL       0xFFFF // Borde destacado blanco nítido

// Tipografía y jerarquía visual
#define COLOR_TEXT_PRIMARY    0xFFFF // Blanco puro (máximo contraste para dígitos y títulos)
#define COLOR_TEXT_SECONDARY  0xC67A // Plata titanio (198, 206, 210) para subtítulos y datos
#define COLOR_TEXT_MUTED      0x52AA // Grafito oscuro (82, 85, 82) para unidades y leyendas

// Acentos cromáticos semánticos
#define COLOR_ACCENT_CYAN     0x07FF // Cian eléctrico (Tiempo, Wi-Fi, selección activa)
#define COLOR_ACCENT_CORAL    0xFA60 // Coral tomate brillante (Enfoque Pomodoro, alertas)
#define COLOR_ACCENT_MINT     0x2FEA // Verde menta zen (Descanso, relax, estado óptimo)
#define COLOR_ACCENT_AMBER    0xFFC0 // Ámbar cálido / Oro (Lámpara de noche, Sol, avisos)
#define COLOR_ACCENT_ROYAL    0xA2BF // Amatista / Púrpura UNED (Calendario académico, hitos)
#define COLOR_ACCENT_EMERALD  0x07E0 // Verde esmeralda (Batería 100%, carga activa)

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
// BARRA SUPERIOR UNIFICADA DE PRECISIÓN (STATUS BAR)
// ============================================================================
void UIManager::drawUnifiedStatusBar(const BatteryManager &battery,
                                     bool wifiConnected, bool audioMuted,
                                     bool autoBrightness, int streakCount,
                                     const char *appTitle) {
  _tft.fillRect(0, 0, SCREEN_WIDTH, 22, COLOR_STATUS_BAR);
  _tft.setTextDatum(TL_DATUM);

  // 1. Izquierda: Botón de Navegación o Logotipo Académico
  if (appTitle != nullptr) {
    // Botón volver estilo cápsula táctil
    _tft.fillRoundRect(4, 2, 54, 18, 3, COLOR_CARD_HL);
    _tft.drawRoundRect(4, 2, 54, 18, 3, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
    _tft.drawString("< ATRAS", 8, 5, 1);

    // Título de la aplicación activa
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_STATUS_BAR);
    _tft.drawString(appTitle, 66, 4, 2);
  } else {
    // Identificador Standby: UNED ÁVILA con punto de estado
    _tft.fillCircle(8, 11, 3, COLOR_ACCENT_MINT);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_STATUS_BAR);
    _tft.drawString("STUDY DECK", 16, 4, 2);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_STATUS_BAR);
    _tft.drawString("UNED", 98, 5, 1);
  }

  // 2. Derecha: Telemetría de hardware (Batería, Wi-Fi, Audio, Brillo)
  int batPct = battery.getPercentage();
  bool isCharging = battery.isCharging();
  float vBat = battery.getVoltage();
  bool usbPower = (vBat < 0.5f);

  // Cápsula de Batería (x: 294..314)
  _tft.drawRoundRect(294, 6, 20, 10, 2, COLOR_TEXT_SECONDARY);
  _tft.fillRect(314, 8, 2, 6, COLOR_TEXT_SECONDARY); // Terminal positivo
  int fillW = map(constrain(batPct, 0, 100), 0, 100, 0, 16);
  if (fillW > 0) {
    uint16_t bCol = (isCharging || usbPower) ? COLOR_ACCENT_EMERALD : ((batPct > 20) ? COLOR_TEXT_PRIMARY : 0xF800);
    _tft.fillRect(296, 8, fillW, 6, bCol);
  }
  if (isCharging || usbPower) {
    // Mini rayo de carga
    _tft.drawPixel(303, 7, COLOR_ACCENT_AMBER);
    _tft.drawPixel(304, 8, COLOR_ACCENT_AMBER);
    _tft.drawPixel(303, 9, COLOR_ACCENT_AMBER);
    _tft.drawPixel(304, 10, COLOR_ACCENT_AMBER);
  }

  // Porcentaje numérico
  char batStr[8];
  snprintf(batStr, sizeof(batStr), "%d%%", batPct);
  _tft.setTextDatum(TR_DATUM);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_STATUS_BAR);
  _tft.drawString(batStr, 290, 5, 1);

  // Indicador Wi-Fi (Verde si conectado, gris si desconectado)
  _tft.setTextColor(wifiConnected ? COLOR_ACCENT_CYAN : COLOR_TEXT_MUTED, COLOR_STATUS_BAR);
  _tft.drawString(wifiConnected ? "WIFI" : "OFF", 258, 5, 1);

  // Indicador de Mute de Audio
  int flagX = 232;
  if (audioMuted) {
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_STATUS_BAR);
    _tft.drawString("MUTE", flagX, 5, 1);
    flagX -= 28;
  }

  // Indicador de Brillo Automático (LDR)
  if (autoBrightness) {
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_STATUS_BAR);
    _tft.drawString("AUTO", flagX, 5, 1);
  }

  // Línea de separación láser de 1px
  _tft.drawFastHLine(0, 22, SCREEN_WIDTH, COLOR_SEPARATOR);
  _tft.setTextDatum(TL_DATUM);
}

// ============================================================================
// MICRO-ICONOS VECTORIALES A TODO COLOR (ALTA PRECISIÓN 16-BIT)
// ============================================================================
void UIManager::drawAppColorIcon(int cx, int cy, int appType) {
  switch (appType) {
  case 1: // 🍅 Pomodoro Chrono (Tomate 3D con reflejo y tallo verde)
    _tft.fillCircle(cx, cy + 2, 11, COLOR_ACCENT_CORAL);
    _tft.fillCircle(cx + 4, cy + 2, 8, 0xE9E0); // Sombra lateral
    _tft.fillCircle(cx - 3, cy - 1, 9, COLOR_ACCENT_CORAL);
    _tft.drawCircle(cx - 4, cy - 1, 4, 0xFDC8); // Reflejo especular
    // Tallo y hojas verdes
    _tft.fillRect(cx - 1, cy - 10, 3, 5, 0x1CE0);
    _tft.fillTriangle(cx, cy - 7, cx - 7, cy - 10, cx - 2, cy - 5, 0x1CE0);
    _tft.fillTriangle(cx, cy - 7, cx + 7, cy - 10, cx + 2, cy - 5, 0x1CE0);
    break;

  case 2: // 📅 Agenda Académica (Cuaderno anillado con hoja blanca y fecha roja)
    _tft.fillRoundRect(cx - 11, cy - 9, 22, 22, 3, 0xFFFF);
    _tft.fillRoundRect(cx - 11, cy - 9, 22, 7, 3, COLOR_ACCENT_ROYAL);
    // Anillas metálicas
    _tft.fillRect(cx - 8, cy - 11, 2, 4, 0x2145);
    _tft.fillRect(cx - 1, cy - 11, 2, 4, 0x2145);
    _tft.fillRect(cx + 6, cy - 11, 2, 4, 0x2145);
    // Cuadrícula de tareas y fecha destacada
    _tft.fillRect(cx - 7, cy + 2, 4, 3, 0x04BF);
    _tft.fillRect(cx - 1, cy + 2, 4, 3, 0x04BF);
    _tft.fillRect(cx + 5, cy + 2, 4, 3, 0x04BF);
    _tft.fillRect(cx - 7, cy + 7, 4, 3, 0x04BF);
    _tft.fillRect(cx - 1, cy + 7, 4, 3, COLOR_ACCENT_CORAL); // Hito HOY en coral
    _tft.fillRect(cx + 5, cy + 7, 4, 3, 0x04BF);
    break;

  case 3: // 🌤️ Clima Atmosférico (Sol dorado radiante con nube y gotas de lluvia)
    _tft.fillCircle(cx - 3, cy - 3, 8, COLOR_ACCENT_AMBER);
    // Rayos solares
    _tft.drawFastVLine(cx - 3, cy - 14, 3, COLOR_ACCENT_AMBER);
    _tft.drawFastHLine(cx - 14, cy - 3, 3, COLOR_ACCENT_AMBER);
    _tft.drawLine(cx - 10, cy - 10, cx - 8, cy - 8, COLOR_ACCENT_AMBER);
    // Nube volumétrica azul cielo
    _tft.fillCircle(cx + 4, cy + 4, 6, 0xCE7F);
    _tft.fillCircle(cx - 1, cy + 5, 5, 0xCE7F);
    _tft.fillRoundRect(cx - 5, cy + 5, 15, 6, 2, 0xCE7F);
    // Gotitas de rocío en cian
    _tft.drawPixel(cx - 1, cy + 12, COLOR_ACCENT_CYAN);
    _tft.drawPixel(cx + 5, cy + 12, COLOR_ACCENT_CYAN);
    break;

  case 4: // 🌙 Reloj Nocturno (Luna creciente mística y estrellas en cielo profundo)
    _tft.fillCircle(cx, cy, 10, 0x723F); // Fondo violeta profundo
    _tft.fillCircle(cx - 1, cy, 9, 0x93FF); // Luna
    _tft.fillCircle(cx + 3, cy - 2, 8, 0x723F); // Recorte de media luna
    // Estrellas de 4 puntos
    _tft.drawPixel(cx + 6, cy + 4, COLOR_ACCENT_AMBER);
    _tft.drawPixel(cx - 6, cy - 7, COLOR_ACCENT_AMBER);
    break;

  case 5: // 🫁 Relax Box (Mandala concéntrico de respiración zen)
    _tft.drawRoundRect(cx - 11, cy - 11, 22, 22, 4, COLOR_ACCENT_MINT);
    _tft.drawRoundRect(cx - 7, cy - 7, 14, 14, 3, COLOR_ACCENT_CYAN);
    _tft.fillCircle(cx, cy, 3, COLOR_TEXT_PRIMARY);
    break;

  case 6: // 💡 Lámpara RGB (Bombilla Edison con filamento incandescente)
    _tft.fillCircle(cx, cy - 3, 8, COLOR_ACCENT_AMBER);
    _tft.drawCircle(cx, cy - 3, 10, 0x8200); // Halo suave
    _tft.fillRect(cx - 4, cy + 4, 8, 4, 0xC618);
    _tft.drawFastHLine(cx - 3, cy + 6, 6, 0x7BEF);
    _tft.drawLine(cx - 11, cy - 9, cx - 8, cy - 6, COLOR_ACCENT_AMBER);
    _tft.drawLine(cx + 11, cy - 9, cx + 8, cy - 6, COLOR_ACCENT_AMBER);
    break;

  case 7: // 🔋 Batería 5000 mAh (Cápsula de litio con ánodo y rayo dorado)
    _tft.drawRoundRect(cx - 11, cy - 6, 20, 13, 2, COLOR_TEXT_PRIMARY);
    _tft.fillRect(cx + 9, cy - 2, 3, 5, 0xC618);
    _tft.fillRect(cx - 9, cy - 4, 14, 9, COLOR_ACCENT_EMERALD);
    _tft.drawLine(cx - 3, cy - 3, cx - 1, cy, COLOR_ACCENT_AMBER);
    _tft.drawLine(cx - 1, cy, cx - 4, cy + 3, COLOR_ACCENT_AMBER);
    break;

  case 8: // 📊 Estadísticas MicroSD (Histograma analítico en tarjeta de memoria)
    _tft.fillRoundRect(cx - 11, cy - 10, 22, 22, 3, COLOR_CARD_HL);
    _tft.drawRoundRect(cx - 11, cy - 10, 22, 22, 3, COLOR_BORDER);
    _tft.fillRect(cx - 8, cy + 2, 4, 7, COLOR_ACCENT_CORAL);
    _tft.fillRect(cx - 2, cy - 4, 4, 13, COLOR_ACCENT_CYAN);
    _tft.fillRect(cx + 4, cy - 8, 4, 17, COLOR_ACCENT_MINT);
    _tft.drawFastHLine(cx - 9, cy + 9, 18, COLOR_TEXT_PRIMARY);
    break;

  case 9: // ⚙️ Ajustes del Sistema (Engranaje de precisión con núcleo cian)
    _tft.fillCircle(cx, cy, 10, 0x8410);
    _tft.drawCircle(cx, cy, 10, COLOR_TEXT_SECONDARY);
    _tft.drawFastVLine(cx, cy - 12, 24, COLOR_TEXT_PRIMARY);
    _tft.drawFastHLine(cx - 12, cy, 24, COLOR_TEXT_PRIMARY);
    _tft.fillCircle(cx, cy, 4, COLOR_BG);
    _tft.drawCircle(cx, cy, 4, COLOR_ACCENT_CYAN);
    break;

  case 10: // ℹ️ Info & Hardware (Microchip de silicio con terminales y glifo "i")
    _tft.fillRoundRect(cx - 10, cy - 10, 20, 20, 3, 0x1925);
    _tft.drawRoundRect(cx - 10, cy - 10, 20, 20, 3, COLOR_ACCENT_CYAN);
    _tft.fillCircle(cx, cy - 4, 2, COLOR_TEXT_PRIMARY);
    _tft.fillRect(cx - 1, cy - 1, 3, 6, COLOR_TEXT_PRIMARY);
    break;

  default:
    _tft.drawRect(cx - 8, cy - 8, 16, 16, COLOR_BORDER);
    break;
  }
}

// Tarjeta de aplicación ergonómica para cuadrícula 2x3
void UIManager::drawAppCard(int x, int y, int w, int h, uint16_t bgColor,
                            int appType, const char *title,
                            const char *subtitle) {
  _tft.fillRoundRect(x, y, w, h, 6, bgColor);
  _tft.drawRoundRect(x, y, w, h, 6, COLOR_BORDER);

  // Micro-insignia circular para el icono
  _tft.fillCircle(x + w / 2, y + 25, 17, 0x0000);
  _tft.drawCircle(x + w / 2, y + 25, 17, COLOR_BORDER);
  drawAppColorIcon(x + w / 2, y + 25, appType);

  // Título en blanco nítido
  _tft.setTextColor(COLOR_TEXT_PRIMARY, bgColor);
  _tft.drawCentreString(title, x + w / 2, y + 46, 2);

  // Subtítulo de telemetría / estado
  _tft.setTextColor(COLOR_TEXT_SECONDARY, bgColor);
  _tft.drawCentreString(subtitle, x + w / 2, y + 64, 1);
}

// ============================================================================
// 1. PANTALLA INICIAL PREDEFINIDA: ESTACIÓN CLIMA & RELOJ DESK (STANDBY)
// ============================================================================
void UIManager::drawWeatherView(const WeatherManager &weather,
                                CalendarManager &calendar,
                                const BatteryManager &battery,
                                bool wifiConnected) {
  _tft.fillScreen(COLOR_BG);
  _tft.setTextDatum(TL_DATUM);
  drawUnifiedStatusBar(battery, wifiConnected, false, true, 0, nullptr);
  _tft.setTextDatum(TL_DATUM);

  // 1. Panel Izquierdo: Reloj Desk & Calendario Vivo (x: 8..188, y: 26..148)
  _tft.fillRoundRect(8, 26, 180, 122, 6, COLOR_CARD);
  _tft.drawRoundRect(8, 26, 180, 122, 6, COLOR_BORDER);

  // Indicador de sincronización NTP
  _tft.fillCircle(18, 36, 3, wifiConnected ? COLOR_ACCENT_CYAN : COLOR_TEXT_MUTED);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString(wifiConnected ? "RELOJ EXACTO NTP" : "HORA RTC", 26, 32, 1);

  // Hora digital grande y fecha
  updateWeatherViewClock(weather, calendar);

  // 2. Panel Derecho: Estación Atmosférica de Ávila (x: 194..312, y: 26..148)
  _tft.fillRoundRect(194, 26, 118, 122, 6, COLOR_CARD);
  _tft.drawRoundRect(194, 26, 118, 122, 6, COLOR_BORDER);

  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawCentreString("ESTACION AVILA", 253, 32, 1);

  // Icono meteorológico grande centrado
  drawAppColorIcon(253, 62, 3);

  // Temperatura
  char tBuf[16];
  if (weather.hasData()) {
    snprintf(tBuf, sizeof(tBuf), "%.1f C", weather.getTemperature());
  } else {
    snprintf(tBuf, sizeof(tBuf), "--.- C");
  }
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(tBuf, 253, 86, 4);

  // Humedad relativa y condición
  char hBuf[32];
  if (weather.hasData()) {
    snprintf(hBuf, sizeof(hBuf), "%d%% HR | %s", weather.getHumidity(),
             weather.getConditionText().c_str());
  } else {
    snprintf(hBuf, sizeof(hBuf), "Conectando...");
  }
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString(hBuf, 253, 118, 1);

  // 3. Franja Intermedia: Radar Académico UNED (x: 8..312, y: 154..196)
  _tft.fillRoundRect(8, 154, 304, 42, 5, COLOR_CARD);
  _tft.drawRoundRect(8, 154, 304, 42, 5, COLOR_BORDER);

  CalendarEvent todaySession;
  bool hasSession = calendar.getTodayMainSession(todaySession);
  bool ongoing = calendar.isStudySessionOngoing();

  if (ongoing) {
    // Alerta de sesión en curso (Coral fuego pulsante)
    _tft.fillRoundRect(10, 156, 300, 38, 4, 0x3800);
    _tft.drawRoundRect(10, 156, 300, 38, 4, COLOR_ACCENT_CORAL);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, 0x3800);
    _tft.drawCentreString("SESION DE ESTUDIO EN CURSO", 160, 160, 2);
    _tft.setTextColor(COLOR_ACCENT_AMBER, 0x3800);
    _tft.drawCentreString(todaySession.timeRange + " | " + todaySession.title.substring(0, 26), 160, 178, 1);
  } else if (hasSession) {
    _tft.fillCircle(18, 175, 4, COLOR_ACCENT_CYAN);
    _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
    _tft.drawString(todaySession.timeRange, 28, 160, 2);

    String sum = todaySession.title;
    if (sum.length() > 28)
      sum = sum.substring(0, 26) + "..";
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(sum, 28, 178, 1);

    // Etiqueta UNED
    _tft.fillRoundRect(248, 162, 56, 24, 3, COLOR_CARD_HL);
    _tft.drawRoundRect(248, 162, 56, 24, 3, COLOR_ACCENT_CYAN);
    _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD_HL);
    _tft.drawCentreString("UNED", 276, 168, 1);
  } else {
    String examTitle;
    String countdown = calendar.getNextDeadlineCountdown(examTitle);
    _tft.fillCircle(18, 175, 4, COLOR_ACCENT_ROYAL);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawString("PROXIMO HITO ACADEMICO", 28, 160, 1);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(examTitle.substring(0, 22), 28, 174, 2);

    _tft.setTextColor(COLOR_ACCENT_ROYAL, COLOR_CARD);
    _tft.drawRightString(countdown, 302, 174, 2);
  }

  // 4. Botón Táctil Inferior para abrir el menú de aplicaciones (y: 202..234)
  _tft.fillRoundRect(8, 202, 304, 32, 5, COLOR_CARD_HL);
  _tft.drawRoundRect(8, 202, 304, 32, 5, COLOR_BORDER_HL);
  _tft.fillCircle(20, 218, 3, COLOR_ACCENT_CYAN);
  _tft.fillCircle(300, 218, 3, COLOR_ACCENT_CYAN);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString("TOCAR PANTALLA PARA ABRIR APPS", 160, 210, 2);
}

void UIManager::updateWeatherViewClock(const WeatherManager & /*weather*/,
                                       CalendarManager & /*calendar*/) {
  time_t now = time(nullptr);
  struct tm *t = localtime(&now);

  char timeBuf[16];
  char dateBuf[48];

  if (t->tm_year > 120) {
    snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t->tm_hour, t->tm_min);
    const char *dias[7] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
    const char *meses[12] = {"Ene", "Feb", "Mar", "Abr", "May", "Jun", "Jul", "Ago", "Sep", "Oct", "Nov", "Dic"};
    snprintf(dateBuf, sizeof(dateBuf), "%s, %d %s %d", dias[t->tm_wday],
             t->tm_mday, meses[t->tm_mon], t->tm_year + 1900);
  } else {
    snprintf(timeBuf, sizeof(timeBuf), "12:00");
    snprintf(dateBuf, sizeof(dateBuf), "Sincronizando hora...");
  }

  // Render del reloj digital centrado en el panel izquierdo (x: 8..188)
  _tft.fillRect(12, 48, 172, 52, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(timeBuf, 98, 50, 7); // Font 7: 7-segment digital look

  // Fecha debajo del reloj
  _tft.fillRect(12, 108, 172, 28, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString(dateBuf, 98, 114, 2);
  _tft.setTextDatum(TL_DATUM);
}

bool UIManager::handleWeatherTouch(int /*x*/, int /*y*/, bool *outOpenMenu) {
  *outOpenMenu = true;
  return true; // Cualquier toque en pantalla abre el menú de aplicaciones
}

// ============================================================================
// 2. MENÚ LANZADOR DE APLICACIONES (APP LAUNCHER 2x3 MATRIX)
// ============================================================================
void UIManager::drawHomeScreen(const PomodoroEngine &engine,
                               const BatteryManager &battery,
                               const WeatherManager &weather,
                               bool wifiConnected, bool audioMuted,
                               bool autoBrightness, int homeTab) {
  _tft.fillScreen(COLOR_BG);

  // Barra superior con botón volver a Clima (Standby)
  drawUnifiedStatusBar(battery, wifiConnected, audioMuted, autoBrightness,
                       engine.getCompletedSessionsToday(), "MENU DE APPS");
  _tft.setTextDatum(TL_DATUM);

  // Selector de Pestañas Superiores (x: 8 y 164, y: 25..46)
  bool tab0 = (homeTab == 0);
  _tft.fillRoundRect(8, 25, 148, 22, 4, tab0 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(8, 25, 148, 22, 4, tab0 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(tab0 ? COLOR_BG : COLOR_TEXT_SECONDARY, tab0 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("1. ESTUDIO & DESK", 82, 29, 2);

  bool tab1 = (homeTab == 1);
  _tft.fillRoundRect(164, 25, 148, 22, 4, tab1 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(164, 25, 148, 22, 4, tab1 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(tab1 ? COLOR_BG : COLOR_TEXT_SECONDARY, tab1 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("2. SISTEMA & UTILS", 238, 29, 2);

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
    // --- PESTAÑA 1: ESTUDIO & DESK (6 APLICACIONES EN CUADRÍCULA 2x3) ---
    // Fila 1 (y: 52, h: 84)
    drawAppCard(8, 52, 96, 84, COLOR_CARD, 1, "Pomodoro", pomoSub);
    drawAppCard(112, 52, 96, 84, COLOR_CARD, 2, "Agenda", "UNED Avila");
    drawAppCard(216, 52, 96, 84, COLOR_CARD, 3, "Clima", wSub);

    // Fila 2 (y: 142, h: 84)
    drawAppCard(8, 142, 96, 84, COLOR_CARD, 4, "Reloj Noche", "Tenue");
    drawAppCard(112, 142, 96, 84, COLOR_CARD, 5, "Relax Box", "4-4-4-4");
    drawAppCard(216, 142, 96, 84, COLOR_CARD, 6, "Lampara", "Luz RGB");
  } else {
    // --- PESTAÑA 2: SISTEMA & AJUSTES ---
    // Fila 1 (y: 52, h: 84)
    drawAppCard(8, 52, 96, 84, COLOR_CARD, 7, "Bateria", batSub);
    drawAppCard(112, 52, 96, 84, COLOR_CARD, 8, "Stats SD", "Historial");
    drawAppCard(216, 52, 96, 84, COLOR_CARD, 9, "Ajustes", "Brillo/Audio");

    // Fila 2 (y: 142, h: 84)
    drawAppCard(8, 142, 148, 84, COLOR_CARD, 10, "Info Hardware", "Guition 2.4\"");
    drawAppCard(164, 142, 148, 84, COLOR_CARD_HL, 3, "Volver Clima", "Standby Desk");
  }
}

int UIManager::handleHomeTouch(int x, int y, int &homeTab) {
  // 1. Botón atrás (esquina superior izquierda: x: 0..64, y: 0..24)
  if (x >= 0 && x <= 64 && y >= 0 && y <= 24) {
    return -20; // Señal de retorno a Pantalla Standby (Clima)
  }

  // 2. Conmutador de Pestañas Superiores (y: 20..48)
  if (y >= 20 && y <= 48) {
    if (x >= 8 && x <= 156 && homeTab != 0) {
      homeTab = 0;
      return -10; // Redibujar
    }
    if (x >= 164 && x <= 312 && homeTab != 1) {
      homeTab = 1;
      return -10; // Redibujar
    }
  }

  // 3. Fila 1 de Aplicaciones (y: 50..138)
  if (y >= 50 && y <= 138) {
    if (homeTab == 0) {
      if (x >= 6 && x <= 106)
        return 1; // 🍅 Pomodoro
      if (x >= 110 && x <= 210)
        return 4; // 📅 Agenda UNED
      if (x >= 214 && x <= 314)
        return 8; // 🌤️ Clima
    } else {
      if (x >= 6 && x <= 106)
        return 12; // 🔋 Batería
      if (x >= 110 && x <= 210)
        return 7; // 📊 Stats
      if (x >= 214 && x <= 314)
        return 5; // ⚙️ Ajustes
    }
  }

  // 4. Fila 2 de Aplicaciones (y: 140..230)
  if (y >= 140 && y <= 230) {
    if (homeTab == 0) {
      if (x >= 6 && x <= 106)
        return 9; // 🌙 Reloj Noche
      if (x >= 110 && x <= 210)
        return 10; // 🫁 Relax Box
      if (x >= 214 && x <= 314)
        return 11; // 💡 Lámpara RGB
    } else {
      if (x >= 8 && x <= 158)
        return 6; // ℹ️ Info Hardware
      if (x >= 160 && x <= 312)
        return -20; // 🌤️ Volver a Standby
    }
  }

  return 0;
}

// ============================================================================
// 3. APLICACIÓN POMODORO CHRONO DECK
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
  _tft.fillRoundRect(8, 26, 304, 32, 4, COLOR_CARD);
  _tft.drawRoundRect(8, 26, 304, 32, 4, COLOR_BORDER);

  if (hasSession) {
    String title = session.title;
    if (title.length() > 24)
      title = title.substring(0, 22) + "..";
    _tft.fillCircle(18, 42, 3, COLOR_ACCENT_AMBER);
    _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
    _tft.drawString(session.timeRange, 26, 30, 2);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString(title, 136, 30, 2);
  } else {
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString("Modo Enfoque Libre | Sin sesion agendada hoy", 160, 36, 1);
  }

  // Selector táctil de modo (25 / 50 / 90 min)
  drawModeButtons(engine);

  // Temporizador y barra de progreso
  drawCountdown(engine);
  drawProgressBar(engine);

  // Botones de Acción: Iniciar / Pausa, Reset, Saltar
  drawActionButtons(engine);
}

void UIManager::drawModeButtons(const PomodoroEngine &engine) {
  uint16_t currentFocus = engine.getFocusMinutes();
  int btnW = 96;
  int btnH = 22;
  int startY = 64;

  int x25 = 8;
  bool sel25 = (currentFocus == 25);
  _tft.fillRoundRect(x25, startY, btnW, btnH, 3, sel25 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(x25, startY, btnW, btnH, 3, sel25 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(sel25 ? COLOR_BG : COLOR_TEXT_SECONDARY, sel25 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("25 MIN", x25 + btnW / 2, startY + 4, 2);

  int x50 = 112;
  bool sel50 = (currentFocus == 50);
  _tft.fillRoundRect(x50, startY, btnW, btnH, 3, sel50 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(x50, startY, btnW, btnH, 3, sel50 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(sel50 ? COLOR_BG : COLOR_TEXT_SECONDARY, sel50 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("50 MIN", x50 + btnW / 2, startY + 4, 2);

  int x90 = 216;
  bool sel90 = (currentFocus == 90);
  _tft.fillRoundRect(x90, startY, btnW, btnH, 3, sel90 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawRoundRect(x90, startY, btnW, btnH, 3, sel90 ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
  _tft.setTextColor(sel90 ? COLOR_BG : COLOR_TEXT_SECONDARY, sel90 ? COLOR_TEXT_PRIMARY : COLOR_CARD);
  _tft.drawCentreString("90 MIN", x90 + btnW / 2, startY + 4, 2);
}

void UIManager::drawCountdown(const PomodoroEngine &engine) {
  uint32_t s = engine.getRemainingSeconds();
  uint16_t m = s / 60;
  uint8_t sec = s % 60;

  char timeBuf[12];
  snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", m, sec);

  // Dígitos de cronómetro gigante
  _tft.fillRect(20, 92, 280, 48, COLOR_BG);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
  _tft.drawCentreString(timeBuf, 160, 94, 7);

  // Insignia flotante de estado actual
  const char *stateStr = "EN REPOSO";
  uint16_t sCol = COLOR_TEXT_MUTED;
  if (engine.isFocus()) {
    stateStr = engine.isPaused() ? "● ENFOQUE EN PAUSA" : "● ENFOQUE INTENSO";
    sCol = engine.isPaused() ? COLOR_ACCENT_AMBER : COLOR_ACCENT_CORAL;
  } else if (engine.isBreak()) {
    stateStr = engine.isPaused() ? "● DESCANSO EN PAUSA" : "● DESCANSO REPARADOR";
    sCol = engine.isPaused() ? COLOR_ACCENT_AMBER : COLOR_ACCENT_MINT;
  }
  _tft.setTextColor(sCol, COLOR_BG);
  _tft.drawCentreString(stateStr, 160, 144, 2);
}

void UIManager::drawProgressBar(const PomodoroEngine &engine) {
  int barX = 16;
  int barY = 164;
  int barW = 288;
  int barH = 6;

  _tft.fillRoundRect(barX, barY, barW, barH, 2, COLOR_CARD);
  _tft.drawRoundRect(barX, barY, barW, barH, 2, COLOR_BORDER);

  float prog = engine.getProgress();
  int fillW = (int)(prog * (barW - 2));
  if (fillW > 0) {
    uint16_t pCol = engine.isBreak() ? COLOR_ACCENT_MINT : COLOR_ACCENT_CORAL;
    _tft.fillRoundRect(barX + 1, barY + 1, fillW, barH - 2, 2, pCol);
  }
}

void UIManager::drawActionButtons(const PomodoroEngine &engine) {
  int btnY = 182;
  int btnH = 46;

  // Botón Principal: INICIAR / PAUSAR
  bool running = engine.isRunning();
  uint16_t mainBg = running ? COLOR_CARD : COLOR_ACCENT_EMERALD;
  uint16_t mainText = running ? COLOR_TEXT_PRIMARY : 0x0000;

  _tft.fillRoundRect(16, btnY, 184, btnH, 5, mainBg);
  _tft.drawRoundRect(16, btnY, 184, btnH, 5, running ? COLOR_TEXT_PRIMARY : COLOR_ACCENT_EMERALD);
  _tft.setTextColor(mainText, mainBg);
  _tft.drawCentreString(running ? "❚❚ PAUSAR" : "▶ INICIAR ESTUDIO", 108, btnY + 14, 2);

  // Botón Reset
  _tft.fillRoundRect(208, btnY, 48, btnH, 5, COLOR_CARD);
  _tft.drawRoundRect(208, btnY, 48, btnH, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString("RESET", 232, btnY + 14, 1);

  // Botón Skip (Saltar)
  _tft.fillRoundRect(262, btnY, 44, btnH, 5, COLOR_CARD);
  _tft.drawRoundRect(262, btnY, 44, btnH, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString("SKIP", 284, btnY + 14, 1);
}

void UIManager::updatePomodoroTimer(const PomodoroEngine &engine,
                                    const CalendarManager & /*calendar*/,
                                    const BatteryManager &battery,
                                    bool wifiConnected) {
  drawCountdown(engine);
  drawProgressBar(engine);
  drawUnifiedStatusBar(battery, wifiConnected, false, true,
                       engine.getCompletedSessionsToday(), "POMODORO");
}

UIButtonAction UIManager::handlePomodoroTouch(int x, int y) {
  // Botón Atrás
  if (x >= 0 && x <= 64 && y >= 0 && y <= 24) {
    return UI_ACTION_BACK_HOME;
  }

  // Presets de tiempo (y: 60..88)
  if (y >= 60 && y <= 88) {
    if (x >= 8 && x <= 104)
      return UI_ACTION_SELECT_25;
    if (x >= 112 && x <= 208)
      return UI_ACTION_SELECT_50;
    if (x >= 216 && x <= 312)
      return UI_ACTION_SELECT_90;
  }

  // Botones de acción inferiores (y: 180..232)
  if (y >= 180 && y <= 232) {
    if (x >= 16 && x <= 200)
      return UI_ACTION_PLAY_PAUSE;
    if (x >= 204 && x <= 256)
      return UI_ACTION_RESET;
    if (x >= 258 && x <= 312)
      return UI_ACTION_SKIP;
  }

  return UI_ACTION_NONE;
}

// ============================================================================
// 4. APLICACIÓN AGENDA ACADÉMICA UNED ÁVILA
// ============================================================================
void UIManager::drawAgendaView(CalendarManager &calendar,
                              const WiFiConfig & /*wifi*/,
                              const BatteryManager &battery,
                              const WeatherManager & /*weather*/) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, true, false, true, 0, "AGENDA UNED");

  CalendarViewMode mode = calendar.getViewMode();

  // Selector superior de 3 Modos: [ HOY ] [ SEMANA ] [ MES ] (y: 25..46)
  int tabW = 96;
  const char *tabs[3] = {"HOY", "SEMANA", "MES"};
  for (int i = 0; i < 3; i++) {
    int tx = 8 + i * 104;
    bool active = ((int)mode == i);
    _tft.fillRoundRect(tx, 25, tabW, 22, 3, active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawRoundRect(tx, 25, tabW, 22, 3, active ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
    _tft.setTextColor(active ? COLOR_BG : COLOR_TEXT_SECONDARY, active ? COLOR_TEXT_PRIMARY : COLOR_CARD);
    _tft.drawCentreString(tabs[i], tx + tabW / 2, 29, 2);
  }

  // --- MODO 1: VISTA DIARIA (TIMELINE DE HOY) ---
  if (mode == CAL_VIEW_DAY) {
    String examTitle;
    String countdown = calendar.getNextDeadlineCountdown(examTitle);

    // Insignia de hito de examen
    _tft.fillRoundRect(8, 52, 304, 30, 4, COLOR_CARD);
    _tft.drawRoundRect(8, 52, 304, 30, 4, COLOR_BORDER);
    _tft.fillCircle(18, 67, 4, COLOR_ACCENT_ROYAL);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawString("EXAMEN: " + examTitle.substring(0, 20), 28, 58, 2);
    _tft.setTextColor(COLOR_ACCENT_ROYAL, COLOR_CARD);
    _tft.drawRightString(countdown, 304, 58, 2);

    // Tarjeta principal con la sesión de hoy
    CalendarEvent session;
    bool hasSession = calendar.getTodayMainSession(session);
    _tft.fillRoundRect(8, 88, 304, 98, 5, COLOR_CARD);
    _tft.drawRoundRect(8, 88, 304, 98, 5, COLOR_BORDER);

    if (hasSession) {
      _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
      _tft.drawString("HORARIO DE ESTUDIO HOY:", 20, 96, 1);
      _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
      _tft.drawString(session.timeRange, 20, 110, 4);

      _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
      _tft.drawString("ASIGNATURA:", 20, 142, 1);
      _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
      _tft.drawString(session.title, 20, 156, 2);
    } else {
      _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
      _tft.drawCentreString("Hoy no hay sesiones agendadas", 160, 120, 2);
      _tft.drawCentreString("Dia libre o de repaso voluntario", 160, 144, 1);
    }

    // Botón volver al menú (y: 194..230)
    _tft.fillRoundRect(8, 194, 304, 36, 4, COLOR_CARD_HL);
    _tft.drawRoundRect(8, 194, 304, 36, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
    _tft.drawCentreString("< VOLVER AL MENU", 160, 204, 2);
  }

  // --- MODO 2: VISTA SEMANAL ---
  else if (mode == CAL_VIEW_WEEK) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    int curD = (t->tm_year > 120) ? t->tm_mday : 31;
    int curM = (t->tm_year > 120) ? (t->tm_mon + 1) : 10;
    int curY = (t->tm_year > 120) ? (t->tm_year + 1900) : 2026;
    int curWDay = (t->tm_year > 120) ? ((t->tm_wday + 6) % 7) : 5; // 0 = Lunes

    const char *dayNames[7] = {"LUN", "MAR", "MIE", "JUE", "VIE", "SAB", "DOM"};

    for (int i = 0; i < 7; i++) {
      int offset = i - curWDay;
      time_t dTime = now + (offset * 86400);
      struct tm *dTm = localtime(&dTime);
      int dNum = dTm->tm_mday;
      int dMon = dTm->tm_mon + 1;
      int dYear = dTm->tm_year + 1900;
      bool isToday = (dNum == curD && dMon == curM && dYear == curY);

      int rowY = 52 + i * 21;

      // Etiqueta del día
      _tft.fillRoundRect(8, rowY, 54, 19, 2, isToday ? COLOR_TEXT_PRIMARY : COLOR_CARD);
      _tft.drawRoundRect(8, rowY, 54, 19, 2, isToday ? COLOR_TEXT_PRIMARY : COLOR_BORDER);
      char dStr[12];
      snprintf(dStr, sizeof(dStr), "%s %02d", dayNames[i], dNum);
      _tft.setTextColor(isToday ? COLOR_BG : COLOR_TEXT_SECONDARY, isToday ? COLOR_TEXT_PRIMARY : COLOR_CARD);
      _tft.drawCentreString(dStr, 35, rowY + 3, 1);

      // Franja de sesión
      _tft.fillRoundRect(66, rowY, 246, 19, 2, COLOR_CARD);
      _tft.drawRoundRect(66, rowY, 246, 19, 2, isToday ? COLOR_ACCENT_CYAN : COLOR_BORDER);

      String sum = calendar.getEventSummaryForDate(dNum, dMon, dYear);
      if (sum.length() > 0) {
        if (sum.length() > 30)
          sum = sum.substring(0, 28) + "..";
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
        _tft.drawString(sum, 72, rowY + 3, 1);
      } else {
        _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
        _tft.drawString(isToday ? "Hoy sin sesion agendada" : "-- Libre --", 72, rowY + 3, 1);
      }
    }

    // Botón volver
    _tft.fillRoundRect(8, 204, 304, 28, 4, COLOR_CARD);
    _tft.drawRoundRect(8, 204, 304, 28, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
    _tft.drawCentreString("< VOLVER AL MENU", 160, 210, 2);
  }

  // --- MODO 3: VISTA DE MES ENTERO ---
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
        "ENERO", "FEBRERO", "MARZO", "ABRIL", "MAYO", "JUNIO",
        "JULIO", "AGOSTO", "SEPTIEMBRE", "OCTUBRE", "NOVIEMBRE", "DICIEMBRE"};

    // Botón mes anterior
    _tft.fillRoundRect(8, 52, 28, 18, 3, COLOR_CARD);
    _tft.drawRoundRect(8, 52, 28, 18, 3, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("<", 22, 53, 2);

    // Título Mes y Año
    char titleBuf[32];
    snprintf(titleBuf, sizeof(titleBuf), "%s %d", mesesFull[dispM - 1], dispY);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
    _tft.drawCentreString(titleBuf, 160, 52, 2);

    // Botón mes siguiente
    _tft.fillRoundRect(284, 52, 28, 18, 3, COLOR_CARD);
    _tft.drawRoundRect(284, 52, 28, 18, 3, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString(">", 298, 53, 2);

    // Cabeceras de días de la semana
    const char *wNames[7] = {"L", "M", "X", "J", "V", "S", "D"};
    for (int col = 0; col < 7; col++) {
      int cx = 10 + col * 43 + 21;
      _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_BG);
      _tft.drawCentreString(wNames[col], cx, 74, 1);
    }
    _tft.drawFastHLine(8, 84, 304, COLOR_SEPARATOR);

    int daysInM = 31;
    if (dispM == 2) {
      daysInM = (dispY % 4 == 0 && (dispY % 100 != 0 || dispY % 400 == 0)) ? 29 : 28;
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
      int cellX = 8 + col * 43;
      int cellY = 88 + row * 17;

      bool isToday = (dispY == curY && dispM == curM && day == curD);
      bool hasEvent = calendar.hasEventOnDate(day, dispM, dispY);
      bool hasExam = calendar.hasDeadlineOnDate(day, dispM, dispY);

      if (isToday) {
        // Enmarcado destacado de HOY: cápsula blanca con halo cian
        _tft.fillRoundRect(cellX + 7, cellY - 1, 28, 16, 4, COLOR_TEXT_PRIMARY);
        _tft.drawRoundRect(cellX + 6, cellY - 2, 30, 18, 4, COLOR_ACCENT_CYAN);
        _tft.setTextColor(COLOR_BG, COLOR_TEXT_PRIMARY);
      } else {
        _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_BG);
      }

      _tft.drawCentreString(String(day), cellX + 21, cellY + 1, 1);

      if (hasEvent || hasExam) {
        uint16_t dotCol = hasExam ? COLOR_ACCENT_CORAL : (isToday ? COLOR_BG : COLOR_ACCENT_CYAN);
        _tft.drawFastHLine(cellX + 16, cellY + 13, 10, dotCol);
      }

      col++;
      if (col > 6) {
        col = 0;
        row++;
      }
    }

    // Botón volver
    _tft.fillRoundRect(8, 198, 304, 34, 4, COLOR_CARD);
    _tft.drawRoundRect(8, 198, 304, 34, 4, COLOR_BORDER);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("< VOLVER AL MENU", 160, 206, 2);
  }
}

void UIManager::updateAgendaClock(CalendarManager &calendar,
                                  const WeatherManager & /*weather*/) {
  if (calendar.getViewMode() != CAL_VIEW_DAY)
    return;
}

bool UIManager::handleAgendaTouch(int x, int y, CalendarManager &calendar,
                                  bool *outAddExam, bool *outSyncGCal,
                                  bool *outBack) {
  *outAddExam = false;
  *outSyncGCal = false;
  *outBack = false;

  // Botón Atrás
  if (x >= 0 && x <= 64 && y >= 0 && y <= 24) {
    *outBack = true;
    return true;
  }

  // Conmutador de 3 Modos: HOY, SEMANA, MES (y: 24..48)
  if (y >= 24 && y <= 48) {
    if (x >= 8 && x <= 104) {
      calendar.setViewMode(CAL_VIEW_DAY);
      return true;
    }
    if (x >= 112 && x <= 208) {
      calendar.setViewMode(CAL_VIEW_WEEK);
      return true;
    }
    if (x >= 216 && x <= 312) {
      calendar.setViewMode(CAL_VIEW_MONTH);
      return true;
    }
  }

  // Navegación en vista de mes
  if (calendar.getViewMode() == CAL_VIEW_MONTH) {
    if (y >= 48 && y <= 72) {
      if (x >= 8 && x <= 40) {
        calendar.changeMonth(-1);
        return true;
      }
      if (x >= 280 && x <= 314) {
        calendar.changeMonth(1);
        return true;
      }
    }
    if (y >= 194 && y <= 236) {
      *outBack = true;
      return true;
    }
  }

  // Botón volver en modo día y semana
  if (y >= 194 && y <= 236) {
    *outBack = true;
    return true;
  }

  return false;
}

// ============================================================================
// 5. APLICACIÓN DEDICADA: RELOJ NOCTURNO (NIGHT DESK CLOCK)
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
  _tft.setTextColor(0x7800, COLOR_BG); // Rojo suave tenue (anti-fatiga nocturna)
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

bool UIManager::handleNightClockTouch(int /*x*/, int /*y*/) {
  return true; // Cualquier toque sale de reloj nocturno
}

// ============================================================================
// 6. APLICACIÓN DEDICADA: RESPIRACIÓN CONSCIENTE (BOX BREATHING 4-4-4-4)
// ============================================================================
void UIManager::drawBreathingView(int phase, int secondsLeft, int cycle,
                                  const BatteryManager &battery) {
  _tft.fillScreen(COLOR_BG);
  drawUnifiedStatusBar(battery, false, false, false, 0, "RELAX BOX");

  const char *phaseNames[4] = {"INHALA", "MANTEN", "EXHALA", "PAUSA"};
  const uint16_t phaseColors[4] = {
      COLOR_ACCENT_MINT,  // Inhala: Menta zen
      COLOR_ACCENT_CYAN,  // Mantén: Cian enfoque
      0x435F,             // Exhala: Azul relajante
      COLOR_TEXT_MUTED    // Pausa: Grafito suave
  };
  uint16_t currentPhaseColor = phaseColors[phase % 4];

  _tft.setTextColor(currentPhaseColor, COLOR_BG);
  _tft.drawCentreString(phaseNames[phase % 4], 160, 34, 4);

  int cx = 160, cy = 115, sz = 44;
  _tft.drawRoundRect(cx - sz, cy - sz, sz * 2, sz * 2, 8, COLOR_BORDER);
  _tft.fillCircle(cx, cy, 26, COLOR_CARD);

  // Anillo de respiración concéntrico de 3px de grosor
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

  // Botón salir
  _tft.fillRoundRect(16, 196, 288, 36, 5, COLOR_CARD);
  _tft.drawRoundRect(16, 196, 288, 36, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("FINALIZAR RESPIRACION", 160, 206, 2);
}

bool UIManager::handleBreathingTouch(int x, int y) {
  if (x >= 0 && x <= 64 && y >= 0 && y <= 24)
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
      0xFFF0, // 1. BLANCO CALIDO (marfil ámbar)
      COLOR_ACCENT_CYAN, // 2. CIAN ENFOQUE
      0x223F, // 3. AZUL NOCTURNO
      COLOR_ACCENT_CORAL // 4. EFECTO VELA
  };

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_BG);
  _tft.drawCentreString("Selecciona el tono de iluminacion ambiental:", 160, 28, 2);

  for (int i = 0; i < 5; i++) {
    int y = 48 + i * 35;
    bool active = (currentMode == i);

    _tft.fillRoundRect(16, y, 288, 30, 4, active ? COLOR_CARD_HL : COLOR_CARD);
    _tft.drawRoundRect(16, y, 288, 30, 4, active ? COLOR_TEXT_PRIMARY : COLOR_BORDER);

    // Muestra de color (swatch)
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
  if (x >= 0 && x <= 64 && y >= 0 && y <= 24) {
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
  drawUnifiedStatusBar(battery, wifiConnected, false, true, 0, "BATERIA 5000 mAh");

  int pct = battery.getPercentage();
  float v = battery.getVoltage();
  bool charging = battery.isCharging();
  float hrs = battery.getEstimatedHoursLeft();

  _tft.fillRoundRect(16, 32, 288, 140, 6, COLOR_CARD);
  _tft.drawRoundRect(16, 32, 288, 140, 6, COLOR_BORDER);

  drawAppColorIcon(50, 75, 7);

  // Dígitos grandes con símbolo %
  char pctStr[16];
  snprintf(pctStr, sizeof(pctStr), "%d", pct);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  int numW = _tft.drawString(pctStr, 80, 52, 6);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("%", 80 + numW + 3, 62, 4);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(charging ? "Cargando LiPo..." : "Descarga normal", 80, 96, 2);

  // Barra de progreso analógica estilo gadget
  int barW = 256;
  _tft.fillRoundRect(32, 118, barW, 10, 3, COLOR_BG);
  _tft.drawRoundRect(32, 118, barW, 10, 3, COLOR_BORDER);
  int fW = map(constrain(pct, 0, 100), 0, 100, 0, barW - 2);
  if (fW > 0) {
    _tft.fillRoundRect(33, 119, fW, 8, 2, COLOR_ACCENT_EMERALD);
  }

  // Telemetría de voltaje y energía
  char infoBuf[48];
  if (v < 0.5f) {
    snprintf(infoBuf, sizeof(infoBuf), "Alimentacion USB (5.0V)  |  18.5 Wh");
  } else {
    snprintf(infoBuf, sizeof(infoBuf), "%.2f V  |  ~%.1f h restantes  |  18.5 Wh", v, hrs);
  }
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawCentreString(infoBuf, 160, 144, 2);

  // Tarjeta de hardware inferior
  _tft.fillRoundRect(16, 184, 288, 44, 5, COLOR_CARD);
  _tft.drawRoundRect(16, 184, 288, 44, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawCentreString("Cargador USB-C integrado | Proteccion LiPo DW01", 160, 200, 1);
}

bool UIManager::handleBatteryTouch(int x, int y) {
  if (x >= 0 && x <= 64 && y >= 0 && y <= 24)
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

  // Tarjeta 1: Sesiones Hoy
  _tft.fillRoundRect(16, 32, 138, 70, 5, COLOR_CARD);
  _tft.drawRoundRect(16, 32, 138, 70, 5, COLOR_BORDER);
  _tft.fillCircle(24, 42, 3, COLOR_ACCENT_CORAL);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("SESIONES HOY", 32, 38, 1);
  char sBuf[12];
  snprintf(sBuf, sizeof(sBuf), "%d", sessionsToday);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(sBuf, 24, 52, 6);

  // Tarjeta 2: Tiempo Total
  _tft.fillRoundRect(166, 32, 138, 70, 5, COLOR_CARD);
  _tft.drawRoundRect(166, 32, 138, 70, 5, COLOR_BORDER);
  _tft.fillCircle(174, 42, 3, COLOR_ACCENT_CYAN);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("TIEMPO TOTAL", 182, 38, 1);
  char mBuf[16];
  snprintf(mBuf, sizeof(mBuf), "%dh %02dm", focusMinToday / 60, focusMinToday % 60);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(mBuf, 174, 58, 4);

  // Tarjeta 3: Registro en Tarjeta MicroSD
  _tft.fillRoundRect(16, 110, 288, 76, 5, COLOR_CARD);
  _tft.drawRoundRect(16, 110, 288, 76, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawString("REGISTRO EN TARJETA MICROSD", 24, 116, 1);

  char lBuf[64];
  snprintf(lBuf, sizeof(lBuf), "Total acumulado: %d sesiones guardadas", totalLogged);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString(lBuf, 24, 134, 2);

  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Archivo: /sessions.csv (Excel / Python)", 24, 158, 1);

  // Botón volver
  _tft.fillRoundRect(16, 194, 288, 36, 5, COLOR_CARD);
  _tft.drawRoundRect(16, 194, 288, 36, 5, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("< VOLVER AL MENU", 160, 204, 2);
}

bool UIManager::handleStatsTouch(int x, int y) {
  if (x >= 0 && x <= 64 && y >= 0 && y <= 24)
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
                       0, "AJUSTES SISTEMA");

  int y = 30;

  // 1. Brillo de Pantalla (PWM)
  _tft.fillRoundRect(14, y, 292, 42, 4, COLOR_CARD);
  _tft.drawRoundRect(14, y, 292, 42, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("Brillo de Pantalla", 24, y + 6, 2);
  char bBuf[24];
  snprintf(bBuf, sizeof(bBuf), "%d%%  %s", (brightness * 100) / 255,
           autoBrightness ? "(LDR AUTO)" : "(MANUAL)");
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(bBuf, 24, y + 24, 1);

  // Botones [ - ] [ + ]
  _tft.fillRoundRect(200, y + 6, 44, 30, 3, COLOR_CARD_HL);
  _tft.drawRoundRect(200, y + 6, 44, 30, 3, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString("-", 222, y + 10, 4);

  _tft.fillRoundRect(252, y + 6, 44, 30, 3, COLOR_CARD_HL);
  _tft.drawRoundRect(252, y + 6, 44, 30, 3, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString("+", 274, y + 10, 4);

  y += 46;

  // 2. Audio Zumbador SC8002B
  _tft.fillRoundRect(14, y, 292, 42, 4, COLOR_CARD);
  _tft.drawRoundRect(14, y, 292, 42, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("Sonido / Alarma Estudio", 24, y + 6, 2);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(soundMuted ? "Silenciado (MUTE)" : "Activo (3s Alarma)", 24, y + 24, 1);

  _tft.fillRoundRect(200, y + 6, 96, 30, 3, soundMuted ? COLOR_CARD : COLOR_ACCENT_EMERALD);
  _tft.drawRoundRect(200, y + 6, 96, 30, 3, soundMuted ? COLOR_BORDER : COLOR_ACCENT_EMERALD);
  _tft.setTextColor(soundMuted ? COLOR_TEXT_PRIMARY : 0x0000, soundMuted ? COLOR_CARD : COLOR_ACCENT_EMERALD);
  _tft.drawCentreString(soundMuted ? "MUTED" : "ACTIVO", 248, y + 12, 2);

  y += 46;

  // 3. Tiempo de Reposo (Inactividad)
  _tft.fillRoundRect(14, y, 292, 42, 4, COLOR_CARD);
  _tft.drawRoundRect(14, y, 292, 42, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("Reposo a Clima", 24, y + 6, 2);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString("Retorno a Standby", 24, y + 24, 1);

  char tBuf[16];
  if (timeoutMinutes == 0) {
    snprintf(tBuf, sizeof(tBuf), "NUNCA");
  } else {
    snprintf(tBuf, sizeof(tBuf), "%d MIN", timeoutMinutes);
  }
  _tft.fillRoundRect(200, y + 6, 96, 30, 3, COLOR_CARD_HL);
  _tft.drawRoundRect(200, y + 6, 96, 30, 3, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD_HL);
  _tft.drawCentreString(tBuf, 248, y + 12, 2);

  y += 46;

  // 4. Configurar Redes Wi-Fi
  _tft.fillRoundRect(14, y, 292, 42, 4, COLOR_CARD);
  _tft.drawRoundRect(14, y, 292, 42, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawString("Redes Wi-Fi 2.4 GHz", 24, y + 6, 2);
  _tft.setTextColor(COLOR_TEXT_SECONDARY, COLOR_CARD);
  _tft.drawString(wifi.isConnected() ? wifi.getSSID().c_str() : "Desconectado", 24, y + 24, 1);

  _tft.fillRoundRect(200, y + 6, 96, 30, 3, COLOR_CARD_HL);
  _tft.drawRoundRect(200, y + 6, 96, 30, 3, COLOR_BORDER);
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD_HL);
  _tft.drawCentreString("CONECTAR", 248, y + 12, 2);
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

  if (x >= 0 && x <= 64 && y >= 0 && y <= 24) {
    *outBack = true;
    return 5;
  }

  // Fila 1: Brillo
  if (y >= 30 && y <= 72) {
    if (x >= 195 && x <= 245) {
      *outBriDown = true;
      return 1;
    }
    if (x >= 248 && x <= 300) {
      *outBriUp = true;
      return 1;
    }
    if (x >= 14 && x <= 190) {
      *outToggleAuto = true;
      return 1;
    }
  }

  // Fila 2: Sonido Mute
  if (y >= 76 && y <= 118 && x >= 195 && x <= 300) {
    *outToggleMute = true;
    return 2;
  }

  // Fila 3: Timeout Reposo
  if (y >= 122 && y <= 164 && x >= 195 && x <= 300) {
    *outTimeoutToggle = true;
    return 3;
  }

  // Fila 4: Abrir Wi-Fi
  if (y >= 168 && y <= 210 && x >= 195 && x <= 300) {
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
  drawUnifiedStatusBar(battery, wifiConnected, false, true, 0, "INFO STUDY DECK");

  _tft.fillRoundRect(14, 28, 292, 174, 5, COLOR_CARD);
  _tft.drawRoundRect(14, 28, 292, 174, 5, COLOR_BORDER);

  int y = 34;
  _tft.setTextColor(COLOR_ACCENT_AMBER, COLOR_CARD);
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
  _tft.drawString("MicroSD: " + String(storage.isAvailable() ? "Detectada (FAT32)" : "No detectada"), 24, y, 2);
  y += 18;
  _tft.drawString("Firmware: v2.0.0 Cyber-Studio OS", 24, y, 2);
  y += 18;
  _tft.drawString("Grado en Ingenieria Informatica", 24, y, 2);

  // Botón volver
  _tft.fillRoundRect(14, 206, 292, 28, 4, COLOR_CARD);
  _tft.drawRoundRect(14, 206, 292, 28, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("< VOLVER AL MENU PRINCIPAL", 160, 212, 2);
}

bool UIManager::handleInfoTouch(int x, int y) {
  if (x >= 0 && x <= 64 && y >= 0 && y <= 24)
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
  drawUnifiedStatusBar(battery, wifi.isConnected(), false, true, 0, "REDES WI-FI");

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
  const int netsPerPage = 4;
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
      snprintf(inf, sizeof(inf), "%s %d dBm", open ? "[ABI]" : "[SEG]", (int)rssi);
      _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
      _tft.drawRightString(inf, 300, y + 10, 1);
    }
  }

  // Botonera inferior
  _tft.fillRoundRect(12, 178, 142, 34, 4, COLOR_CARD);
  _tft.drawRoundRect(12, 178, 142, 34, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("REESCANEAR", 83, 187, 2);

  int maxPage = (totalNets > 0) ? ((totalNets - 1) / netsPerPage) : 0;
  char pagStr[32];
  snprintf(pagStr, sizeof(pagStr), "PAG %d/%d >", page + 1, maxPage + 1);

  _tft.fillRoundRect(166, 178, 142, 34, 4, COLOR_CARD);
  _tft.drawRoundRect(166, 178, 142, 34, 4, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString(pagStr, 237, 187, 2);

  _tft.fillRoundRect(12, 216, 296, 20, 2, COLOR_CARD);
  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawCentreString("Toca una red para conectarte | Boton ATRAS para salir", 160, 220, 1);
}

int UIManager::handleWiFiScanTouch(int x, int y, int netCount, int page,
                                   bool *outRescan, bool *outBack,
                                   bool *outNextPage) {
  *outRescan = false;
  *outBack = false;
  *outNextPage = false;

  if (x >= 0 && x <= 64 && y >= 0 && y <= 24) {
    *outBack = true;
    return -1;
  }

  // Selección de lista
  const int netsPerPage = 4;
  int startIndex = page * netsPerPage;
  for (int i = 0; i < netsPerPage; i++) {
    int rowY = 28 + i * 36;
    if (y >= rowY && y <= rowY + 32) {
      int chosen = startIndex + i;
      if (chosen < netCount) {
        return chosen;
      }
    }
  }

  // Botones inferiores
  if (y >= 174 && y <= 214) {
    if (x >= 12 && x <= 154) {
      *outRescan = true;
      return -1;
    }
    if (x >= 166 && x <= 308) {
      *outNextPage = true;
      return -1;
    }
  }

  return -1;
}

void UIManager::drawWiFiConnecting(const String &ssid) {
  _tft.fillScreen(COLOR_BG);
  _tft.fillRoundRect(20, 70, 280, 100, 6, COLOR_CARD);
  _tft.drawRoundRect(20, 70, 280, 100, 6, COLOR_BORDER);
  _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
  _tft.drawCentreString("Conectando a red Wi-Fi...", 160, 85, 2);

  String displaySSID = (ssid.length() > 22) ? ssid.substring(0, 20) + ".." : ssid;
  _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
  _tft.drawCentreString(displaySSID, 160, 110, 4);

  _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
  _tft.drawCentreString("Negociando DHCP y WPA2...", 160, 142, 1);
}

void UIManager::drawWiFiResult(bool success, const String &info) {
  _tft.fillScreen(COLOR_BG);
  _tft.fillRoundRect(20, 60, 280, 120, 6, COLOR_CARD);
  _tft.drawRoundRect(20, 60, 280, 120, 6, success ? COLOR_ACCENT_EMERALD : COLOR_ACCENT_CORAL);

  if (success) {
    _tft.setTextColor(COLOR_ACCENT_EMERALD, COLOR_CARD);
    _tft.drawCentreString("CONEXION EXITOSA", 160, 75, 4);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString("IP asignada por DHCP:", 160, 105, 2);
    _tft.setTextColor(COLOR_ACCENT_CYAN, COLOR_CARD);
    _tft.drawCentreString(info, 160, 125, 4);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawCentreString("Sincronizando hora NTP y Google Calendar...", 160, 155, 1);
  } else {
    _tft.setTextColor(COLOR_ACCENT_CORAL, COLOR_CARD);
    _tft.drawCentreString("FALLO DE CONEXION", 160, 80, 4);
    _tft.setTextColor(COLOR_TEXT_PRIMARY, COLOR_CARD);
    _tft.drawCentreString(info, 160, 115, 2);
    _tft.setTextColor(COLOR_TEXT_MUTED, COLOR_CARD);
    _tft.drawCentreString("Verifique la clave de red ingresada", 160, 145, 1);
  }
}
