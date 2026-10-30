#include "CalendarManager.h"

void CalendarManager::begin() {
    _prefs.begin("study_cal", false);

    // Agenda académica por defecto
    _events[0] = { "Clase de Matematicas", "Aula 204", "09:00 - 11:00", false };
    _events[1] = { "Sesion de Estudio Pomo", "Biblioteca", "16:00 - 18:30", true };
    _events[2] = { "Entrega de Tarea", "Campus Virtual", "20:00", false };

    loadDeadlines();
}

void CalendarManager::loadDeadlines() {
    _deadlineCount = _prefs.getInt("dl_count", -1);
    if (_deadlineCount <= 0 || _deadlineCount > MAX_DEADLINES) {
        // Hitos iniciales
        _deadlineCount = 2;
        _deadlines[0] = { "Examen de Fisica", 15, 11, 2026, 10 };
        _deadlines[1] = { "Entrega Proyecto Final", 28, 11, 2026, 23 };
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

        _deadlines[i].title       = _prefs.getString(kT, "Examen");
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

void CalendarManager::update() {
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    if (t->tm_year > 120) {
        int currentHour = t->tm_hour;
        _events[0].isOngoing = (currentHour >= 9 && currentHour < 11);
        _events[1].isOngoing = (currentHour >= 16 && currentHour < 18);
        _events[2].isOngoing = (currentHour >= 19 && currentHour < 20);
    }
}

CalendarEvent CalendarManager::getEvent(int index) const {
    if (index >= 0 && index < _eventCount) {
        return _events[index];
    }
    return CalendarEvent{ "", "", "", false };
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
        // Reloj aún sin sincronizar por NTP
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
