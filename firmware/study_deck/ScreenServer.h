#ifndef SCREEN_SERVER_H
#define SCREEN_SERVER_H

#include <Arduino.h>
#include <FS.h>
using fs::FS;
#include <WebServer.h>
#include <TFT_eSPI.h>

class ScreenServer {
public:
    typedef void (*TouchCallback)(int x, int y);
    typedef void (*NavigateCallback)(int viewId);
    typedef void (*RefreshCallback)();

    ScreenServer(TFT_eSPI &tft) : _tft(tft), _server(80), _touchCb(nullptr), _navCb(nullptr), _refreshCb(nullptr) {}

    void begin(TouchCallback touchCb, NavigateCallback navCb, RefreshCallback refreshCb) {
        _touchCb = touchCb;
        _navCb = navCb;
        _refreshCb = refreshCb;

        _server.on("/", HTTP_GET, [this]() {
            handleRoot();
        });

        _server.on("/screenshot.bmp", HTTP_GET, [this]() {
            handleScreenshotBmp();
        });

        _server.on("/touch", HTTP_GET, [this]() {
            handleTouch();
        });

        _server.on("/navigate", HTTP_GET, [this]() {
            handleNavigate();
        });

        _server.begin();
        Serial.printf("[ScreenServer] Servidor web de capturas iniciado en http://%s/\n", WiFi.localIP().toString().c_str());
    }

    void handleClient() {
        _server.handleClient();
    }

private:
    TFT_eSPI &_tft;
    WebServer _server;
    TouchCallback _touchCb;
    NavigateCallback _navCb;
    RefreshCallback _refreshCb;

    void handleScreenshotBmp() {
        WiFiClient client = _server.client();
        if (!client) return;

        const uint32_t width = 320;
        const uint32_t height = 240;
        const uint32_t rowBytesCount = width * 3; // 960 bytes (múltiplo de 4, sin padding)
        const uint32_t imageSize = rowBytesCount * height; // 230400 bytes
        const uint32_t fileSize = 54 + imageSize; // 230454 bytes

        uint8_t bmpHeader[54] = {
            'B', 'M',
            (uint8_t)(fileSize & 0xFF), (uint8_t)((fileSize >> 8) & 0xFF), (uint8_t)((fileSize >> 16) & 0xFF), (uint8_t)((fileSize >> 24) & 0xFF),
            0, 0, 0, 0, // Reservados
            54, 0, 0, 0, // Offset a los datos de píxeles
            40, 0, 0, 0, // Tamaño DIB header
            (uint8_t)(width & 0xFF), (uint8_t)((width >> 8) & 0xFF), (uint8_t)((width >> 16) & 0xFF), (uint8_t)((width >> 24) & 0xFF),
            (uint8_t)(height & 0xFF), (uint8_t)((height >> 8) & 0xFF), (uint8_t)((height >> 16) & 0xFF), (uint8_t)((height >> 24) & 0xFF),
            1, 0, // Planos de color
            24, 0, // 24 bits por píxel (RGB888)
            0, 0, 0, 0, // Compresión BI_RGB (0)
            (uint8_t)(imageSize & 0xFF), (uint8_t)((imageSize >> 8) & 0xFF), (uint8_t)((imageSize >> 16) & 0xFF), (uint8_t)((imageSize >> 24) & 0xFF),
            0x13, 0x0B, 0, 0, // 2835 ppm horiz
            0x13, 0x0B, 0, 0, // 2835 ppm vert
            0, 0, 0, 0, 0, 0, 0, 0
        };

        _server.sendHeader("Content-Disposition", "inline; filename=screenshot.bmp");
        _server.sendHeader("Access-Control-Allow-Origin", "*");
        _server.setContentLength(fileSize);
        _server.send(200, "image/bmp", "");

        client.write(bmpHeader, sizeof(bmpHeader));

        uint16_t rowBuffer[width];
        uint8_t rowBytes[rowBytesCount];

        // BMP se serializa de abajo hacia arriba (y = 239 .. 0)
        for (int y = (int)height - 1; y >= 0; y--) {
            _tft.readRect(0, y, width, 1, rowBuffer);
            for (uint32_t x = 0; x < width; x++) {
                uint16_t raw = rowBuffer[x];
                uint16_t c = (uint16_t)((raw >> 8) | (raw << 8));
                // Formato BMP: B, G, R
                rowBytes[x * 3 + 0] = (uint8_t)((c & 0x001F) << 3);          // Azul
                rowBytes[x * 3 + 1] = (uint8_t)(((c >> 5) & 0x003F) << 2);  // Verde
                rowBytes[x * 3 + 2] = (uint8_t)(((c >> 11) & 0x001F) << 3); // Rojo
            }
            client.write(rowBytes, rowBytesCount);
        }
    }

    void handleRoot() {
        String html = F(
            "<!DOCTYPE html><html><head><meta charset='utf-8'>"
            "<meta name='viewport' content='width=device-width, initial-scale=1'>"
            "<title>StudyDeck 2.4\" Control Remoto</title>"
            "<style>"
            "body{background:#0d1117;color:#c9d1d9;font-family:system-ui,sans-serif;margin:0;padding:20px;text-align:center;}"
            "h1{color:#58a6ff;margin-bottom:6px;font-size:22px;}"
            "p.sub{color:#8b949e;margin-top:0;font-size:13px;}"
            ".screen-box{display:inline-block;background:#000;padding:8px;border-radius:12px;border:2px solid #30363d;box-shadow:0 8px 24px rgba(0,0,0,0.6);margin-bottom:14px;}"
            "#screen{width:320px;height:240px;image-rendering:pixelated;cursor:crosshair;display:block;border-radius:4px;}"
            ".btn-row{margin:6px 0;display:flex;flex-wrap:wrap;justify-content:center;gap:6px;max-width:560px;margin-left:auto;margin-right:auto;}"
            "button{background:#21262d;color:#f0f6fc;border:1px solid #30363d;padding:8px 12px;border-radius:6px;cursor:pointer;font-size:13px;font-weight:600;transition:0.15s;}"
            "button:hover{background:#30363d;border-color:#8b949e;}"
            "button.accent{background:#238636;border-color:#2ea043;color:#fff;}"
            "button.accent:hover{background:#2ea043;}"
            "#status{font-size:12px;color:#7ee787;margin-top:8px;height:18px;}"
            "</style></head><body>"
            "<h1>StudyDeck ESP32 &bull; Captura y Control</h1>"
            "<p class='sub'>Haz clic sobre la pantalla para interactuar táctilmente</p>"
            "<div class='screen-box'>"
            "<img id='screen' src='/screenshot.bmp' alt='Pantalla ESP32' onclick='onScreenClick(event)'>"
            "</div><br>"
            "<div class='btn-row'>"
            "<button class='accent' onclick='refreshScreen()'>Capturar Pantalla</button>"
            "<a href='/screenshot.bmp' download='studydeck.bmp'><button>Descargar BMP</button></a>"
            "</div>"
            "<div class='btn-row'>"
            "<button onclick=\"nav('weather')\">Clima (Standby)</button>"
            "<button onclick=\"nav('home')\">Menú Apps</button>"
            "<button onclick=\"nav('pomodoro')\">Pomodoro</button>"
            "<button onclick=\"nav('agenda')\">Agenda UNED</button>"
            "<button onclick=\"nav('night')\">Reloj Noche</button>"
            "<button onclick=\"nav('relax')\">Relax</button>"
            "<button onclick=\"nav('lamp')\">Lámpara</button>"
            "<button onclick=\"nav('battery')\">Batería</button>"
            "<button onclick=\"nav('stats')\">Estadísticas</button>"
            "<button onclick=\"nav('settings')\">Ajustes</button>"
            "</div>"
            "<div id='status'>Listo</div>"
            "<script>"
            "function refreshScreen(){"
            "  var img=document.getElementById('screen');"
            "  var s=document.getElementById('status');"
            "  s.innerText='Actualizando captura...';"
            "  img.src='/screenshot.bmp?t='+Date.now();"
            "  img.onload=function(){s.innerText='Captura actualizada';setTimeout(function(){s.innerText='Listo';},1500);};"
            "}"
            "function onScreenClick(e){"
            "  var rect=e.target.getBoundingClientRect();"
            "  var scaleX=320/rect.width;"
            "  var scaleY=240/rect.height;"
            "  var x=Math.round((e.clientX-rect.left)*scaleX);"
            "  var y=Math.round((e.clientY-rect.top)*scaleY);"
            "  var s=document.getElementById('status');"
            "  s.innerText='Toque en ('+x+','+y+')...';"
            "  fetch('/touch?x='+x+'&y='+y).then(function(){"
            "    setTimeout(refreshScreen, 150);"
            "  });"
            "}"
            "function nav(v){"
            "  var s=document.getElementById('status');"
            "  s.innerText='Cambiando vista a '+v+'...';"
            "  fetch('/navigate?view='+v).then(function(){"
            "    setTimeout(refreshScreen, 150);"
            "  });"
            "}"
            "</script>"
            "</body></html>"
        );
        _server.send(200, "text/html", html);
    }

    void handleTouch() {
        if (_server.hasArg("x") && _server.hasArg("y") && _touchCb) {
            int x = _server.arg("x").toInt();
            int y = _server.arg("y").toInt();
            _touchCb(x, y);
            _server.send(200, "text/plain", "OK");
        } else {
            _server.send(400, "text/plain", "Missing x/y");
        }
    }

    void handleNavigate() {
        if (_server.hasArg("view") && _navCb) {
            String v = _server.arg("view");
            int targetView = -1;
            if (v == "weather") targetView = 0;
            else if (v == "home") targetView = 1;
            else if (v == "pomodoro") targetView = 2;
            else if (v == "agenda") targetView = 3;
            else if (v == "settings") targetView = 4;
            else if (v == "info") targetView = 5;
            else if (v == "night") targetView = 6;
            else if (v == "relax") targetView = 7;
            else if (v == "lamp") targetView = 8;
            else if (v == "battery") targetView = 9;
            else if (v == "stats") targetView = 10;
            else if (v == "wifi") targetView = 12;

            if (targetView >= 0) {
                _navCb(targetView);
                _server.send(200, "text/plain", "OK");
                return;
            }
        }
        _server.send(400, "text/plain", "Invalid view");
    }
};

#endif // SCREEN_SERVER_H

