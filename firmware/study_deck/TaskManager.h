#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "GoogleCalendarClient.h"

#define MAX_TASKS 10

typedef GCalEvent CalendarEvent;

struct StudyTask {
    String text;
    bool completed;
};

class TaskManager {
public:
    void begin();
    
    int getTaskCount() const { return _taskCount; }
    StudyTask getTask(int index) const;
    void toggleTask(int index);
    bool addTask(const String &text);
    void deleteTask(int index);
    void clearCompleted();
    void resetToDefaults();
    int getCompletedCount() const;

    // Sincronización automática de eventos del calendario como tareas
    int syncFromCalendar(const CalendarEvent *events, int eventCount);

private:
    Preferences _prefs;
    int _taskCount = 0;
    StudyTask _tasks[MAX_TASKS];

    void loadTasks();
    void saveTasks();
};

#endif // TASK_MANAGER_H
