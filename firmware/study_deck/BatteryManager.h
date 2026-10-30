#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>
#include "HardwareConfig.h"

class BatteryManager {
public:
    BatteryManager(uint8_t adcPin = PIN_BATTERY);

    void begin();
    void update(); // Llamar periódicamente en loop() cada 500-1000ms

    float getVoltage() const { return _smoothedVoltage; }
    int getPercentage() const { return _percentage; }
    bool isCharging() const { return _isCharging; }
    bool isLow() const { return _percentage <= 20; }
    bool isCritical() const { return _percentage <= 10; }
    uint16_t getStatusColor() const; // Color TFT: Verde, Ámbar o Rojo
    String getFormattedText() const;

private:
    uint8_t _pin;
    float _smoothedVoltage = 3.95f;
    int _percentage = 85;
    bool _isCharging = false;
    unsigned long _lastReadTime = 0;

    float readRawVoltage();
    int calculatePercentage(float v);
};

#endif // BATTERY_MANAGER_H

