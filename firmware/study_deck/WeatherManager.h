#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

class WeatherManager {
public:
    void begin();
    void update(bool wifiConnected); // Consulta periódica no bloqueante

    bool hasData() const { return _hasData; }
    bool consumeNewData() {
        bool temp = _hasNewData;
        _hasNewData = false;
        return temp;
    }
    float getTemperature() const { return _temperature; }
    int getHumidity() const { return _humidity; }
    int getWeatherCode() const { return _weatherCode; }
    String getConditionText() const;
    String getCityName() const { return _cityName; }
    String getFormattedWeather() const;

    void setLocation(float lat, float lon, const String &cityName);
    void forceRefresh(bool wifiConnected);

private:
    float _latitude = 40.6565f;  // Ávila (40°39'23" N)
    float _longitude = -4.6818f; // Ávila (4°40'54" O)
    String _cityName = "Avila";

    bool _hasData = false;
    bool _hasNewData = false;
    float _temperature = 14.0f;
    int _humidity = 60;
    int _weatherCode = 0;
    unsigned long _lastFetchTime = 0;
    bool _isFetching = false;

    bool fetchOpenMeteo();
};

#endif // WEATHER_MANAGER_H

