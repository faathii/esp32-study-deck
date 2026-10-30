#include "WiFiConfig.h"

void WiFiConfig::begin() {
    WiFi.mode(WIFI_STA);
    _prefs.begin("study_wifi", false);

    // Si hay credenciales definidas en Secrets.h y no están en NVS, guardarlas y conectar
    if (!hasSavedCredentials() && strlen(SECRET_WIFI_SSID) > 0 && strlen(SECRET_WIFI_PASS) > 0) {
        saveCredentials(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
        startConnect(SECRET_WIFI_SSID, SECRET_WIFI_PASS);
    } else if (hasSavedCredentials()) {
        // Conexión automática con las credenciales guardadas en NVS
        startConnect(getSavedSSID(), getSavedPassword());
    }
}

bool WiFiConfig::hasSavedCredentials() {
    String s = _prefs.getString("ssid", "");
    return (s.length() > 0);
}

String WiFiConfig::getSavedSSID() {
    return _prefs.getString("ssid", "");
}

String WiFiConfig::getSavedPassword() {
    return _prefs.getString("pass", "");
}

void WiFiConfig::saveCredentials(const String &ssid, const String &password) {
    _prefs.putString("ssid", ssid);
    _prefs.putString("pass", password);
    Serial.printf("[WiFi] Credenciales guardadas en NVS: %s\n", ssid.c_str());
}

void WiFiConfig::clearCredentials() {
    _prefs.remove("ssid");
    _prefs.remove("pass");
    WiFi.disconnect(true);
}

int WiFiConfig::scanNetworks() {
    Serial.println("[WiFi] Escaneando redes cercanas...");
    WiFi.disconnect();
    delay(100);

    int n = WiFi.scanNetworks();
    Serial.printf("[WiFi] %d redes encontradas.\n", n);

    _networkCount = (n > 16) ? 16 : n;
    for (int i = 0; i < _networkCount; i++) {
        _networks[i].ssid = WiFi.SSID(i);
        _networks[i].rssi = WiFi.RSSI(i);
        _networks[i].isEncrypted = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }

    WiFi.scanDelete();
    return _networkCount;
}

DiscoveredNetwork WiFiConfig::getNetwork(int index) const {
    if (index >= 0 && index < _networkCount) {
        return _networks[index];
    }
    return DiscoveredNetwork{"", 0, false};
}

void WiFiConfig::startConnect(const String &ssid, const String &password) {
    _currentSSID = ssid;
    _currentPassword = password;
    _connectStartTime = millis();

    Serial.printf("[WiFi] Intentando conectar a '%s'...\n", ssid.c_str());
    WiFi.disconnect();
    WiFi.begin(ssid.c_str(), password.c_str());
}

WiFiConnectStatus WiFiConfig::checkConnectStatus() {
    if (WiFi.status() == WL_CONNECTED) {
        return WIFI_STATUS_CONNECTED;
    }
    // Timeout de 15 segundos
    if (millis() - _connectStartTime > 15000) {
        return WIFI_STATUS_FAILED;
    }
    return WIFI_STATUS_CONNECTING;
}

void WiFiConfig::disconnect() {
    WiFi.disconnect();
}
