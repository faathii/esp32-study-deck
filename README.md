# 📚 ESP32 Study Deck — Estación de Estudio Inteligente

Dispositivo de escritorio inteligente dedicado exclusivamente al enfoque, productividad, administración del tiempo de estudio y gamificación de hábitos académicos.

Construido sobre la placa **Guition ESP32-2432S024** (variante *Cheap Yellow Display* de 2.4 pulgadas con **táctil capacitivo CST820**).

---

## 🛠️ Especificaciones del Hardware

* **Módulo:** Guition ESP32-2432S024 (CYD 2.4" Capacitive)
* **MCU:** ESP32-WROOM-32 (Dual Core 240 MHz, Wi-Fi 802.11 b/g/n, Bluetooth v4.2 BR/EDR y BLE)
* **Pantalla:** TFT LCD 2.4" a color (240 × 320 px, controlador ILI9341 en bus SPI)
* **Panel Táctil:** Capacitivo multitáctil I2C (controlador **CST820**)
* **LED de Estado:** RGB de alta intensidad (Cátodo común / Lógica activa en bajo)
* **Audio:** Amplificador SC8002B con zumbador/altavoz integrado
* **Sensores:** Fotorresistencia LDR (GT36516) para brillo ambiental adaptativo
* **Almacenamiento:** Ranura MicroSD integrada (bus SPI independiente)

---

## 📌 Pinout Oficial Verificado (PCB v1.0)

| Función | Componente | Pines ESP32 | Notas |
| :--- | :--- | :--- | :--- |
| **Pantalla TFT** | ILI9341 | `MOSI: 13`, `MISO: 12`, `SCK: 14`, `CS: 15`, `DC: 2` | Bus HSPI compartido |
| **Iluminación** | Backlight | `GPIO 27` | Control PWM vía MOSFET AO3402 |
| **Táctil Capacitivo**| CST820 | `SDA: 33`, `SCL: 32`, `RST: 25`, `INT: 21` | Bus I2C en dirección `0x15` |
| **LED RGB** | Cátodo común | `Rojo: 4`, `Verde: 16`, `Azul: 17` | Activo a nivel BAJO (`LOW = ON`) |
| **Sensor de Luz** | LDR | `GPIO 34` | Entrada analógica ADC1 |
| **Audio / Tono** | SC8002B | `GPIO 26` | Control de tonos y beeps de aviso |
| **MicroSD** | TF Slot | `CS: 5`, `MOSI: 23`, `SCK: 18`, `MISO: 19` | Bus VSPI dedicado |

---

## 🎯 Funcionalidades Principales

1. **Gestión del Tiempo & Pomodoro:**
   - Temporizador de enfoque con selector de bloques (25m / 50m / descansos de 5m y 15m).
   - Semáforo visual "No Molestar" mediante el LED RGB (🔴 Rojo = Estudiando, 🟢 Verde = Descanso, 🔵 Azul = Pausa).
   - Alertas sonoras discretas al inicio y fin de cada intervalo.

2. **Agenda & Planificación:**
   - Sincronización con **Google Calendar** para visualizar clases y exámenes del día.
   - Micro lista de tareas (To-Do) interactiva para marcar hitos cumplidos directamente en la pantalla.
   - Cuenta atrás para fechas de entrega y exámenes.

3. **Gamificación del Estudio:**
   - Contador de rachas diarias y registro de bloques completados.
   - Modo **Flashcards táctil** para repaso activo de conceptos o fórmulas.
   - Guardado automático de estadísticas de estudio en tarjeta MicroSD.

4. **Entorno de Escritorio:**
   - Reloj sincronizado vía NTP por Wi-Fi.
   - Widget meteorológico en tiempo real.
   - Regulación automática del brillo mediante sensor LDR para evitar fatiga visual.

---

## 🚀 Estructura del Repositorio

* `/test_hardware`: Sketch de diagnóstico inicial para verificar el correcto funcionamiento de pantalla, táctil capacitivo CST820, LED RGB, LDR y zumbador.
* `/study_deck`: Firmware principal de la estación de estudio.
* `PROYECTO_ESTUDIO.md`: Hoja de ruta completa y arquitectura del proyecto.

---

## 📦 Compilación y Carga

Compilado usando **Arduino CLI** con el núcleo oficial ESP32 de Espressif:

```bash
# Compilar
arduino-cli compile --fqbn esp32:esp32:esp32 ./test_hardware

# Subir al dispositivo (ejemplo COM5)
arduino-cli upload -p COM5 --fqbn esp32:esp32:esp32 ./test_hardware
```
