# 🧪 Test de Hardware — Guition ESP32-2432S024

Sketch de diagnóstico autónomo para verificar todos los componentes físicos de la placa antes de cargar el firmware de producción.

---

## 🎯 Componentes que Valida

1. **Pantalla TFT LCD 2.4" ILI9341:**
   - Renderizado horizontal a 320×240 píxeles.
   - Activación de retroiluminación en `GPIO 27`.

2. **Panel Táctil Capacitivo CST820:**
   - Comunicación I2C en dirección `0x15` (`SDA: 33`, `SCL: 32`, `RST: 25`, `INT: 21`).
   - Muestra en pantalla el estado `ONLINE` o `NO DETECTADO`.
   - Área interactiva de dibujo táctil para trazar con la yema del dedo.

3. **LED RGB:**
   - Botón táctil para conmutar los colores (Rojo `GPIO 4`, Verde `GPIO 16`, Azul `GPIO 17`, Apagado).

4. **Sensor de Luz LDR:**
   - Lectura analógica continua en `GPIO 34` mostrada en tiempo real.

5. **Audio / Zumbador:**
   - Emisión de pitidos de confirmación en `GPIO 26`.

---

## 🚀 Compilación y Carga

```bash
# Compilar
arduino-cli compile --fqbn esp32:esp32:esp32 ./firmware/test_hardware

# Flashear (ajusta tu puerto COM)
arduino-cli upload -p COM5 --fqbn esp32:esp32:esp32 ./firmware/test_hardware
```

