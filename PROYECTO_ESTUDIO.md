# 📚 Estación de Estudio Inteligente (ESP32-2432S024)

Dispositivo de escritorio dedicado exclusivamente a la concentración, productividad y administración del tiempo de estudio.

---

## 🛠️ Hardware Objetivo

* **Módulo:** Guition ESP32-2432S024 (CYD 2.4")
* **Microcontrolador:** ESP32-WROOM-32 (Dual Core 240 MHz, Wi-Fi 2.4 GHz, Bluetooth)
* **Pantalla:** TFT LCD de 2.4 pulgadas (240 × 320 px, controlador ILI9341)
* **Panel Táctil:** Capacitivo (controlador CST820 por I2C: SDA=33, SCL=32, RST=25, INT=21)
* **Retroiluminación:** PWM en GPIO 27 (controlada por MOSFET AO3402)
* **Periféricos integrados:**
  * LED RGB direccionable / analógico (aviso visual de estado)
  * Sensor de luz LDR en GPIO 34 (ajuste de brillo ambiental)
  * Altavoz / Zumbador integrado (alertas sonoras)
  * Ranura para tarjeta MicroSD (registro de datos y configuración)

---

## 🎯 Lista Oficial de Funcionalidades

### 1. Gestión del Tiempo y Concentración
* **Temporizador Pomodoro con selector de intervalos:** Configuración de tiempos de enfoque (25m, 50m o personalizado) y descansos (5m, 10m, 15m) con visualización de progreso.
* **Alertas sonoras y timbres de descanso:** Avisos sonoros al iniciar y finalizar cada ciclo de trabajo o pausa.
* **Semáforo de concentración "No Molestar" (LED RGB):** 
  * Rojo = En sesión de estudio profundo.
  * Verde = Tiempo de descanso.
  * Azul / Apagado = Modo espera.

### 2. Agenda y Tareas
* **Sincronizador de eventos de Google Calendar:** Muestra el evento o clase en curso y el siguiente bloque del día con cuenta regresiva.
* **Lista de tareas (Google Tasks o To-Do local):** Visualización y marcado de las tareas prioritarias del día.
* **Cuenta regresiva para exámenes y entregas:** Indicador con los días y horas restantes para hitos académicos clave.

### 3. Técnicas de Aprendizaje y Gamificación
* **Sistema de tarjetas de estudio (Flashcards / Repetición espaciada):** Repaso rápido de conceptos o fórmulas directamente en la pantalla táctil.
* **Contador de rachas y horas de estudio diarias:** Registro visual de bloques completados para mantener el hábito.
* **Registro de tiempos y estadísticas en MicroSD:** Guardado de sesiones, tiempos acumulados y fechas en un archivo `.csv` o `.json`.

### 4. Entorno y Utilidades de Escritorio
* **Reloj y fecha sincronizados por internet (NTP):** Hora exacta con sincronización automática.
* **Widget de clima y temperatura:** Datos meteorológicos actuales del día.
* **Brillo automático con sensor LDR:** Regulación suave de la intensidad de la pantalla según la luz ambiental.
* **Modo reloj nocturno / Protector de pantalla tenue:** Atenuación automática durante inactividad para evitar distracciones visuales.

---

## 🏗️ Arquitectura Técnica Preliminar

1. **Entorno de desarrollo:** VS Code + PlatformIO (Framework Arduino/ESP-IDF).
2. **Motor gráfico:** LVGL 8/9 + LovyanGFX (para rendimiento óptimo de pantalla y respuesta táctil).
3. **Servicio Cloud para Google Calendar:** Google Apps Script Web App (puente ligero que entrega JSON optimizado sin saturar la RAM del ESP32).
4. **Almacenamiento local:** SPIFFS / LittleFS para configuración interna y tarjeta MicroSD para registros extensos e insumos (tarjetas flashcards).

---

## 📋 Fases del Proyecto

- [x] **Fase 1:** Validación y calibración de hardware (TFT ILI9341, táctil capacitivo CST820, LED RGB, LDR y buzzer). ¡Completado y verificado!
- [ ] **Fase 2:** Implementación del motor Pomodoro, estados y alertas (LED + Sonido).
- [ ] **Fase 3:** Conexión Wi-Fi, sincronización de hora (NTP) y clima.
- [ ] **Fase 4:** Integración con Google Calendar y Google Tasks.
- [ ] **Fase 5:** Sistema de Flashcards y registro en MicroSD.
- [ ] **Fase 6:** Diseño y pulido final de la interfaz gráfica táctil.

