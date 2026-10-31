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
    int startHour;
    int startMin;
    int endHour;
    int endMin;
    bool isOngoing;
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

