#include "GoogleCalendarClient.h"

GoogleCalendarClient::GoogleCalendarClient() {}

void GoogleCalendarClient::begin() {
    _hasData = false;
    _lastSyncTime = 0;
    _statusMsg = "Sin sincronizar";
}

bool GoogleCalendarClient::isConfigured(const String &url) const {
    return (url.length() > 15 && (url.startsWith("http://") || url.startsWith("https://")));
}

int GoogleCalendarClient::getTimezoneOffsetHours() {
    time_t now = time(nullptr);
    struct tm *g = gmtime(&now);
    int gmHour = g->tm_hour;
    struct tm *l = localtime(&now);
    int locHour = l->tm_hour;

    int diff = locHour - gmHour;
    if (diff < -12) diff += 24;
    if (diff > 12) diff -= 24;
    return diff;
}

bool GoogleCalendarClient::fetchEvents(const String &url, GCalEvent *outEvents, int maxEvents, int &outCount) {
    outCount = 0;
    if (!isConfigured(url)) {
        _statusMsg = "Configura URL en Secrets.h";
        _hasData = false;
        return false;
    }

    Serial.println("[GoogleCalendar] Conectando a Google Calendar...");
    _statusMsg = "Conectando con Google...";

    WiFiClientSecure client;
    client.setInsecure(); // No requiere certificados SSL complejos

    HTTPClient http;
    http.begin(client, url);
    http.setFollowRedirects(HTTPC_FORCE_FOLLOW_REDIRECTS);
    http.setTimeout(12000);

    int httpCode = http.GET();
    Serial.printf("[GoogleCalendar] Codigo HTTP: %d\n", httpCode);

    if (httpCode != HTTP_CODE_OK) {
        _statusMsg = "Error HTTP " + String(httpCode);
        http.end();
        return false;
    }

    WiFiClient *stream = http.getStreamPtr();
    int tzOffset = getTimezoneOffsetHours();

    // Comprobar si la respuesta es iCal (ICS) o JSON de Apps Script
    // Para no bloquear, leer los primeros caracteres
    parseIcsStream(stream, outEvents, maxEvents, outCount, tzOffset);

    http.end();

    if (outCount > 0) {
        _hasData = true;
        _lastSyncTime = millis();
        _statusMsg = String(outCount) + " eventos sincronizados";
        Serial.printf("[GoogleCalendar] Sincronizacion completada: %d eventos.\n", outCount);
        return true;
    } else {
        _hasData = true; // Sincronizado, pero sin eventos hoy
        _lastSyncTime = millis();
        _statusMsg = "Sin eventos hoy en GCal";
        Serial.println("[GoogleCalendar] Sincronizado, no hay eventos hoy.");
        return true;
    }
}

void GoogleCalendarClient::parseIcsStream(WiFiClient *stream, GCalEvent *outEvents, int maxEvents, int &outCount, int tzOffsetHours) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    char todayStr[16];
    if (t->tm_year > 120) {
        snprintf(todayStr, sizeof(todayStr), "%04d%02d%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday);
    } else {
        todayStr[0] = '\0'; // Reloj aún no sincronizado
    }

    bool inEvent = false;
    String summary = "";
    String location = "";
    String dtstart = "";
    String dtend = "";

    unsigned long startMs = millis();

    while (stream->connected() && (stream->available() || (millis() - startMs < 6000))) {
        if (!stream->available()) {
            delay(10);
            continue;
        }

        String line = stream->readStringUntil('\n');
        line.trim();

        if (line.startsWith("BEGIN:VEVENT")) {
            inEvent = true;
            summary = "";
            location = "";
            dtstart = "";
            dtend = "";
        } else if (inEvent) {
            if (line.startsWith("SUMMARY:")) {
                summary = line.substring(8);
                summary.replace("\\,", ",");
                summary.replace("\\n", " ");
            } else if (line.startsWith("LOCATION:")) {
                location = line.substring(9);
                location.replace("\\,", ",");
            } else if (line.startsWith("DTSTART")) {
                int c = line.indexOf(':');
                if (c != -1) dtstart = line.substring(c + 1);
            } else if (line.startsWith("DTEND")) {
                int c = line.indexOf(':');
                if (c != -1) dtend = line.substring(c + 1);
            } else if (line.startsWith("END:VEVENT")) {
                inEvent = false;

                // Si no hay título, nombrar "Evento"
                if (summary.length() == 0) summary = "Evento Google";

                // Filtrado por fecha: si el reloj está sincronizado, preferir eventos de hoy o próximos
                bool isToday = false;
                if (todayStr[0] != '\0' && dtstart.length() >= 8) {
                    if (dtstart.startsWith(todayStr)) {
                        isToday = true;
                    }
                } else {
                    // Si el reloj no tiene hora exacta aún, aceptar los primeros eventos disponibles
                    isToday = true;
                }

                if (isToday && outCount < maxEvents) {
                    int sH = 9, sM = 0, eH = 10, eM = 0;
                    String timeRange = "Todo el dia";

                    int tIdx = dtstart.indexOf('T');
                    if (tIdx != -1 && (int)dtstart.length() >= tIdx + 5) {
                        sH = dtstart.substring(tIdx + 1, tIdx + 3).toInt();
                        sM = dtstart.substring(tIdx + 3, tIdx + 5).toInt();
                        if (dtstart.endsWith("Z")) {
                            sH = (sH + tzOffsetHours + 24) % 24;
                        }

                        if (dtend.indexOf('T') != -1) {
                            int teIdx = dtend.indexOf('T');
                            eH = dtend.substring(teIdx + 1, teIdx + 3).toInt();
                            eM = dtend.substring(teIdx + 3, teIdx + 5).toInt();
                            if (dtend.endsWith("Z")) {
                                eH = (eH + tzOffsetHours + 24) % 24;
                            }
                        } else {
                            eH = (sH + 1) % 24;
                            eM = sM;
                        }

                        char rBuf[32];
                        snprintf(rBuf, sizeof(rBuf), "%02d:%02d - %02d:%02d", sH, sM, eH, eM);
                        timeRange = String(rBuf);
                    }

                    // Determinar si está en curso ahora
                    bool ongoing = false;
                    if (t->tm_year > 120 && isToday && tIdx != -1) {
                        int nowMins = t->tm_hour * 60 + t->tm_min;
                        int startMins = sH * 60 + sM;
                        int endMins = eH * 60 + eM;
                        if (endMins < startMins) endMins += 1440;
                        ongoing = (nowMins >= startMins && nowMins <= endMins);
                    }

                    outEvents[outCount] = { summary, location, timeRange, sH, sM, eH, eM, ongoing };
                    outCount++;

                    if (outCount >= maxEvents) {
                        break;
                    }
                }
            }
        }
    }
}

String GoogleCalendarClient::getLastSyncString() const {
    if (_lastSyncTime == 0) return "Nunca";
    unsigned long diffSec = (millis() - _lastSyncTime) / 1000;
    if (diffSec < 60) return "Ahora";
    if (diffSec < 3600) return String(diffSec / 60) + " min";
    return String(diffSec / 3600) + " h";
}

