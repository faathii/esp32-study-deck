#include "PomodoroEngine.h"

PomodoroEngine::PomodoroEngine() {
    _state = POMO_STATE_IDLE;
    _remainingSeconds = _focusMinutes * 60;
    _totalDurationSeconds = _remainingSeconds;
}

void PomodoroEngine::begin() {
    _state = POMO_STATE_IDLE;
    _remainingSeconds = _focusMinutes * 60;
    _totalDurationSeconds = _remainingSeconds;
    _currentCycle = 1;
    _lastTickMillis = millis();
}

bool PomodoroEngine::isRunning() const {
    return (_state == POMO_STATE_FOCUS || _state == POMO_STATE_SHORT_BREAK || _state == POMO_STATE_LONG_BREAK);
}

bool PomodoroEngine::isPaused() const {
    return (_state == POMO_STATE_FOCUS_PAUSED || _state == POMO_STATE_SHORT_BREAK_PAUSED || _state == POMO_STATE_LONG_BREAK_PAUSED);
}

bool PomodoroEngine::isFocus() const {
    return (_state == POMO_STATE_FOCUS || _state == POMO_STATE_FOCUS_PAUSED || _state == POMO_STATE_IDLE);
}

bool PomodoroEngine::isBreak() const {
    return (_state == POMO_STATE_SHORT_BREAK || _state == POMO_STATE_SHORT_BREAK_PAUSED ||
            _state == POMO_STATE_LONG_BREAK || _state == POMO_STATE_LONG_BREAK_PAUSED);
}

float PomodoroEngine::getProgress() const {
    if (_totalDurationSeconds == 0) return 0.0f;
    uint32_t elapsed = _totalDurationSeconds - _remainingSeconds;
    return (float)elapsed / (float)_totalDurationSeconds;
}

void PomodoroEngine::setFocusMinutes(uint16_t minutes) {
    _focusMinutes = minutes;
    if (_state == POMO_STATE_IDLE) {
        _remainingSeconds = _focusMinutes * 60;
        _totalDurationSeconds = _remainingSeconds;
    }
}

void PomodoroEngine::start() {
    if (_state == POMO_STATE_IDLE) {
        _state = POMO_STATE_FOCUS;
        _remainingSeconds = _focusMinutes * 60;
        _totalDurationSeconds = _remainingSeconds;
        _lastTickMillis = millis();
    }
}

void PomodoroEngine::pause() {
    if (_state == POMO_STATE_FOCUS) {
        _state = POMO_STATE_FOCUS_PAUSED;
    } else if (_state == POMO_STATE_SHORT_BREAK) {
        _state = POMO_STATE_SHORT_BREAK_PAUSED;
    } else if (_state == POMO_STATE_LONG_BREAK) {
        _state = POMO_STATE_LONG_BREAK_PAUSED;
    }
}

void PomodoroEngine::resume() {
    if (_state == POMO_STATE_FOCUS_PAUSED) {
        _state = POMO_STATE_FOCUS;
        _lastTickMillis = millis();
    } else if (_state == POMO_STATE_SHORT_BREAK_PAUSED) {
        _state = POMO_STATE_SHORT_BREAK;
        _lastTickMillis = millis();
    } else if (_state == POMO_STATE_LONG_BREAK_PAUSED) {
        _state = POMO_STATE_LONG_BREAK;
        _lastTickMillis = millis();
    }
}

void PomodoroEngine::togglePlayPause() {
    if (_state == POMO_STATE_IDLE) {
        start();
    } else if (isRunning()) {
        pause();
    } else if (isPaused()) {
        resume();
    }
}

void PomodoroEngine::reset() {
    _state = POMO_STATE_IDLE;
    _remainingSeconds = _focusMinutes * 60;
    _totalDurationSeconds = _remainingSeconds;
}

void PomodoroEngine::skip() {
    if (isFocus()) {
        prepareNextState(true);
    } else {
        prepareNextState(false);
    }
}

void PomodoroEngine::prepareNextState(bool focusFinished) {
    if (focusFinished) {
        _completedSessionsToday++;
        if (_currentCycle >= _totalCycles) {
            // Toca descanso largo
            _state = POMO_STATE_LONG_BREAK;
            _remainingSeconds = _longBreakMinutes * 60;
            _totalDurationSeconds = _remainingSeconds;
            _currentCycle = 1; // Reiniciar ciclo
        } else {
            // Descanso corto
            _state = POMO_STATE_SHORT_BREAK;
            _remainingSeconds = _shortBreakMinutes * 60;
            _totalDurationSeconds = _remainingSeconds;
            _currentCycle++;
        }
    } else {
        // Fin de descanso -> Volver a estudio
        _state = POMO_STATE_FOCUS;
        _remainingSeconds = _focusMinutes * 60;
        _totalDurationSeconds = _remainingSeconds;
    }
    _lastTickMillis = millis();
}

PomodoroEvent PomodoroEngine::update() {
    if (!isRunning()) {
        return POMO_EVENT_NONE;
    }

    unsigned long now = millis();
    if (now - _lastTickMillis >= 1000) {
        _lastTickMillis += 1000;
        if (_remainingSeconds > 0) {
            _remainingSeconds--;
        }

        if (_remainingSeconds == 0) {
            if (isFocus()) {
                prepareNextState(true);
                return POMO_EVENT_FOCUS_COMPLETE;
            } else {
                prepareNextState(false);
                return POMO_EVENT_BREAK_COMPLETE;
            }
        }
    }

    return POMO_EVENT_NONE;
}

