# 🏛️ Arquitectura de Software — Study Deck

Este documento define la estructura y diseño técnico del firmware para el **ESP32 Study Deck**.

---

## 1. Principios de Diseño

1. **Diseño No Bloqueante (Zero Blocking):**
   - El bucle principal (`loop()`) nunca utiliza llamadas a `delay()`.
   - Todas las tareas (actualización del temporizador, lectura del táctil, parpadeo del LED, refresco de pantalla) se ejecutan mediante marcas de tiempo basadas en `millis()`.

2. **Modularidad Estricta:**
   - La lógica de negocio (temporizador Pomodoro) es completamente independiente del motor de renderizado gráfico.
   - Los controladores de hardware (pantalla, táctil, audio, LED) exponen interfaces claras y desacopladas.

3. **Optimización de Memoria (RAM y DMA):**
   - El bus SPI utiliza transacciones rápidas (55 MHz para escritura gráfica).
   - Los buffers de dibujo evitan asignaciones dinámicas repetitivas (`malloc`) para prevenir la fragmentación de la memoria heap del ESP32.

---

## 2. Diagrama de Módulos

```mermaid
flowchart TD
    Main[study_deck.ino] --> UI[UIManager]
    Main --> Pomo[PomodoroEngine]
    Main --> Touch[CST820 Touch Driver]
    Main --> Hardware[HardwareManager]

    Hardware --> LED[LED RGB Driver]
    Hardware --> Audio[Buzzer Audio Notifier]
    Hardware --> LDR[LDR Auto-Brightness]

    UI --> TFT[TFT_eSPI Driver]
    Touch --> I2C[I2C Bus - GPIO 33/32]
```

---

## 3. Máquina de Estados del Pomodoro

```mermaid
stateDiagram-v2
    [*] --> IDLE : Inicio / Espera

    IDLE --> STUDY_RUNNING : Pulsar Iniciar
    STUDY_RUNNING --> STUDY_PAUSED : Pulsar Pausa
    STUDY_PAUSED --> STUDY_RUNNING : Reanudar
    STUDY_RUNNING --> IDLE : Reiniciar

    STUDY_RUNNING --> SHORT_BREAK : Fin bloque estudio
    SHORT_BREAK --> BREAK_PAUSED : Pulsar Pausa
    BREAK_PAUSED --> SHORT_BREAK : Reanudar
    SHORT_BREAK --> STUDY_RUNNING : Fin descanso (Ciclo < 4)

    STUDY_RUNNING --> LONG_BREAK : Fin bloque 4
    LONG_BREAK --> IDLE : Fin descanso largo
```

---

## 4. Semáforo de Concentración "No Molestar"

| Estado Pomodoro | Color LED RGB | Modo | Alerta Sonora |
| :--- | :--- | :--- | :--- |
| **Estudio Activo** | 🔴 Rojo | Fijo | Beep corto ascendente al iniciar |
| **Descanso Corto/Largo** | 🟢 Verde | Fijo | Melodía doble al finalizar estudio |
| **Pausado** | 🟡 Amarillo | Parpadeo suave | Sin sonido continuo |
| **En Espera (Idle)** | 🔵 Azul | Apagado / Respiración | Tono de confirmación |

