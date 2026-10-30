#include <Arduino.h>
#include <TFT_eSPI.h>
#include "CST820.h"

// Pines oficiales de la Guition ESP32-2432S024
#define PIN_LED_R      4
#define PIN_LED_G     16
#define PIN_LED_B     17
#define PIN_LDR       34
#define PIN_BUZZER    26
#define PIN_BACKLIGHT 27

// Pines I2C del táctil capacitivo CST820
#define I2C_SDA 33
#define I2C_SCL 32
#define TP_RST  25
#define TP_INT  21

TFT_eSPI tft = TFT_eSPI();
CST820 touch(I2C_SDA, I2C_SCL, TP_RST, TP_INT);

bool cst_ok = false;
int led_state = 0;
unsigned long last_ldr_read = 0;

void setRgbLed(bool r, bool g, bool b) {
  digitalWrite(PIN_LED_R, !r);
  digitalWrite(PIN_LED_G, !g);
  digitalWrite(PIN_LED_B, !b);
}

void beep() {
  tone(PIN_BUZZER, 2000, 80);
}

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("\n\n>>> INICIANDO TEST CAPACITIVO GUITION ESP32-2432S024C <<<");

  // Retroiluminación (exclusivamente GPIO 27, sin interferir con GPIO 21)
  pinMode(PIN_BACKLIGHT, OUTPUT);
  digitalWrite(PIN_BACKLIGHT, HIGH);

  // Periféricos
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);
  setRgbLed(false, false, false);

  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_BUZZER, OUTPUT);

  // Inicializar Pantalla
  tft.begin();
  tft.setRotation(1); // Modo Horizontal 320x240
  tft.fillScreen(TFT_BLACK);

  // Inicializar Táctil Capacitivo CST820
  cst_ok = touch.begin();

  // Encabezado
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("ESTACION DE ESTUDIO", 160, 10, 4);

  if (cst_ok) {
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawCentreString("Tactil Capacitivo CST820: ONLINE", 160, 38, 2);
  } else {
    tft.setTextColor(TFT_RED, TFT_BLACK);
    tft.drawCentreString("Tactil Capacitivo: NO DETECTADO (0x15)", 160, 38, 2);
  }

  // Marco de dibujo táctil
  tft.drawRoundRect(10, 60, 195, 168, 6, TFT_DARKGREY);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Zona Tactil (Toca con el dedo)", 20, 68, 2);

  // Botón para probar LED / Buzzer
  tft.fillRoundRect(215, 60, 95, 65, 8, TFT_BLUE);
  tft.drawRoundRect(215, 60, 95, 65, 8, TFT_WHITE);
  tft.setTextColor(TFT_WHITE, TFT_BLUE);
  tft.drawCentreString("BOTON", 262, 75, 2);
  tft.drawCentreString("LED/BEEP", 262, 97, 2);

  // Tarjeta LDR
  tft.drawRoundRect(215, 135, 95, 93, 8, TFT_NAVY);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawString("Sensor Luz", 222, 145, 2);
  tft.drawString("LDR:", 222, 168, 2);

  // Beep de bienvenida y parpadeo verde
  beep();
  setRgbLed(false, true, false);
  delay(150);
  setRgbLed(false, false, false);
}

void loop() {
  uint16_t rawX = 0, rawY = 0;
  uint8_t gesture = 0;

  if (touch.getTouch(&rawX, &rawY, &gesture)) {
    // Mapeo de coordenadas según rotación 1 (horizontal 320x240)
    // CST820 en vertical nativo: X: 0..240, Y: 0..320
    int screenX = rawY;
    int screenY = 240 - rawX;

    Serial.printf("Touch RAW: X=%d, Y=%d | Pantalla: X=%d, Y=%d\n", rawX, rawY, screenX, screenY);

    // Mostrar coordenadas en pantalla
    tft.fillRect(20, 90, 175, 20, TFT_BLACK);
    tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK);
    tft.setCursor(20, 90, 2);
    tft.printf("X=%d  Y=%d", screenX, screenY);

    // Si toca dentro de la zona de dibujo
    if (screenX >= 12 && screenX <= 200 && screenY >= 62 && screenY <= 224) {
      tft.fillCircle(screenX, screenY, 4, TFT_GREEN);
    }
    // Si toca el botón derecho
    else if (screenX >= 215 && screenX <= 310 && screenY >= 60 && screenY <= 125) {
      beep();
      led_state = (led_state + 1) % 4;
      if (led_state == 0) setRgbLed(false, false, false);
      else if (led_state == 1) setRgbLed(true, false, false);
      else if (led_state == 2) setRgbLed(false, true, false);
      else if (led_state == 3) setRgbLed(false, false, true);
      delay(150);
    }
  }

  // Lectura periódica de luz LDR
  if (millis() - last_ldr_read > 400) {
    last_ldr_read = millis();
    int ldr_val = analogRead(PIN_LDR);
    tft.fillRect(222, 190, 80, 25, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawNumber(ldr_val, 222, 190, 2);
  }
}
