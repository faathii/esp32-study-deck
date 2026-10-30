#ifndef CALENDAR_MANAGER_H
#define CALENDAR_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <time.h>

struct CalendarEvent {
    String title;
    String location;
    String timeRange;
    bool isOngoing;
};

struct AcademicDeadline {
    String title;
    int targetDay;
    int targetMonth;
    int targetYear;
    int targetHour;
};

class CalendarManager {
public:
    void begin();
    void update();

    int getEventCount() const { return _eventCount; }
    CalendarEvent getEvent(int index) const;
    
    // Hitos y cuenta regresiva de exámenes
    int getDeadlineCount() const { return _deadlineCount; }
    AcademicDeadline getDeadline(int index) const;
    String getNextDeadlineCountdown(String &outTitle);
    bool addDeadline(const String &title, int day, int month, int year, int hour = 9);
    void deleteDeadline(int index);

private:
    int _eventCount = 3;
    CalendarEvent _events[3];
    int _deadlineCount = 0;
    static const int MAX_DEADLINES = 6;
    AcademicDeadline _deadlines[MAX_DEADLINES];
    Preferences _prefs;

    void loadDeadlines();
    void saveDeadlines();
};

#endif // CALENDAR_MANAGER_H
