#include "CalendarManager.h"

void CalendarManager::begin() {
    _prefs.begin("study_cal", false);
    _gcal.begin();

    // Purgar datos de ejemplo antiguos de versiones previas si existen en NVS
    if (!_prefs.getBool("real_only_v2", false)) {
        _prefs.clear();
        _prefs.putBool("real_only_v2", true);
        _deadlineCount = 0;
        _eventCount = 0;
    } else {
        loadDeadlines();
    }
}

void CalendarManager::loadDefaultEvents() {
    _eventCount = 0; // Cero eventos por defecto. Solo datos reales.
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
        _eventCount = 0; // Sin eventos hoy
        _lastAutoSync = millis();
        return true;
    }

    return false;
}

void CalendarManager::loadDeadlines() {
    _deadlineCount = _prefs.getInt("dl_count", 0);
    if (_deadlineCount <= 0 || _deadlineCount > MAX_DEADLINES) {
        _deadlineCount = 0;
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

    outTitle = _deadlines[0].title;
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    if (t->tm_year <= 120) {
        return "Conecta WiFi";
    }

    struct tm target = {0};
    target.tm_year = _deadlines[0].targetYear - 1900;
    target.tm_mon  = _deadlines[0].targetMonth - 1;
    target.tm_mday = _deadlines[0].targetDay;
    target.tm_hour = _deadlines[0].targetHour;
    target.tm_min  = 0;
    target.tm_sec  = 0;

    time_t targetTime = mktime(&target);
    double diffSeconds = difftime(targetTime, now);

    if (diffSeconds <= 0) {
        return "¡HOY!";
    }

    long days = (long)(diffSeconds / 86400);
    long hours = (long)(((long)diffSeconds % 86400) / 3600);
    long mins = (long)(((long)diffSeconds % 3600) / 60);

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
    return false;
}

bool CalendarManager::hasDeadlineOnDate(int day, int month, int year) const {
    for (int i = 0; i < _deadlineCount; i++) {
        if (_deadlines[i].targetDay == day && _deadlines[i].targetMonth == month && _deadlines[i].targetYear == year) {
            return true;
        }
    }
    return false;
}

String CalendarManager::getEventSummaryForDate(int day, int month, int year) const {
    for (int i = 0; i < _deadlineCount; i++) {
        if (_deadlines[i].targetDay == day && _deadlines[i].targetMonth == month && _deadlines[i].targetYear == year) {
            return "[!] " + _deadlines[i].title;
        }
    }

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
    return "";
}
