#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

#include <Arduino.h>

// ============================================================================
// CONFIGURACIÓN OFICIAL DE PINES - GUITION ESP32-2432S024 (CYD 2.4" CAPACITIVA)
// ============================================================================

// --- PANTALLA TFT LCD (ILI9341 - SPI) ---
// Los pines SPI están gestionados por la librería TFT_eSPI (User_Setup.h):
// MOSI = 13, MISO = 12, SCLK = 14, CS = 15, DC = 2, RST = EN (-1)
#define SCREEN_WIDTH      320
#define SCREEN_HEIGHT     240
#define SCREEN_ROTATION   1    // 1 = Horizontal apaisado (Landscape)

// --- RETROILUMINACIÓN (PWM) ---
#define PIN_BACKLIGHT     27   // Puerta del MOSFET AO3402
#define PWM_CHANNEL_BL    0
#define PWM_FREQ_BL       5000
#define PWM_RES_BL        8    // 8 bits (0 - 255)

// --- PANEL TÁCTIL CAPACITIVO (CST820 - I2C) ---
#define I2C_SDA_PIN       33
#define I2C_SCL_PIN       32
#define TP_RST_PIN        25
#define TP_INT_PIN        21
#define CST820_I2C_ADDR   0x15

// --- LED RGB DE ESTADO (Cátodo común / Activo a nivel BAJO) ---
#define PIN_LED_RED       4
#define PIN_LED_GREEN     16
#define PIN_LED_BLUE      17

// --- SENSOR DE LUZ AMBIENTAL (LDR) ---
#define PIN_LDR           34   // Entrada analógica ADC1

// --- MONITOR DE BATERÍA (ADC1) ---
#define PIN_BATTERY       35   // Entrada analógica ADC1 (Divisor de tensión LiPo 3.7V/4.2V)

// --- AUDIO / ZUMBADOR ---
#define PIN_BUZZER        26   // Hacia amplificador SC8002B

// --- TARJETA MICROSD (VSPI) ---
#define PIN_SD_CS         5
#define PIN_SD_MOSI       23
#define PIN_SD_CLK        18
#define PIN_SD_MISO       19

#endif // HARDWARE_CONFIG_H

