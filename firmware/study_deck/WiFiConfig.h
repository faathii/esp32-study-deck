#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

#include <Arduino.h>
#include <WiFi.h>
#include <Preferences.h>

struct DiscoveredNetwork {
    String ssid;
    int32_t rssi;
    bool isEncrypted;
};

enum WiFiConnectStatus {
    WIFI_STATUS_DISCONNECTED,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_FAILED
};

class WiFiConfig {
public:
    void begin();
    
    // Almacenamiento en memoria no volátil (NVS)
    bool hasSavedCredentials();
    String getSavedSSID();
    String getSavedPassword();
    void saveCredentials(const String &ssid, const String &password);
    void clearCredentials();

    // Escaneo de redes
    int scanNetworks();
    int getNetworkCount() const { return _networkCount; }
    DiscoveredNetwork getNetwork(int index) const;

    // Conexión
    void startConnect(const String &ssid, const String &password);
    WiFiConnectStatus checkConnectStatus();
    void disconnect();

    bool isConnected() const { return WiFi.status() == WL_CONNECTED; }
    String getSSID() const { return WiFi.SSID(); }
    String getLocalIP() const { return WiFi.localIP().toString(); }
    int32_t getRSSI() const { return WiFi.RSSI(); }

private:
    Preferences _prefs;
    String _currentSSID;
    String _currentPassword;
    int _networkCount = 0;
    DiscoveredNetwork _networks[16]; // Hasta 16 redes encontradas
    unsigned long _connectStartTime = 0;
};

#endif // WIFI_CONFIG_H

