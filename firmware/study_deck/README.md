# 📚 Study Deck Firmware

Firmware principal de la **Estación de Estudio Inteligente** para la placa Guition ESP32-2432S024.

---

## 🏗️ Módulos del Firmware

* `study_deck.ino`: Bucle de control principal sin bloqueos (`millis()`).
* `HardwareConfig.h`: Definición de pines y parámetros de hardware.
* `CST820.h / .cpp`: Driver táctil capacitivo adaptado para orientación horizontal.
* `PomodoroEngine.h / .cpp`: Lógica de conteo, descansos y ciclos Pomodoro.
* `UIManager.h / .cpp`: Dibujo de interfaz gráfica con botones táctiles y barra superior.
* `AudioNotifier.h`: Generación de alertas melódicas mediante el zumbador integrado.

---

## 🚀 Compilación y Carga

```bash
# Compilar
arduino-cli compile --fqbn esp32:esp32:esp32 ./firmware/study_deck

# Flashear (ajusta tu puerto COM)
arduino-cli upload -p COM5 --fqbn esp32:esp32:esp32 ./firmware/study_deck
```

