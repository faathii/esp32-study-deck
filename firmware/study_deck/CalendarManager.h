#ifndef CALENDAR_MANAGER_H
#define CALENDAR_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include <time.h>
#include "GoogleCalendarClient.h"

#if __has_include("Secrets.h")
#include "Secrets.h"
#endif

#ifndef SECRET_GOOGLE_CALENDAR_URL
#define SECRET_GOOGLE_CALENDAR_URL ""
#endif

typedef GCalEvent CalendarEvent;

enum CalendarViewMode {
    CAL_VIEW_DAY = 0,
    CAL_VIEW_WEEK,
    CAL_VIEW_MONTH
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
    void update(bool wifiConnected = false);

    // Modo de vista de calendario (Día, Semana, Mes)
    CalendarViewMode getViewMode() const { return _viewMode; }
    void setViewMode(CalendarViewMode mode) { _viewMode = mode; }
    int getMonthOffset() const { return _monthOffset; }
    void changeMonth(int delta) { _monthOffset += delta; }
    void resetMonth() { _monthOffset = 0; }

    // Sincronización con Google Calendar
    bool syncGoogleCalendar(bool force = false);
    bool isGoogleCalendarConfigured() const;
    String getGoogleCalendarStatus() const { return _gcal.getStatusMessage(); }
    String getLastSyncString() const { return _gcal.getLastSyncString(); }
    bool isSynced() const { return _gcal.hasData(); }

    // Eventos (Google Calendar o locales)
    int getEventCount() const { return _eventCount; }
    CalendarEvent getEvent(int index) const;
    const CalendarEvent* getEvents() const { return _events; }
    int getTodayEvents(CalendarEvent *outList, int maxOut) const;
    bool hasEventOnDate(int day, int month, int year) const;
    bool hasDeadlineOnDate(int day, int month, int year) const;
    String getEventSummaryForDate(int day, int month, int year) const;
    
    // Hitos y cuenta regresiva de exámenes
    int getDeadlineCount() const { return _deadlineCount; }
    AcademicDeadline getDeadline(int index) const;
    String getNextDeadlineCountdown(String &outTitle);
    bool addDeadline(const String &title, int day, int month, int year, int hour = 9);
    void deleteDeadline(int index);

private:
    GoogleCalendarClient _gcal;
    static const int MAX_EVENTS = 16;
    int _eventCount = 0;
    CalendarEvent _events[MAX_EVENTS];

    CalendarViewMode _viewMode = CAL_VIEW_DAY;
    int _monthOffset = 0;

    int _deadlineCount = 0;
    static const int MAX_DEADLINES = 6;
    AcademicDeadline _deadlines[MAX_DEADLINES];
    Preferences _prefs;

    unsigned long _lastAutoSync = 0;

    void loadDefaultEvents();
    void loadDeadlines();
    void saveDeadlines();
};

#endif // CALENDAR_MANAGER_H
