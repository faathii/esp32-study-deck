// ============================================================================
// AcademicCalendarData.h
// Calendario Oficial UNED - Curso 2026/2027 (Grado en Ingenieria Informatica)
// Sincronizado y generado directamente desde el feed oficial de Google Calendar
// ============================================================================
#ifndef ACADEMIC_CALENDAR_DATA_H
#define ACADEMIC_CALENDAR_DATA_H

#include <Arduino.h>

struct AcademicEventDef {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t startHour;
    uint8_t startMin;
    uint8_t endHour;
    uint8_t endMin;
    const char *title;
    const char *location;
};

struct TutoriaDef {
    uint8_t wday; // 0=Dom, 1=Lun, 2=Mar, 3=Mie, 4=Jue, 5=Vie, 6=Sab
    uint8_t startHour;
    uint8_t startMin;
    uint8_t endHour;
    uint8_t endMin;
    long untilDate; // YYYYMMDD
    const char *title;
};

struct DeadlineDef {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    const char *title;
};

// Tutorias semanales recurrentes UNED (Martes, Miercoles, Jueves hasta 22/01/2027)
const int NUM_TUTORIAS = 3;
const TutoriaDef REAL_TUTORIAS[NUM_TUTORIAS] = {
    { 2, 20,  0, 21,  0, 20270122, "Tutoria: Sist. Digitales" },
    { 3, 18, 15, 19, 15, 20270122, "Tutoria: Programacion C" },
    { 4, 18,  0, 19,  0, 20270122, "Tutoria: Logica Discreta" }
};

// Hitos academicos, PECs y Examenes Oficiales UNED Avila
const int NUM_REAL_DEADLINES = 7;
const DeadlineDef REAL_DEADLINES[NUM_REAL_DEADLINES] = {
    { 2026, 11, 28, 23, 55, "Entrega PEC 1: Sist. Digitales" },
    { 2027,  1, 10, 23, 55, "Entrega PEC 2: Sist. Digitales" },
    { 2027,  1, 13,  9,  0, "Entrega 4a Practica C (Tutor)" },
    { 2027,  1, 24, 23, 55, "Cierre Tests PEC: Logica" },
    { 2027,  1, 25, 10, 30, "EXAMEN: Sist. Digitales (Avila)" },
    { 2027,  1, 29, 15,  0, "EXAMEN: Logica Discreta (Avila)" },
    { 2027,  2,  9,  8,  0, "EXAMEN: Programacion C (Avila)" }
};

// Sesiones de estudio, simulacros y practicas calendarizadas
const int NUM_DATED_EVENTS = 76;
const AcademicEventDef REAL_DATED_EVENTS[NUM_DATED_EVENTS] = {
    { 2026, 10, 29, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 1", "" },
    { 2026, 10, 30, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 1", "" },
    { 2026, 11, 2, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 2 (Practica 1)", "" },
    { 2026, 11, 3, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 2", "" },
    { 2026, 11, 4, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 2 (Teoria)", "" },
    { 2026, 11, 5, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 2", "" },
    { 2026, 11, 6, 9, 30, 13, 30, "Simulacro Acumulativo Semanas 12 (4h)", "" },
    { 2026, 11, 9, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 3 (Practica 2)", "" },
    { 2026, 11, 10, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 3", "" },
    { 2026, 11, 11, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 3 (Teoria)", "" },
    { 2026, 11, 12, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 3", "" },
    { 2026, 11, 13, 9, 30, 13, 30, "Simulacro Acumulativo Semanas 13 (4h)", "" },
    { 2026, 11, 16, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 4 (PEC 1)", "" },
    { 2026, 11, 17, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 4 (PEC 1)", "" },
    { 2026, 11, 18, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 4", "" },
    { 2026, 11, 19, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 4", "" },
    { 2026, 11, 20, 9, 30, 13, 30, "Simulacro Acumulativo Semanas 14 (4h)", "" },
    { 2026, 11, 23, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 5 (Entrega", "" },
    { 2026, 11, 24, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 5", "" },
    { 2026, 11, 25, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 5", "" },
    { 2026, 11, 26, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 5 (PEC Bloque", "" },
    { 2026, 11, 27, 9, 30, 13, 30, "Simulacro Acumulativo Semanas 15 (4h)", "" },
    { 2026, 11, 28, 9, 0, 10, 30, "FECHA LIMITE (23:55h): Entrega PEC 1 - Fundamentos de Sistemas D", "" },
    { 2026, 11, 30, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 6 (Practica 3)", "" },
    { 2026, 12, 1, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 6", "" },
    { 2026, 12, 2, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 6 (Teoria)", "" },
    { 2026, 12, 3, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 6", "" },
    { 2026, 12, 4, 9, 30, 13, 30, "Simulacro Acumulativo Semanas 16 (4h)", "" },
    { 2026, 12, 7, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 7 (Practica 3)", "" },
    { 2026, 12, 8, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 7 (Automata", "" },
    { 2026, 12, 9, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 7 (Teoria)", "" },
    { 2026, 12, 10, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 7", "" },
    { 2026, 12, 11, 9, 30, 13, 30, "Simulacro Acumulativo Semanas 17 (4h)", "" },
    { 2026, 12, 14, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 8 (TADs)", "" },
    { 2026, 12, 15, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 8", "" },
    { 2026, 12, 16, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 8 (TADs)", "" },
    { 2026, 12, 17, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 8", "" },
    { 2026, 12, 18, 9, 30, 13, 30, "Simulacro Acumulativo Semanas 18 (4h)", "" },
    { 2026, 12, 21, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 9 (PEC 2)", "" },
    { 2026, 12, 22, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 9 (Memorias", "" },
    { 2026, 12, 23, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 9", "" },
    { 2026, 12, 24, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 9", "" },
    { 2026, 12, 28, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 10 (Practica 4)", "" },
    { 2026, 12, 29, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 10 (PEC 2)", "" },
    { 2026, 12, 30, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 10 (Practica 4)", "" },
    { 2026, 12, 31, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 10 (PEC Bloqu", "" },
    { 2027, 1, 4, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 11 (Entrega", "" },
    { 2027, 1, 5, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 11 (Examene", "" },
    { 2027, 1, 6, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 11 (Entrega Prac", "" },
    { 2027, 1, 7, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 11 (Asegurar", "" },
    { 2027, 1, 8, 9, 30, 13, 30, "Simulacro Oficial Convocatoria 2025 (4h)", "" },
    { 2027, 1, 10, 9, 0, 10, 30, "FECHA LIMITE (23:55h): Entrega PEC 2 - Fundamentos de Sistemas D", "" },
    { 2027, 1, 11, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 12 (Examenes 202", "" },
    { 2027, 1, 12, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 12 (Examene", "" },
    { 2027, 1, 13, 9, 0, 10, 30, "ENTREGA: 4 Practica Final en C al Tutor - Fundamentos de Progr", "" },
    { 2027, 1, 13, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 12 (Examenes 202", "" },
    { 2027, 1, 14, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 12 (Examenes", "" },
    { 2027, 1, 15, 9, 30, 13, 30, "Simulacro Oficial Convocatoria 2023 (4h)", "" },
    { 2027, 1, 18, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 13 (Sprint", "" },
    { 2027, 1, 19, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 13 (12 Patr", "" },
    { 2027, 1, 20, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Programacion  Sem 13 (Mantenimient", "" },
    { 2027, 1, 21, 9, 30, 13, 30, "Estudio (4h): Logica y Estructuras Discretas  Sem 13 (Cierre PE", "" },
    { 2027, 1, 22, 9, 30, 13, 30, "Estudio (4h): Fundamentos de Sistemas Digitales  Sem 13 (Simulac", "" },
    { 2027, 1, 24, 9, 0, 10, 30, "CIERRE DEFINITIVO: Tests PEC Online (3 puntos) - Logica y Estruc", "" },
    { 2027, 1, 25, 10, 30, 12, 30, "EXAMEN OFICIAL (1 Semana): Fundamentos de Sistemas Digitales (7", "Centro Asociado UNED Avila" },
    { 2027, 1, 26, 9, 30, 13, 30, "Sprint Final 1 Semana (4h): Logica y Estructuras Discretas (Dia", "" },
    { 2027, 1, 27, 9, 30, 13, 30, "Sprint Final 1 Semana (4h): Logica y Estructuras Discretas (Dia", "" },
    { 2027, 1, 28, 9, 30, 13, 30, "Sprint Final 1 Semana (4h): Logica y Estructuras Discretas (Dia", "" },
    { 2027, 1, 29, 15, 0, 17, 0, "EXAMEN OFICIAL (1 Semana): Logica y Estructuras Discretas (7190", "Centro Asociado UNED Avila" },
    { 2027, 2, 1, 9, 30, 13, 30, "Sprint Final 2 Semana (4h): Programacion (Tests Bloque A-B)", "" },
    { 2027, 2, 2, 9, 30, 13, 30, "Sprint Final 2 Semana (4h): Programacion (Tests Bloque C)", "" },
    { 2027, 2, 3, 9, 30, 13, 30, "Sprint Final 2 Semana (4h): Programacion (TADs en papel)", "" },
    { 2027, 2, 4, 9, 30, 13, 30, "Sprint Final 2 Semana (4h): Programacion (TADs en papel)", "" },
    { 2027, 2, 5, 9, 30, 13, 30, "Sprint Final 2 Semana (4h): Programacion (TADs en papel)", "" },
    { 2027, 2, 8, 9, 30, 13, 30, "Sprint Final 2 Semana (4h): Programacion (Simulacro Final)", "" },
    { 2027, 2, 9, 8, 0, 10, 0, "EXAMEN OFICIAL (2 Semana): Fundamentos de Programacion (7190102", "Centro Asociado UNED Avila" },
};

#endif // ACADEMIC_CALENDAR_DATA_H
