#include "BatteryManager.h"

// Colores TFT estándar
#define COLOR_BAT_GOOD   0x05E6 // Verde esmeralda
#define COLOR_BAT_MEDIUM 0xFDA0 // Ámbar / Amarillo
#define COLOR_BAT_LOW    0xF986 // Rojo alerta

BatteryManager::BatteryManager(uint8_t adcPin) : _pin(adcPin) {}

void BatteryManager::begin() {
    pinMode(_pin, INPUT);
    analogSetPinAttenuation(_pin, ADC_11db); // Rango de medición hasta ~3.3V

    // Cargar calibración previa de NVS
    _prefs.begin("study_bat", false);
    _calOffset = _prefs.getFloat("cal_offset", 0.0f);

    // Muestreo inicial estabilizado
    float initialV = 0.0f;
    for (int i = 0; i < 16; i++) {
        initialV += readRawVoltage();
        delay(5);
    }
    _smoothedVoltage = initialV / 16.0f;
    _percentage = calculatePercentage(_smoothedVoltage + _calOffset);
}

float BatteryManager::readRawVoltage() {
    uint32_t sum = 0;
    const int samples = 16;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(_pin);
        delayMicroseconds(100);
    }
    float raw = (float)sum / (float)samples;

    // Conversión ADC (12 bits -> 0..4095 a 3.3V)
    // Divisor de tensión estándar de placa (proporción 1:2)
    float pinVoltage = (raw / 4095.0f) * 3.3f;
    float batteryVoltage = pinVoltage * 2.0f;

    return batteryVoltage;
}

int BatteryManager::calculatePercentage(float v) {
    // Si la lectura en GPIO 35 es inferior a 1.5V, significa que el pin está flotante o a masa
    // porque la placa no tiene divisor resistivo conectado a una batería. Está por USB continuo.
    if (v < 1.5f) {
        _hasBatterySense = false;
        _isUsbPowered = true;
        _isCharging = false;
        return -1; // Modo USB 5V fijo
    }

    _hasBatterySense = true;

    // Tensión de celda LiPo superior a 4.22V indica cargador USB activo cargando la batería
    if (v >= 4.22f) {
        _isCharging = true;
        _isUsbPowered = true;
        return 100;
    }

    _isCharging = false;
    _isUsbPowered = false;

    // Curva de descarga característica para celda LiPo 1S de 5000 mAh (3.7V / 18.5Wh)
    if (v >= 4.18f) return 100;
    if (v >= 4.10f) return 90 + (int)((v - 4.10f) / 0.08f * 10);
    if (v >= 4.00f) return 80 + (int)((v - 4.00f) / 0.10f * 10);
    if (v >= 3.90f) return 68 + (int)((v - 3.90f) / 0.10f * 12);
    if (v >= 3.82f) return 55 + (int)((v - 3.82f) / 0.08f * 13);
    if (v >= 3.74f) return 40 + (int)((v - 3.74f) / 0.08f * 15);
    if (v >= 3.66f) return 25 + (int)((v - 3.66f) / 0.08f * 15);
    if (v >= 3.55f) return 12 + (int)((v - 3.55f) / 0.11f * 13);
    if (v >= 3.40f) return 4  + (int)((v - 3.40f) / 0.15f * 8);
    if (v >= 3.25f) return 1  + (int)((v - 3.25f) / 0.15f * 3);
    return 0;
}

void BatteryManager::update() {
    unsigned long now = millis();
    if (now - _lastReadTime >= 1000) {
        _lastReadTime = now;

        float instantV = readRawVoltage();
        _smoothedVoltage = (_smoothedVoltage * 0.85f) + (instantV * 0.15f);

        int targetPct = calculatePercentage(_smoothedVoltage + _calOffset);
        _percentage = targetPct;
    }
}

float BatteryManager::getEstimatedHoursLeft() const {
    if (!_hasBatterySense || _percentage < 0) return 99.0f; // Alimentado por USB continuo
    float remainingMah = ((float)_percentage / 100.0f) * (float)BATTERY_CAPACITY_MAH;
    return remainingMah / BATTERY_AVG_DRAW_MA;
}

uint16_t BatteryManager::getStatusColor() const {
    if (!_hasBatterySense || _percentage < 0) return 0x07E0; // Verde USB continuo
    if (_percentage > 40) return COLOR_BAT_GOOD;
    if (_percentage > 18) return COLOR_BAT_MEDIUM;
    return COLOR_BAT_LOW;
}

String BatteryManager::getFormattedText() const {
    if (!_hasBatterySense || _percentage < 0) {
        return "USB 5V";
    }
    if (_isCharging) {
        return String(_percentage) + "% [CARGA]";
    }
    return String(_percentage) + "%";
}

String BatteryManager::getRuntimeString() const {
    if (!_hasBatterySense || _percentage < 0) {
        return "Alimentacion continua USB 5V";
    }
    if (_isCharging) {
        return "Cargando bateria LiPo...";
    }
    float hours = getEstimatedHoursLeft();
    int h = (int)hours;
    int m = (int)((hours - h) * 60);
    char buf[28];
    snprintf(buf, sizeof(buf), "~%dh %02dm restantes", h, m);
    return String(buf);
}

void BatteryManager::setCalibrationOffset(float offset) {
    _calOffset = offset;
    _prefs.putFloat("cal_offset", _calOffset);
    _percentage = calculatePercentage(_smoothedVoltage + _calOffset);
}
