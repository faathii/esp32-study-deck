# Registro de Cambios (Changelog)

Todos los cambios notables en este proyecto se documentarán en este archivo.

El formato se basa en [Keep a Changelog](https://keepachangelog.com/es-ES/1.0.0/) y este proyecto se adhiere a [Semantic Versioning](https://semver.org/lang/es/).

---

## [Unreleased]
### Added
- Estructura modular del proyecto con estándares de código abierto.
- Especificaciones de arquitectura de software para la estación de estudio en `docs/architecture.md`.

---

## [0.1.0] - 2026-10-30
### Added
- Validación completa del hardware para la placa **Guition ESP32-2432S024** (variante capacitiva CST820).
- Driver oficial de inicialización y lectura para táctil capacitivo CST820 en bus I2C (`GPIO 33/32/25/21`).
- Mapeo verificado de periféricos integrados:
  - Pantalla TFT ILI9341 (240x320 px) en bus HSPI.
  - Retroiluminación PWM en GPIO 27 mediante MOSFET AO3402.
  - LED RGB integrado en GPIOs 4 (Rojo), 16 (Verde) y 17 (Azul) activo en bajo.
  - Fotorresistencia analógica LDR en GPIO 34 para medición lumínica.
  - Zumbador / Altavoz en GPIO 26 mediante amplificador SC8002B.
- Sketch de diagnóstico interactivo de hardware en `firmware/test_hardware/`.
- Hoja de ruta funcional del proyecto en `PROYECTO_ESTUDIO.md`.

