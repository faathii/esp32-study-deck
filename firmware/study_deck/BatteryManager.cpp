#include "BatteryManager.h"

// Colores TFT estándar
#define COLOR_BAT_GOOD   0x05E6 // Verde esmeralda
#define COLOR_BAT_MEDIUM 0xFDA0 // Ámbar / Amarillo
#define COLOR_BAT_LOW    0xF986 // Rojo alerta

BatteryManager::BatteryManager(uint8_t adcPin) : _pin(adcPin) {}

void BatteryManager::begin() {
    pinMode(_pin, INPUT);
    analogSetPinAttenuation(_pin, ADC_11db); // Rango hasta ~3.3V
    
    // Lectura inicial múltiple para asentar el filtro
    float initialV = 0.0f;
    for (int i = 0; i < 8; i++) {
        initialV += readRawVoltage();
        delay(5);
    }
    _smoothedVoltage = initialV / 8.0f;
    _percentage = calculatePercentage(_smoothedVoltage);
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
    // El divisor de tensión en placa divide el voltaje LiPo aproximadamente por 2 (100k/100k)
    float pinVoltage = (raw / 4095.0f) * 3.3f;
    float batteryVoltage = pinVoltage * 2.0f;

    return batteryVoltage;
}

int BatteryManager::calculatePercentage(float v) {
    // Si la lectura es anormalmente baja (ej. pin flotante o sin batería conectada),
    // asumimos funcionamiento por alimentación USB fija
    if (v < 2.5f) {
        _isCharging = true;
        return 100;
    }

    if (v >= 4.25f) {
        _isCharging = true;
        return 100;
    }

    _isCharging = false;

    // Curva de descarga característica de batería LiPo (1S 3.7V / 4.2V máx)
    if (v >= 4.15f) return 100;
    if (v >= 4.05f) return 90 + (int)((v - 4.05f) / 0.10f * 10);
    if (v >= 3.92f) return 75 + (int)((v - 3.92f) / 0.13f * 15);
    if (v >= 3.82f) return 55 + (int)((v - 3.82f) / 0.10f * 20);
    if (v >= 3.74f) return 35 + (int)((v - 3.74f) / 0.08f * 20);
    if (v >= 3.65f) return 20 + (int)((v - 3.65f) / 0.09f * 15);
    if (v >= 3.50f) return 8  + (int)((v - 3.50f) / 0.15f * 12);
    if (v >= 3.30f) return 1  + (int)((v - 3.30f) / 0.20f * 7);
    return 0;
}

void BatteryManager::update() {
    unsigned long now = millis();
    if (now - _lastReadTime >= 1000) {
        _lastReadTime = now;

        float instantV = readRawVoltage();
        // Filtro exponencial para evitar fluctuaciones y ruido del ADC
        _smoothedVoltage = (_smoothedVoltage * 0.85f) + (instantV * 0.15f);

        int targetPct = calculatePercentage(_smoothedVoltage);
        // Suavizado de porcentaje
        if (abs(targetPct - _percentage) >= 1) {
            _percentage = targetPct;
        }
    }
}

uint16_t BatteryManager::getStatusColor() const {
    if (_percentage > 40) return COLOR_BAT_GOOD;
    if (_percentage > 18) return COLOR_BAT_MEDIUM;
    return COLOR_BAT_LOW;
}

String BatteryManager::getFormattedText() const {
    if (_isCharging) {
        return "CARGANDO";
    }
    return String(_percentage) + "%";
}

