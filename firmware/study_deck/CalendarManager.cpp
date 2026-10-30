#include "CalendarManager.h"

void CalendarManager::begin() {
    // Agenda académica por defecto
    _events[0] = { "Clase de Matematicas", "Aula 204", "09:00 - 11:00", false };
    _events[1] = { "Sesion de Estudio Pomo", "Biblioteca", "16:00 - 18:30", true };
    _events[2] = { "Entrega de Tarea", "Campus Virtual", "20:00", false };

    // Hitos y fechas de exámenes próximos
    _deadlines[0] = { "Examen de Fisica", 15, 11, 2026, 10 };
    _deadlines[1] = { "Entrega Proyecto Final", 28, 11, 2026, 23 };
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

String CalendarManager::getNextDeadlineCountdown(String &outTitle) {
    if (_deadlineCount == 0) {
        outTitle = "Sin examenes";
        return "--";
    }

    outTitle = _deadlines[0].title;
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    if (t->tm_year <= 120) {
        // Sin hora sincronizada aún
        return "16 dias (aprox)";
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

