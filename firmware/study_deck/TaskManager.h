#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

#define MAX_TASKS 6

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

private:
    Preferences _prefs;
    int _taskCount = 0;
    StudyTask _tasks[MAX_TASKS];

    void loadTasks();
    void saveTasks();
};

#endif // TASK_MANAGER_H
