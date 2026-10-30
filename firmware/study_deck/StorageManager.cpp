#include "StorageManager.h"
#include <time.h>

void StorageManager::begin() {
    Serial.println("[Storage] Comprobando ranura MicroSD...");
    // Configurar pines de CS
    pinMode(PIN_SD_CS, OUTPUT);
    digitalWrite(PIN_SD_CS, HIGH);

    if (SD.begin(PIN_SD_CS)) {
        _sdAvailable = true;
        uint8_t cardType = SD.cardType();
        if (cardType != CARD_NONE) {
            _cardSizeMB = SD.cardSize() / (1024 * 1024);
            Serial.printf("[Storage] MicroSD montada con éxito (%llu MB).\n", _cardSizeMB);
            
            // Crear cabecera si el archivo no existe
            if (!SD.exists("/study_log.csv")) {
                File f = SD.open("/study_log.csv", FILE_WRITE);
                if (f) {
                    f.println("Timestamp,Fecha_Hora,Tipo_Sesion,Duracion_Minutos,Completada");
                    f.close();
                }
            }
        } else {
            _sdAvailable = false;
            Serial.println("[Storage] Tarjeta no detectada.");
        }
    } else {
        _sdAvailable = false;
        Serial.println("[Storage] Ranura MicroSD vacía o no formateada (Modo offline activo).");
    }
}

void StorageManager::logSession(const char *sessionType, int durationMinutes, bool completed) {
    if (!_sdAvailable) return;

    File f = SD.open("/study_log.csv", FILE_APPEND);
    if (!f) return;

    time_t now = time(nullptr);
    struct tm *t = localtime(&now);
    char dateBuf[32];
    snprintf(dateBuf, sizeof(dateBuf), "%04d-%02d-%02d %02d:%02d",
             t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);

    f.printf("%ld,\"%s\",\"%s\",%d,%s\n",
             now, dateBuf, sessionType, durationMinutes, completed ? "SI" : "NO");
    f.close();
    Serial.printf("[Storage] Sesión registrada en MicroSD: %s (%d min)\n", sessionType, durationMinutes);
}

int StorageManager::getTotalLoggedSessions() {
    if (!_sdAvailable || !SD.exists("/study_log.csv")) return 0;
    File f = SD.open("/study_log.csv", FILE_READ);
    if (!f) return 0;

    int lines = 0;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.length() > 5) lines++;
    }
    f.close();
    return (lines > 0) ? (lines - 1) : 0; // Menos cabecera
}

int StorageManager::getTotalStudyMinutes() {
    if (!_sdAvailable || !SD.exists("/study_log.csv")) return 0;
    File f = SD.open("/study_log.csv", FILE_READ);
    if (!f) return 0;

    int totalMins = 0;
    bool isHeader = true;
    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (isHeader) { isHeader = false; continue; }
        // Parsear duración (columna 4)
        int idx1 = line.indexOf(',');
        if (idx1 < 0) continue;
        int idx2 = line.indexOf(',', idx1 + 1);
        if (idx2 < 0) continue;
        int idx3 = line.indexOf(',', idx2 + 1);
        if (idx3 < 0) continue;
        int idx4 = line.indexOf(',', idx3 + 1);
        if (idx4 < 0) continue;
        String durStr = line.substring(idx3 + 1, idx4);
        totalMins += durStr.toInt();
    }
    f.close();
    return totalMins;
}

