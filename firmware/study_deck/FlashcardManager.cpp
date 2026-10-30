#include "FlashcardManager.h"

void FlashcardManager::begin() {
    _totalCards = 6;
    _currentIndex = 0;
    _isFlipped = false;
    _easyCount = 0;
    _hardCount = 0;

    _cards[0] = {
        "Ley de Ohm",
        "V = I * R\n(Voltaje = Intensidad x Resistencia)",
        "Fisica"
    };
    _cards[1] = {
        "Energia Cinetica",
        "Ec = 1/2 * m * v^2\n(Medida en Julios [J])",
        "Fisica"
    };
    _cards[2] = {
        "Tecnica Feynman",
        "1. Elige concepto\n2. Explicalo a un nino\n3. Identifica lagunas\n4. Simplifica",
        "Metodo"
    };
    _cards[3] = {
        "Regla 80/20 (Pareto)",
        "El 80% de los resultados\nproviene del 20% del esfuerzo clave.",
        "Productividad"
    };
    _cards[4] = {
        "Curva del Olvido",
        "Ebbinghaus: Repasar a las\n24h, 3 dias y 7 dias fija la memoria.",
        "Memoria"
    };
    _cards[5] = {
        "Ciclo de Krebs",
        "Ruta metabolica en la matriz\nmitocondrial para generar ATP y NADH.",
        "Biologia"
    };
}

Flashcard FlashcardManager::getCurrentCard() const {
    if (_currentIndex >= 0 && _currentIndex < _totalCards) {
        return _cards[_currentIndex];
    }
    return { "Fin de tarjetas", "Has repasado todas las fichas.", "Info" };
}

void FlashcardManager::flip() {
    _isFlipped = !_isFlipped;
}

void FlashcardManager::nextCard(bool wasEasy) {
    if (wasEasy) _easyCount++;
    else _hardCount++;

    _currentIndex = (_currentIndex + 1) % _totalCards;
    _isFlipped = false;
}

void FlashcardManager::previousCard() {
    if (_currentIndex > 0) {
        _currentIndex--;
    } else {
        _currentIndex = _totalCards - 1;
    }
    _isFlipped = false;
}

void FlashcardManager::reset() {
    _currentIndex = 0;
    _isFlipped = false;
    _easyCount = 0;
    _hardCount = 0;
}

