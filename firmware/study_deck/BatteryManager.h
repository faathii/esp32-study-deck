#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>
#include "HardwareConfig.h"

// Perfil de batería: Celda LiPo 1S de 5000 mAh, 3.7V nominal, 18.5 Wh
#define BATTERY_CAPACITY_MAH   5000
#define BATTERY_ENERGY_WH      18.5f
#define BATTERY_NOMINAL_VOLT   3.7f
#define BATTERY_AVG_DRAW_MA    170.0f // Consumo medio estimado con pantalla y CPU activa

class BatteryManager {
public:
    BatteryManager(uint8_t adcPin = PIN_BATTERY);

    void begin();
    void update(); // Llamar periódicamente en loop() cada 1000ms

    float getVoltage() const { return _smoothedVoltage + _calOffset; }
    int getPercentage() const { return _percentage; }
    int getRemainingCapacityMah() const { 
        if (_percentage < 0) return BATTERY_CAPACITY_MAH;
        return (_percentage * BATTERY_CAPACITY_MAH) / 100; 
    }
    float getEstimatedHoursLeft() const;
    bool isCharging() const { return _isCharging; }
    bool isUsbPowered() const { return _isUsbPowered; }
    bool hasBatterySense() const { return _hasBatterySense; }
    bool isLow() const { return (_hasBatterySense && _percentage <= 20); }
    bool isCritical() const { return (_hasBatterySense && _percentage <= 10); }
    uint16_t getStatusColor() const; // Color TFT: Verde, Ámbar o Rojo
    String getFormattedText() const;
    String getRuntimeString() const;

    // Calibración fina de tensión (guardada en NVS)
    void setCalibrationOffset(float offset);
    float getCalibrationOffset() const { return _calOffset; }

private:
    uint8_t _pin;
    float _smoothedVoltage = 0.0f;
    int _percentage = -1; // -1 = USB 5V fijo
    bool _isCharging = false;
    bool _isUsbPowered = true;
    bool _hasBatterySense = false;
    unsigned long _lastReadTime = 0;
    float _calOffset = 0.0f;
    Preferences _prefs;

    float readRawVoltage();
    int calculatePercentage(float v);
};

#endif // BATTERY_MANAGER_H
