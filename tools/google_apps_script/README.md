# ☁️ Herramientas Cloud — Google Apps Script Proxy

Esta carpeta contendrá el script ligero de **Google Apps Script** que servirá como intermediario entre la API de **Google Calendar / Google Tasks** y el ESP32.

---

## 🎯 ¿Por qué un proxy ligero?

La autenticación directa OAuth2 y el procesamiento de grandes respuestas JSON con decenas de campos de Google Calendar pueden saturar la memoria RAM del microcontrolador ESP32. 

Mediante este script web app:
1. El script se ejecuta en los servidores de Google con tu cuenta.
2. Filtra únicamente los eventos del día de hoy y tareas pendientes.
3. Devuelve un **JSON ultra-compacto** (pocos bytes) con:
   - Nombre de la tarea / clase.
   - Hora de inicio y fin.
   - Tiempo restante en minutos.
4. El ESP32 consume este JSON de manera rápida, segura y sin gastar recursos de memoria.

