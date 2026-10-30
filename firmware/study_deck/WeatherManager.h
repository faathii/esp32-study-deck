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
    float getTemperature() const { return _temperature; }
    int getHumidity() const { return _humidity; }
    int getWeatherCode() const { return _weatherCode; }
    String getConditionText() const;
    String getCityName() const { return _cityName; }
    String getFormattedWeather() const;

    void setLocation(float lat, float lon, const String &cityName);
    void forceRefresh(bool wifiConnected);

private:
    float _latitude = 40.4168f;  // Madrid por defecto
    float _longitude = -3.7038f;
    String _cityName = "Madrid";

    bool _hasData = false;
    float _temperature = 18.0f;
    int _humidity = 55;
    int _weatherCode = 0;
    unsigned long _lastFetchTime = 0;
    bool _isFetching = false;

    bool fetchOpenMeteo();
};

#endif // WEATHER_MANAGER_H

