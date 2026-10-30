#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include "HardwareConfig.h"

class StorageManager {
public:
    void begin();
    bool isAvailable() const { return _sdAvailable; }
    uint64_t getCardSizeMB() const { return _cardSizeMB; }

    void logSession(const char *sessionType, int durationMinutes, bool completed);
    int getTotalLoggedSessions();
    int getTotalStudyMinutes();

private:
    bool _sdAvailable = false;
    uint64_t _cardSizeMB = 0;
};

#endif // STORAGE_MANAGER_H

