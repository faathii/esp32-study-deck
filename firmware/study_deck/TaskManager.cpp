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

void TaskManager::addTask(const String &text) {
    if (_taskCount < MAX_TASKS) {
        _tasks[_taskCount] = { text, false };
        _taskCount++;
        saveTasks();
    }
}

int TaskManager::getCompletedCount() const {
    int c = 0;
    for (int i = 0; i < _taskCount; i++) {
        if (_tasks[i].completed) c++;
    }
    return c;
}

