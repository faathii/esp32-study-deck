#ifndef CALENDAR_MANAGER_H
#define CALENDAR_MANAGER_H

#include <Arduino.h>
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
    String getNextDeadlineCountdown(String &outTitle);

private:
    int _eventCount = 3;
    CalendarEvent _events[3];
    int _deadlineCount = 2;
    AcademicDeadline _deadlines[2];
};

#endif // CALENDAR_MANAGER_H

