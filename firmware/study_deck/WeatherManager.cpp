#include "WeatherManager.h"

void WeatherManager::begin() {
    _lastFetchTime = 0;
    _hasData = false;
}

void WeatherManager::setLocation(float lat, float lon, const String &cityName) {
    _latitude = lat;
    _longitude = lon;
    _cityName = cityName;
    _hasData = false;
}

void WeatherManager::update(bool wifiConnected) {
    if (!wifiConnected) return;

    unsigned long now = millis();
    // Reintentar tras 60s si falló o cada 15 min (900000ms) si ya tiene datos
    unsigned long interval = _hasData ? 900000 : 60000;
    if (_lastFetchTime == 0 || (now - _lastFetchTime >= interval)) {
        forceRefresh(wifiConnected);
    }
}

void WeatherManager::forceRefresh(bool wifiConnected) {
    if (!wifiConnected || _isFetching) return;
    _isFetching = true;
    _lastFetchTime = millis();

    if (fetchOpenMeteo()) {
        _hasData = true;
    }
    _isFetching = false;
}

bool WeatherManager::fetchOpenMeteo() {
    WiFiClientSecure client;
    client.setInsecure(); // SSL sin verificación de CA para no saturar memoria del ESP32

    HTTPClient http;
    char url[180];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f&current=temperature_2m,relative_humidity_2m,weather_code",
             _latitude, _longitude);

    Serial.printf("[Weather] Consultando Open-Meteo: %s\n", url);
    http.begin(client, url);
    http.setTimeout(5000);

    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        int curIdx = payload.indexOf("\"current\":");
        if (curIdx != -1) {
            int tempIdx = payload.indexOf("\"temperature_2m\":", curIdx);
            if (tempIdx != -1) {
                int commaIdx = payload.indexOf(',', tempIdx);
                if (commaIdx != -1) {
                    _temperature = payload.substring(tempIdx + 17, commaIdx).toFloat();
                }
            }

            int humIdx = payload.indexOf("\"relative_humidity_2m\":", curIdx);
            if (humIdx != -1) {
                int commaIdx = payload.indexOf(',', humIdx);
                if (commaIdx != -1) {
                    _humidity = payload.substring(humIdx + 23, commaIdx).toInt();
                }
            }

            int codeIdx = payload.indexOf("\"weather_code\":", curIdx);
            if (codeIdx != -1) {
                int c1 = payload.indexOf(',', codeIdx);
                int c2 = payload.indexOf('}', codeIdx);
                int endIdx = (c1 != -1 && (c2 == -1 || c1 < c2)) ? c1 : c2;
                if (endIdx != -1) {
                    _weatherCode = payload.substring(codeIdx + 15, endIdx).toInt();
                }
            }

            Serial.printf("[Weather] Datos recibidos: %.1f C, %d%% humedad, WMO %d\n",
                          _temperature, _humidity, _weatherCode);
            http.end();
            return true;
        }
    } else {
        Serial.printf("[Weather] Error HTTP: %d\n", httpCode);
    }

    http.end();
    return false;
}

String WeatherManager::getConditionText() const {
    switch (_weatherCode) {
        case 0: return "Despejado";
        case 1:
        case 2: return "Parc. Nublado";
        case 3: return "Nublado";
        case 45:
        case 48: return "Niebla";
        case 51:
        case 53:
        case 55: return "Llovizna";
        case 61:
        case 63:
        case 65: return "Lluvia";
        case 71:
        case 73:
        case 75: return "Nieve";
        case 80:
        case 81:
        case 82: return "Chubascos";
        case 95:
        case 96:
        case 99: return "Tormenta";
        default: return "Despejado";
    }
}

String WeatherManager::getFormattedWeather() const {
    if (!_hasData) return "-- C (Sin datos)";
    char buf[32];
    snprintf(buf, sizeof(buf), "%.1f C %s", _temperature, getConditionText().c_str());
    return String(buf);
}

