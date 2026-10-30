# 🔌 Mapeo Oficial de Hardware y Pinout

Este documento detalla la conexión interna de cada periférico en la placa de desarrollo **Guition ESP32-2432S024** (comúnmente conocida dentro de la familia *Cheap Yellow Display* de 2.4 pulgadas).

---

## 1. Pantalla TFT LCD (2.4" ILI9341 - 240×320)

La pantalla utiliza el bus hardware SPI (HSPI) del ESP32:

| Señal | Pin ESP32 | Función |
| :--- | :--- | :--- |
| **TFT_SCLK** | `GPIO 14` | Reloj de transmisión SPI |
| **TFT_MOSI** | `GPIO 13` | Datos hacia la pantalla (SDI) |
| **TFT_MISO** | `GPIO 12` | Lectura de datos (SDO) vía R24 (0Ω) |
| **TFT_CS** | `GPIO 15` | Selección de chip (activo en bajo) |
| **TFT_DC** | `GPIO 2` | Selección Datos / Comando (RS) |
| **TFT_RST** | *EN / RST* | Conectado al botón de reset del ESP32 |
| **TFT_BL** | `GPIO 27` | Puerta del transistor MOSFET N AO3402 |

> ⚠️ **Importante:** La retroiluminación se controla a través del **GPIO 27** con PWM. El **GPIO 21 NO es retroiluminación** en este modelo (está asignado a la interrupción del táctil y conector de sensores).

---

## 2. Panel Táctil Capacitivo (CST820 — Variante -024C)

El controlador capacitivo se comunica a través de $I^2C$ en la dirección esclava `0x15`:

| Señal | Pin ESP32 | Circuito en PCB |
| :--- | :--- | :--- |
| **CTP_SDA** | `GPIO 33` | Resistencia pull-up R23 (4.7 kΩ) a 3.3V |
| **CTP_SCL** | `GPIO 32` | Resistencia pull-up R22 (4.7 kΩ) a 3.3V |
| **CTP_RST** | `GPIO 25` | Línea de reset por hardware |
| **CTP_INT** | `GPIO 21` | Línea de interrupción de toque vía R25 (0Ω) |

---

## 3. Periféricos Integrados

### LED RGB
* **Lógica:** Cátodo común / Lógica activa en BAJO (`LOW` enciende el LED).
* **Rojo:** `GPIO 4` (con resistencia limitadora de 1 kΩ)
* **Verde:** `GPIO 16` (con resistencia limitadora de 1 kΩ)
* **Azul:** `GPIO 17` (con resistencia limitadora de 1 kΩ)

### Sensor de Luz Ambiental (LDR)
* **Pin:** `GPIO 34` (Entrada puramente analógica ADC1).
* **Circuito:** Divisor de tensión formado por R15 (1 MΩ), R19 (1 MΩ) y la fotorresistencia GT36516.

### Monitor de Batería LiPo
* **Pin:** `GPIO 35` (Entrada puramente analógica ADC1).
* **Circuito:** Divisor de tensión para medir la tensión de la celda LiPo (3.0V – 4.2V) conectada al puerto JST de 1.25 mm y gestionada por el chip de carga IP5603. Permite cálculo no bloqueante de porcentaje (0% - 100%) y detección de carga por USB.

### Audio / Zumbador
* **Pin:** `GPIO 26`
* **Circuito:** Filtro pasa-bajos hacia el amplificador SC8002B que alimenta el zumbador/altavoz y el conector P4 JST.

### Ranura MicroSD (TF Card)
* **CS:** `GPIO 5`
* **MOSI:** `GPIO 23`
* **SCK:** `GPIO 18`
* **MISO:** `GPIO 19`
* Opera en el bus VSPI independiente, evitando colisiones con la pantalla.

