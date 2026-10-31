#ifndef GOOGLE_CALENDAR_CLIENT_H
#define GOOGLE_CALENDAR_CLIENT_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>

struct GCalEvent {
    String title;
    String location;
    String timeRange;
    int day;
    int month;
    int year;
    int startHour;
    int startMin;
    int endHour;
    int endMin;
    bool isOngoing;

    // Soporte completo para eventos recurrentes (RRULE:FREQ=WEEKLY;BYDAY=...)
    bool isRecurring;
    uint8_t repeatDaysMask; // bit 0=Dom, 1=Lun, 2=Mar, 3=Mie, 4=Jue, 5=Vie, 6=Sab
    long startDate;         // YYYYMMDD
    long untilDate;         // YYYYMMDD (0 si no expira)

    bool occursOnDate(int d, int m, int y, int wday) const {
        long target = (long)y * 10000 + m * 100 + d;
        if (!isRecurring) {
            return (day == d && month == m && (year == y || year == 0));
        }
        if (startDate > 0 && target < startDate) return false;
        if (untilDate > 0 && target > untilDate) return false;
        return (repeatDaysMask & (1 << wday)) != 0;
    }
};

class GoogleCalendarClient {
public:
    GoogleCalendarClient();

    void begin();
    
    // Consulta periódica y bajo demanda
    bool fetchEvents(const String &url, GCalEvent *outEvents, int maxEvents, int &outCount);

    bool hasData() const { return _hasData; }
    bool isConfigured(const String &url) const;
    String getStatusMessage() const { return _statusMsg; }
    unsigned long getLastSyncMillis() const { return _lastSyncTime; }
    String getLastSyncString() const;

private:
    bool _hasData = false;
    unsigned long _lastSyncTime = 0;
    String _statusMsg = "Sin sincronizar";

    void parseIcsStream(WiFiClient *stream, GCalEvent *outEvents, int maxEvents, int &outCount, int tzOffsetHours);
    int parseHour(const String &dtStr);
    int parseMinute(const String &dtStr);
    int getTimezoneOffsetHours();
};

#endif // GOOGLE_CALENDAR_CLIENT_H

