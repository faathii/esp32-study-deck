#include "CalendarManager.h"

void CalendarManager::begin() {
    _prefs.begin("study_cal", false);
    _gcal.begin();

    // Purgar datos de ejemplo y asegurar datos 100% reales de UNED Ávila
    if (!_prefs.getBool("real_uned_v3", false)) {
        _prefs.clear();
        _prefs.putBool("real_uned_v3", true);
        _deadlineCount = 0;
        _eventCount = 0;
    }

    loadDeadlines();
    loadDefaultEvents();
}

void CalendarManager::loadDefaultEvents() {
    _eventCount = 0;
    // Cargar los eventos calendarizados reales de la UNED en memoria de trabajo
    for (int i = 0; i < NUM_DATED_EVENTS && _eventCount < MAX_EVENTS; i++) {
        char tr[32];
        snprintf(tr, sizeof(tr), "%02d:%02d - %02d:%02d",
                 REAL_DATED_EVENTS[i].startHour, REAL_DATED_EVENTS[i].startMin,
                 REAL_DATED_EVENTS[i].endHour, REAL_DATED_EVENTS[i].endMin);

        _events[_eventCount] = {
            String(REAL_DATED_EVENTS[i].title),
            String(REAL_DATED_EVENTS[i].location),
            String(tr),
            REAL_DATED_EVENTS[i].day,
            REAL_DATED_EVENTS[i].month,
            REAL_DATED_EVENTS[i].year,
            REAL_DATED_EVENTS[i].startHour,
            REAL_DATED_EVENTS[i].startMin,
            REAL_DATED_EVENTS[i].endHour,
            REAL_DATED_EVENTS[i].endMin,
            false,
            false, 0, 0, 0
        };
        _eventCount++;
    }
}

bool CalendarManager::isGoogleCalendarConfigured() const {
    return _gcal.isConfigured(String(SECRET_GOOGLE_CALENDAR_URL));
}

bool CalendarManager::syncGoogleCalendar(bool force) {
    if (!isGoogleCalendarConfigured()) {
        return false;
    }

    if (!force && _lastAutoSync > 0 && (millis() - _lastAutoSync < 900000)) { // 15 minutos
        return true;
    }

    int count = 0;
    CalendarEvent tempEvents[MAX_EVENTS];
    bool ok = _gcal.fetchEvents(String(SECRET_GOOGLE_CALENDAR_URL), tempEvents, MAX_EVENTS, count);

    if (ok && count > 0) {
        _eventCount = count;
        for (int i = 0; i < count; i++) {
            _events[i] = tempEvents[i];
        }
        _lastAutoSync = millis();
        return true;
    } else if (ok && count == 0) {
        // Mantener los eventos oficiales precargados
        _lastAutoSync = millis();
        return true;
    }

    return false;
}

void CalendarManager::loadDeadlines() {
    _deadlineCount = _prefs.getInt("dl_count", 0);
    if (_deadlineCount <= 0 || _deadlineCount > MAX_DEADLINES) {
        // Cargar los 7 hitos, PECs y exámenes oficiales reales de la UNED Ávila
        _deadlineCount = NUM_REAL_DEADLINES;
        for (int i = 0; i < NUM_REAL_DEADLINES && i < MAX_DEADLINES; i++) {
            _deadlines[i].title       = REAL_DEADLINES[i].title;
            _deadlines[i].targetDay   = REAL_DEADLINES[i].day;
            _deadlines[i].targetMonth = REAL_DEADLINES[i].month;
            _deadlines[i].targetYear  = REAL_DEADLINES[i].year;
            _deadlines[i].targetHour  = REAL_DEADLINES[i].hour;
        }
        saveDeadlines();
        return;
    }

    for (int i = 0; i < _deadlineCount; i++) {
        char kT[16], kD[16], kM[16], kY[16], kH[16];
        snprintf(kT, sizeof(kT), "t_%d", i);
        snprintf(kD, sizeof(kD), "d_%d", i);
        snprintf(kM, sizeof(kM), "m_%d", i);
        snprintf(kY, sizeof(kY), "y_%d", i);
        snprintf(kH, sizeof(kH), "h_%d", i);

        _deadlines[i].title       = _prefs.getString(kT, "");
        _deadlines[i].targetDay   = _prefs.getInt(kD, 1);
        _deadlines[i].targetMonth = _prefs.getInt(kM, 1);
        _deadlines[i].targetYear  = _prefs.getInt(kY, 2026);
        _deadlines[i].targetHour  = _prefs.getInt(kH, 10);
    }
}

void CalendarManager::saveDeadlines() {
    _prefs.putInt("dl_count", _deadlineCount);
    for (int i = 0; i < _deadlineCount; i++) {
        char kT[16], kD[16], kM[16], kY[16], kH[16];
        snprintf(kT, sizeof(kT), "t_%d", i);
        snprintf(kD, sizeof(kD), "d_%d", i);
        snprintf(kM, sizeof(kM), "m_%d", i);
        snprintf(kY, sizeof(kY), "y_%d", i);
        snprintf(kH, sizeof(kH), "h_%d", i);

        _prefs.putString(kT, _deadlines[i].title);
        _prefs.putInt(kD, _deadlines[i].targetDay);
        _prefs.putInt(kM, _deadlines[i].targetMonth);
        _prefs.putInt(kY, _deadlines[i].targetYear);
        _prefs.putInt(kH, _deadlines[i].targetHour);
    }
}

void CalendarManager::update(bool wifiConnected) {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    if (t->tm_year > 120) {
        int nowMins = t->tm_hour * 60 + t->tm_min;
        for (int i = 0; i < _eventCount; i++) {
            int s = _events[i].startHour * 60 + _events[i].startMin;
            int e = _events[i].endHour * 60 + _events[i].endMin;
            if (e < s) e += 1440;
            _events[i].isOngoing = (nowMins >= s && nowMins <= e);
        }
    }

    if (wifiConnected && isGoogleCalendarConfigured()) {
        if (_lastAutoSync == 0 || (millis() - _lastAutoSync >= 900000)) { // Auto-sync cada 15 min
            syncGoogleCalendar(false);
        }
    }
}

CalendarEvent CalendarManager::getEvent(int index) const {
    if (index >= 0 && index < _eventCount) {
        return _events[index];
    }
    return CalendarEvent{ "", "", "", 0, 0, 0, 0, false };
}

AcademicDeadline CalendarManager::getDeadline(int index) const {
    if (index >= 0 && index < _deadlineCount) {
        return _deadlines[index];
    }
    return AcademicDeadline{ "", 0, 0, 0, 0 };
}

String CalendarManager::getNextDeadlineCountdown(String &outTitle) {
    if (_deadlineCount == 0) {
        outTitle = "Sin examenes programados";
        return "--";
    }

    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    int bestIdx = -1;
    double bestDiff = 999999999.0;

    for (int i = 0; i < _deadlineCount; i++) {
        struct tm target = {0};
        target.tm_year = _deadlines[i].targetYear - 1900;
        target.tm_mon  = _deadlines[i].targetMonth - 1;
        target.tm_mday = _deadlines[i].targetDay;
        target.tm_hour = _deadlines[i].targetHour;
        target.tm_min  = 0;
        target.tm_sec  = 0;

        time_t targetTime = mktime(&target);
        double diffSeconds = difftime(targetTime, now);

        if (diffSeconds > 0 && diffSeconds < bestDiff) {
            bestDiff = diffSeconds;
            bestIdx = i;
        }
    }

    if (bestIdx == -1) {
        if (t->tm_year <= 120) {
            outTitle = _deadlines[0].title;
            return "Conecta WiFi";
        }
        outTitle = _deadlines[_deadlineCount - 1].title;
        return "Finalizado";
    }

    outTitle = _deadlines[bestIdx].title;
    long days = (long)(bestDiff / 86400);
    long hours = (long)(((long)bestDiff % 86400) / 3600);
    long mins = (long)(((long)bestDiff % 3600) / 60);

    char buf[32];
    if (days > 0) {
        snprintf(buf, sizeof(buf), "%ldd %ldh %ldm", days, hours, mins);
    } else {
        snprintf(buf, sizeof(buf), "%ldh %ldm", hours, mins);
    }
    return String(buf);
}

bool CalendarManager::addDeadline(const String &title, int day, int month, int year, int hour) {
    if (title.length() == 0) return false;
    if (_deadlineCount < MAX_DEADLINES) {
        _deadlines[_deadlineCount] = { title, day, month, year, hour };
        _deadlineCount++;
        saveDeadlines();
        return true;
    }
    return false;
}

void CalendarManager::deleteDeadline(int index) {
    if (index >= 0 && index < _deadlineCount) {
        for (int i = index; i < _deadlineCount - 1; i++) {
            _deadlines[i] = _deadlines[i + 1];
        }
        _deadlineCount--;
        saveDeadlines();
    }
}

int CalendarManager::getTodayEvents(CalendarEvent *outList, int maxOut) const {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    int cD = (t->tm_year > 120) ? t->tm_mday : 31;
    int cM = (t->tm_year > 120) ? (t->tm_mon + 1) : 10;
    int cY = (t->tm_year > 120) ? (t->tm_year + 1900) : 2026;
    int cW = (t->tm_year > 120) ? t->tm_wday : 6;

    int matched = 0;

    // 1. Eventos sincronizados desde Google Calendar para hoy
    for (int i = 0; i < _eventCount && matched < maxOut; i++) {
        if (_events[i].occursOnDate(cD, cM, cY, cW)) {
            outList[matched] = _events[i];
            if (t->tm_year > 120) {
                int nowMins = t->tm_hour * 60 + t->tm_min;
                int startMins = _events[i].startHour * 60 + _events[i].startMin;
                int endMins = _events[i].endHour * 60 + _events[i].endMin;
                if (endMins < startMins) endMins += 1440;
                outList[matched].isOngoing = (nowMins >= startMins && nowMins <= endMins);
            }
            matched++;
        }
    }

    // 2. Eventos calendarizados reales en REAL_DATED_EVENTS
    for (int i = 0; i < NUM_DATED_EVENTS && matched < maxOut; i++) {
        if (REAL_DATED_EVENTS[i].year == cY && 
            REAL_DATED_EVENTS[i].month == cM && 
            REAL_DATED_EVENTS[i].day == cD) {
            
            bool dup = false;
            for (int k = 0; k < matched; k++) {
                if (outList[k].title.indexOf(REAL_DATED_EVENTS[i].title) != -1) {
                    dup = true;
                    break;
                }
            }
            if (!dup) {
                char tr[32];
                snprintf(tr, sizeof(tr), "%02d:%02d - %02d:%02d",
                         REAL_DATED_EVENTS[i].startHour, REAL_DATED_EVENTS[i].startMin,
                         REAL_DATED_EVENTS[i].endHour, REAL_DATED_EVENTS[i].endMin);

                bool ongoing = false;
                if (t->tm_year > 120) {
                    int nowMins = t->tm_hour * 60 + t->tm_min;
                    int startMins = REAL_DATED_EVENTS[i].startHour * 60 + REAL_DATED_EVENTS[i].startMin;
                    int endMins = REAL_DATED_EVENTS[i].endHour * 60 + REAL_DATED_EVENTS[i].endMin;
                    if (endMins < startMins) endMins += 1440;
                    ongoing = (nowMins >= startMins && nowMins <= endMins);
                }

                outList[matched] = {
                    String(REAL_DATED_EVENTS[i].title),
                    String(REAL_DATED_EVENTS[i].location),
                    String(tr),
                    cD, cM, cY,
                    REAL_DATED_EVENTS[i].startHour, REAL_DATED_EVENTS[i].startMin,
                    REAL_DATED_EVENTS[i].endHour, REAL_DATED_EVENTS[i].endMin,
                    ongoing,
                    false, 0, 0, 0
                };
                matched++;
            }
        }
    }

    // 3. Tutorías semanales de la UNED para hoy
    long curDateNum = (long)cY * 10000 + cM * 100 + cD;
    for (int i = 0; i < NUM_TUTORIAS && matched < maxOut; i++) {
        if (REAL_TUTORIAS[i].wday == cW && curDateNum <= REAL_TUTORIAS[i].untilDate) {
            bool dup = false;
            for (int k = 0; k < matched; k++) {
                if (outList[k].title.indexOf("Tutoria") != -1) {
                    dup = true;
                    break;
                }
            }
            if (!dup) {
                char tr[32];
                snprintf(tr, sizeof(tr), "%02d:%02d - %02d:%02d",
                         REAL_TUTORIAS[i].startHour, REAL_TUTORIAS[i].startMin,
                         REAL_TUTORIAS[i].endHour, REAL_TUTORIAS[i].endMin);

                bool ongoing = false;
                if (t->tm_year > 120) {
                    int nowMins = t->tm_hour * 60 + t->tm_min;
                    int startMins = REAL_TUTORIAS[i].startHour * 60 + REAL_TUTORIAS[i].startMin;
                    int endMins = REAL_TUTORIAS[i].endHour * 60 + REAL_TUTORIAS[i].endMin;
                    if (endMins < startMins) endMins += 1440;
                    ongoing = (nowMins >= startMins && nowMins <= endMins);
                }

                outList[matched] = {
                    String(REAL_TUTORIAS[i].title),
                    "UNED Avila",
                    String(tr),
                    cD, cM, cY,
                    REAL_TUTORIAS[i].startHour, REAL_TUTORIAS[i].startMin,
                    REAL_TUTORIAS[i].endHour, REAL_TUTORIAS[i].endMin,
                    ongoing,
                    true, (uint8_t)(1 << cW), 20261001, REAL_TUTORIAS[i].untilDate
                };
                matched++;
            }
        }
    }

    return matched;
}

bool CalendarManager::hasEventOnDate(int day, int month, int year) const {
    struct tm dTm = {0};
    dTm.tm_year = year - 1900;
    dTm.tm_mon = month - 1;
    dTm.tm_mday = day;
    mktime(&dTm);
    int wday = dTm.tm_wday;

    for (int i = 0; i < _eventCount; i++) {
        if (_events[i].occursOnDate(day, month, year, wday)) {
            return true;
        }
    }

    for (int i = 0; i < NUM_DATED_EVENTS; i++) {
        if (REAL_DATED_EVENTS[i].year == year && 
            REAL_DATED_EVENTS[i].month == month && 
            REAL_DATED_EVENTS[i].day == day) {
            return true;
        }
    }

    long targetDate = (long)year * 10000 + month * 100 + day;
    for (int i = 0; i < NUM_TUTORIAS; i++) {
        if (REAL_TUTORIAS[i].wday == wday && targetDate <= REAL_TUTORIAS[i].untilDate) {
            return true;
        }
    }

    return false;
}

bool CalendarManager::hasDeadlineOnDate(int day, int month, int year) const {
    for (int i = 0; i < _deadlineCount; i++) {
        if (_deadlines[i].targetDay == day && _deadlines[i].targetMonth == month && _deadlines[i].targetYear == year) {
            return true;
        }
    }
    for (int i = 0; i < NUM_REAL_DEADLINES; i++) {
        if (REAL_DEADLINES[i].day == day && REAL_DEADLINES[i].month == month && REAL_DEADLINES[i].year == year) {
            return true;
        }
    }
    return false;
}

String CalendarManager::getEventSummaryForDate(int day, int month, int year) const {
    // 1. Exámenes y entregas prioritarias
    for (int i = 0; i < _deadlineCount; i++) {
        if (_deadlines[i].targetDay == day && _deadlines[i].targetMonth == month && _deadlines[i].targetYear == year) {
            return "[!] " + _deadlines[i].title;
        }
    }

    // 2. Eventos dinámicos de Google Calendar
    struct tm dTm = {0};
    dTm.tm_year = year - 1900;
    dTm.tm_mon = month - 1;
    dTm.tm_mday = day;
    mktime(&dTm);
    int wday = dTm.tm_wday;

    for (int i = 0; i < _eventCount; i++) {
        if (_events[i].occursOnDate(day, month, year, wday)) {
            if (_events[i].startHour > 0) {
                return _events[i].timeRange.substring(0, 5) + " " + _events[i].title;
            }
            return _events[i].title;
        }
    }

    // 3. Eventos calendarizados reales de la UNED
    for (int i = 0; i < NUM_DATED_EVENTS; i++) {
        if (REAL_DATED_EVENTS[i].year == year && 
            REAL_DATED_EVENTS[i].month == month && 
            REAL_DATED_EVENTS[i].day == day) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%02d:%02d %s",
                     REAL_DATED_EVENTS[i].startHour, REAL_DATED_EVENTS[i].startMin,
                     REAL_DATED_EVENTS[i].title);
            return String(buf);
        }
    }

    // 4. Tutorías semanales UNED
    long targetDate = (long)year * 10000 + month * 100 + day;
    for (int i = 0; i < NUM_TUTORIAS; i++) {
        if (REAL_TUTORIAS[i].wday == wday && targetDate <= REAL_TUTORIAS[i].untilDate) {
            char buf[64];
            snprintf(buf, sizeof(buf), "%02d:%02d %s",
                     REAL_TUTORIAS[i].startHour, REAL_TUTORIAS[i].startMin,
                     REAL_TUTORIAS[i].title);
            return String(buf);
        }
    }

    return "";
}
