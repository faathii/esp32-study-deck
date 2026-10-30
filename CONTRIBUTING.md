# Guía de Contribución (Contributing Guidelines)

¡Gracias por tu interés en colaborar en el proyecto **ESP32 Study Deck**! Este es un proyecto de código abierto y cualquier contribución para mejorar el rendimiento, añadir nuevas funciones o pulir la documentación es bienvenida.

---

## 📜 Código de Conducta

Al participar en este proyecto, te comprometes a respetar y seguir nuestro [Código de Conducta](CODE_OF_CONDUCT.md).

---

## 🛠️ ¿Cómo contribuir?

### 1. Reportar un error (Bug Report)
Si encuentras un fallo o un comportamiento inesperado:
* Comprueba si el error ya ha sido reportado en la sección de [Issues](https://github.com/faathii/esp32-study-deck/issues).
* Si es nuevo, abre un issue detallando:
  * Revisión exacta de tu placa (p. ej. `ESP32-2432S024C` capacitiva o `R` resistiva).
  * Versión del núcleo de ESP32 y librerías utilizadas.
  * Pasos claros para reproducir el fallo.
  * Salida del monitor serie a 115200 baudios.

### 2. Proponer una nueva funcionalidad (Feature Request)
* Si tienes una idea para una nueva tarjeta, widget o integración, abre un issue para discutirla antes de escribir código extenso.

### 3. Enviar una Pull Request (PR)
1. Haz un **Fork** del repositorio.
2. Crea una rama descriptiva para tu función o corrección:
   ```bash
   git checkout -b feat/nueva-funcionalidad
   # o
   git checkout -b fix/correccion-error
   ```
3. Desarrolla tus cambios siguiendo las normas de estilo de código.
4. Asegúrate de que el código compila limpiamente sin advertencias críticas mediante `arduino-cli`.
5. Realiza commits claros siguiendo la convención de [Conventional Commits](https://www.conventionalcommits.org/):
   * `feat: ...` para nuevas funcionalidades.
   * `fix: ...` para corrección de errores.
   * `docs: ...` para cambios en la documentación.
   * `refactor: ...` para mejoras de código sin cambio de comportamiento.
6. Abre la Pull Request hacia la rama `main`.

---

## 📐 Estándares de Código

* **Diseño no bloqueante:** Queda estrictamente prohibido el uso de `delay()` en el bucle principal (`loop()`). Se debe utilizar `millis()` y máquinas de estados para temporizadores y lectura de periféricos.
* **Nombres:**
  * Variables y funciones en `camelCase` (p. ej. `updateTimerDisplay()`).
  * Constantes y macros de pines en `SCREAMING_SNAKE_CASE` (p. ej. `PIN_BACKLIGHT`).
  * Clases en `PascalCase` (p. ej. `PomodoroEngine`).
* **Documentación:** Todo archivo nuevo debe incluir comentarios claros en la cabecera explicando su responsabilidad y autoría.

