#include "TaskManager.h"

void TaskManager::begin() {
    _prefs.begin("study_tasks", false);
    loadTasks();
}

void TaskManager::loadTasks() {
    _taskCount = _prefs.getInt("count", -1);
    if (_taskCount <= 0 || _taskCount > MAX_TASKS) {
        resetToDefaults();
        return;
    }

    for (int i = 0; i < _taskCount; i++) {
        char keyText[16], keyDone[16];
        snprintf(keyText, sizeof(keyText), "t_%d", i);
        snprintf(keyDone, sizeof(keyDone), "d_%d", i);
        _tasks[i].text = _prefs.getString(keyText, "Tarea");
        _tasks[i].completed = _prefs.getBool(keyDone, false);
    }
}

void TaskManager::saveTasks() {
    _prefs.putInt("count", _taskCount);
    for (int i = 0; i < _taskCount; i++) {
        char keyText[16], keyDone[16];
        snprintf(keyText, sizeof(keyText), "t_%d", i);
        snprintf(keyDone, sizeof(keyDone), "d_%d", i);
        _prefs.putString(keyText, _tasks[i].text);
        _prefs.putBool(keyDone, _tasks[i].completed);
    }
}

void TaskManager::resetToDefaults() {
    _taskCount = 4;
    _tasks[0] = { "Repasar apuntes del tema 1", false };
    _tasks[1] = { "Resolver 5 ejercicios practicos", false };
    _tasks[2] = { "Hacer resumen esquematico", false };
    _tasks[3] = { "Sesion de flashcards de repaso", false };
    saveTasks();
}

StudyTask TaskManager::getTask(int index) const {
    if (index >= 0 && index < _taskCount) {
        return _tasks[index];
    }
    return { "", false };
}

void TaskManager::toggleTask(int index) {
    if (index >= 0 && index < _taskCount) {
        _tasks[index].completed = !_tasks[index].completed;
        saveTasks();
    }
}

bool TaskManager::addTask(const String &text) {
    if (text.length() == 0) return false;
    if (_taskCount < MAX_TASKS) {
        _tasks[_taskCount] = { text, false };
        _taskCount++;
        saveTasks();
        return true;
    }
    return false;
}

void TaskManager::deleteTask(int index) {
    if (index >= 0 && index < _taskCount) {
        for (int i = index; i < _taskCount - 1; i++) {
            _tasks[i] = _tasks[i + 1];
        }
        _taskCount--;
        saveTasks();
    }
}

void TaskManager::clearCompleted() {
    int writeIdx = 0;
    for (int i = 0; i < _taskCount; i++) {
        if (!_tasks[i].completed) {
            _tasks[writeIdx++] = _tasks[i];
        }
    }
    _taskCount = writeIdx;
    saveTasks();
}

int TaskManager::getCompletedCount() const {
    int c = 0;
    for (int i = 0; i < _taskCount; i++) {
        if (_tasks[i].completed) c++;
    }
    return c;
}

int TaskManager::syncFromCalendar(const CalendarEvent *events, int eventCount) {
    if (!events || eventCount <= 0) return 0;
    int added = 0;

    for (int i = 0; i < eventCount; i++) {
        String title = events[i].title;
        title.trim();
        if (title.length() == 0) continue;

        String taskText = "[CAL] " + title;
        if (taskText.length() > 24) {
            taskText = taskText.substring(0, 22) + "..";
        }

        bool exists = false;
        for (int j = 0; j < _taskCount; j++) {
            if (_tasks[j].text == taskText || _tasks[j].text.indexOf(title) != -1) {
                exists = true;
                break;
            }
        }

        if (!exists && _taskCount < MAX_TASKS) {
            _tasks[_taskCount] = { taskText, false };
            _taskCount++;
            added++;
        }
    }

    if (added > 0) {
        saveTasks();
        Serial.printf("[TaskManager] %d eventos sincronizados como tareas.\n", added);
    }
    return added;
}
