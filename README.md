# 📚 ESP32 Study Deck — Estación de Estudio Inteligente

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![Platform: ESP32](https://img.shields.io/badge/Platform-ESP32-blue.svg)](https://www.espressif.com/)
[![Touch: CST820 Capacitive](https://img.shields.io/badge/Touch-CST820%20Capacitive-brightgreen.svg)](#)

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

Consulta el desglose detallado en [docs/hardware_pinout.md](docs/hardware_pinout.md).

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

Conforme a las mejores prácticas de código abierto:

```text
├── .github/              # Plantillas de Issues y Pull Requests
├── docs/                 # Documentación técnica, esquemas y arquitectura
│   ├── architecture.md
│   ├── hardware_pinout.md
│   └── README.md
├── firmware/             # Código fuente de los firmwares
│   ├── study_deck/       # Aplicación principal de la estación de estudio
│   └── test_hardware/    # Diagnóstico y verificación de hardware
├── tools/                # Scripts auxiliares y proxy de Google Apps Script
├── CHANGELOG.md          # Historial de cambios siguiendo Keep a Changelog
├── CODE_OF_CONDUCT.md    # Código de conducta para contribuyentes
├── CONTRIBUTING.md       # Guía de contribución y estilo de código
├── LICENSE               # Licencia de código abierto MIT
├── PROYECTO_ESTUDIO.md   # Especificaciones funcionales y fases
└── README.md             # Esta guía principal
```

---

## 📦 Compilación y Carga

Compilado usando **Arduino CLI** con el núcleo oficial ESP32 de Espressif:

```bash
# Compilar test de hardware
arduino-cli compile --fqbn esp32:esp32:esp32 ./firmware/test_hardware

# Flashear al dispositivo (ejemplo puerto COM5)
arduino-cli upload -p COM5 --fqbn esp32:esp32:esp32 ./firmware/test_hardware
```

---

## 📄 Licencia

Este proyecto está distribuido bajo la licencia de código abierto **MIT**. Consulta el archivo [LICENSE](LICENSE) para más detalles.
