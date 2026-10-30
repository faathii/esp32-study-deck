#ifndef POMODORO_ENGINE_H
#define POMODORO_ENGINE_H

#include <Arduino.h>

enum PomodoroState {
    POMO_STATE_IDLE,
    POMO_STATE_FOCUS,
    POMO_STATE_FOCUS_PAUSED,
    POMO_STATE_SHORT_BREAK,
    POMO_STATE_SHORT_BREAK_PAUSED,
    POMO_STATE_LONG_BREAK,
    POMO_STATE_LONG_BREAK_PAUSED
};

enum PomodoroEvent {
    POMO_EVENT_NONE,
    POMO_EVENT_STARTED,
    POMO_EVENT_PAUSED,
    POMO_EVENT_RESUMED,
    POMO_EVENT_RESET,
    POMO_EVENT_FOCUS_COMPLETE,
    POMO_EVENT_BREAK_COMPLETE,
    POMO_EVENT_SKIPPED
};

class PomodoroEngine {
public:
    PomodoroEngine();

    void begin();
    PomodoroEvent update(); // Llamado periódicamente en loop()

    // Acciones del usuario
    void start();
    void pause();
    void resume();
    void togglePlayPause();
    void reset();
    void skip();

    // Configuración de tiempos
    void setFocusMinutes(uint16_t minutes);
    uint16_t getFocusMinutes() const { return _focusMinutes; }
    void setShortBreakMinutes(uint16_t minutes) { _shortBreakMinutes = minutes; }
    void setLongBreakMinutes(uint16_t minutes) { _longBreakMinutes = minutes; }

    // Consultas de estado
    PomodoroState getState() const { return _state; }
    bool isRunning() const;
    bool isPaused() const;
    bool isFocus() const;
    bool isBreak() const;

    uint32_t getRemainingSeconds() const { return _remainingSeconds; }
    uint32_t getTotalSeconds() const { return _totalDurationSeconds; }
    float getProgress() const; // 0.0f a 1.0f

    uint8_t getCurrentCycle() const { return _currentCycle; }
    uint8_t getTotalCycles() const { return _totalCycles; }
    uint16_t getCompletedSessionsToday() const { return _completedSessionsToday; }

private:
    PomodoroState _state = POMO_STATE_IDLE;
    uint16_t _focusMinutes = 25;
    uint16_t _shortBreakMinutes = 5;
    uint16_t _longBreakMinutes = 15;
    uint8_t _totalCycles = 4;
    uint8_t _currentCycle = 1;
    uint16_t _completedSessionsToday = 0;

    uint32_t _remainingSeconds = 25 * 60;
    uint32_t _totalDurationSeconds = 25 * 60;
    unsigned long _lastTickMillis = 0;

    void prepareNextState(bool focusFinished);
};

#endif // POMODORO_ENGINE_H

