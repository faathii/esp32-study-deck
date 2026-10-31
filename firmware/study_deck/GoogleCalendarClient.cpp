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

static String sanitizeText(const String &s) {
    String out = "";
    out.reserve(s.length());
    for (size_t i = 0; i < s.length(); i++) {
        unsigned char c = (unsigned char)s[i];
        if (c >= 32 && c <= 126) {
            out += (char)c;
        } else if (c == 0xC3 && i + 1 < s.length()) { // Caracteres españoles UTF-8 comunes
            unsigned char c2 = (unsigned char)s[++i];
            if (c2 == 0xA1 || c2 == 0x81) out += 'a';
            else if (c2 == 0xA9 || c2 == 0x89) out += 'e';
            else if (c2 == 0xAD || c2 == 0x8D) out += 'i';
            else if (c2 == 0xB3 || c2 == 0x93) out += 'o';
            else if (c2 == 0xBA || c2 == 0x9A) out += 'u';
            else if (c2 == 0xB1 || c2 == 0x91) out += 'n';
        }
    }
    out.trim();
    return out;
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
    http.setTimeout(25000); // 25s de timeout para permitir el stream de 88KB

    int httpCode = http.GET();
    Serial.printf("[GoogleCalendar] Codigo HTTP: %d\n", httpCode);

    if (httpCode != HTTP_CODE_OK) {
        _statusMsg = "Error HTTP " + String(httpCode);
        http.end();
        return false;
    }

    WiFiClient *stream = http.getStreamPtr();
    int tzOffset = getTimezoneOffsetHours();

    parseIcsStream(stream, outEvents, maxEvents, outCount, tzOffset);

    http.end();

    if (outCount > 0) {
        _hasData = true;
        _lastSyncTime = millis();
        _statusMsg = String(outCount) + " eventos sincronizados";
        Serial.printf("[GoogleCalendar] Sincronizacion completada: %d eventos.\n", outCount);
        return true;
    } else {
        _hasData = true;
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
        todayStr[0] = '\0';
    }

    bool inEvent = false;
    String summary = "";
    String location = "";
    String dtstart = "";
    String dtend = "";
    String rrule = "";

    unsigned long startMs = millis();

    while (stream->connected() && (stream->available() || (millis() - startMs < 25000))) {
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
            rrule = "";
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
            } else if (line.startsWith("RRULE:")) {
                rrule = line.substring(6);
            } else if (line.startsWith("END:VEVENT")) {
                inEvent = false;

                if (summary.length() == 0) summary = "Evento Google";
                summary = sanitizeText(summary);
                location = sanitizeText(location);

                int evYear = 0, evMonth = 0, evDay = 0;
                if (dtstart.length() >= 8) {
                    evYear = dtstart.substring(0, 4).toInt();
                    evMonth = dtstart.substring(4, 6).toInt();
                    evDay = dtstart.substring(6, 8).toInt();
                }

                // Filtrar eventos antiguos previos al semestre 2026/2027
                if (evYear > 0 && evYear < 2026) {
                    continue;
                }

                long startDate = 0;
                if (evYear > 0) {
                    startDate = (long)evYear * 10000 + evMonth * 100 + evDay;
                }

                bool isRecurring = false;
                uint8_t repeatDaysMask = 0;
                long untilDate = 0;

                if (rrule.length() > 0) {
                    isRecurring = true;

                    int uIdx = rrule.indexOf("UNTIL=");
                    if (uIdx != -1) {
                        String uPart = rrule.substring(uIdx + 6);
                        if (uPart.length() >= 8) {
                            untilDate = (long)uPart.substring(0, 4).toInt() * 10000 +
                                        uPart.substring(4, 6).toInt() * 100 +
                                        uPart.substring(6, 8).toInt();
                        }
                    }

                    // Ignorar eventos recurrentes viejos que caducaron antes de octubre 2026
                    if (untilDate > 0 && untilDate < 20261001) {
                        continue;
                    }

                    int bIdx = rrule.indexOf("BYDAY=");
                    if (bIdx != -1) {
                        String daysPart = rrule.substring(bIdx + 6);
                        int semi = daysPart.indexOf(';');
                        if (semi != -1) daysPart = daysPart.substring(0, semi);

                        if (daysPart.indexOf("SU") != -1) repeatDaysMask |= (1 << 0);
                        if (daysPart.indexOf("MO") != -1) repeatDaysMask |= (1 << 1);
                        if (daysPart.indexOf("TU") != -1) repeatDaysMask |= (1 << 2);
                        if (daysPart.indexOf("WE") != -1) repeatDaysMask |= (1 << 3);
                        if (daysPart.indexOf("TH") != -1) repeatDaysMask |= (1 << 4);
                        if (daysPart.indexOf("FR") != -1) repeatDaysMask |= (1 << 5);
                        if (daysPart.indexOf("SA") != -1) repeatDaysMask |= (1 << 6);
                    } else {
                        if (rrule.indexOf("FREQ=DAILY") != -1) {
                            repeatDaysMask = 0x7F;
                        } else if (evYear > 0) {
                            struct tm sTm = {0};
                            sTm.tm_year = evYear - 1900;
                            sTm.tm_mon = evMonth - 1;
                            sTm.tm_mday = evDay;
                            mktime(&sTm);
                            repeatDaysMask |= (1 << sTm.tm_wday);
                        }
                    }
                }

                // Criterio de aceptación para el curso 2026/2027:
                long todayNum = (todayStr[0] != '\0') ? atol(todayStr) : 20261031;
                bool accept = false;
                if (isRecurring) {
                    if (untilDate == 0 || todayNum <= untilDate) {
                        accept = true;
                    }
                } else {
                    if (startDate >= todayNum || startDate >= 20261001) {
                        accept = true;
                    }
                }

                if (accept && outCount < maxEvents) {
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

                    bool isToday = false;
                    if (t->tm_year > 120) {
                        int curWDay = t->tm_wday;
                        if (isRecurring) {
                            if ((todayNum >= startDate) && (untilDate == 0 || todayNum <= untilDate)) {
                                if ((repeatDaysMask & (1 << curWDay)) != 0) isToday = true;
                            }
                        } else {
                            if (dtstart.startsWith(todayStr)) isToday = true;
                        }
                    }

                    bool ongoing = false;
                    if (t->tm_year > 120 && isToday && tIdx != -1) {
                        int nowMins = t->tm_hour * 60 + t->tm_min;
                        int startMins = sH * 60 + sM;
                        int endMins = eH * 60 + eM;
                        if (endMins < startMins) endMins += 1440;
                        ongoing = (nowMins >= startMins && nowMins <= endMins);
                    }

                    outEvents[outCount] = { 
                        summary, location, timeRange, 
                        evDay, evMonth, evYear, 
                        sH, sM, eH, eM, ongoing,
                        isRecurring, repeatDaysMask, startDate, untilDate
                    };
                    Serial.printf("[GoogleCalendar] Evento UNED: '%s' | %s\n", summary.c_str(), timeRange.c_str());
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
