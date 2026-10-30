#include "FlashcardManager.h"

void FlashcardManager::begin() {
    _currentIndex = 0;
    _isFlipped = false;
    _easyCount = 0;
    _hardCount = 0;
    _totalCards = 0;

    _prefs.begin("study_cards", false);

    // Intentar cargar primero desde MicroSD
    loadFromSD();

    // Si no hay SD o está vacía, cargar de NVS o por defecto
    if (_totalCards == 0) {
        loadCustomFromNVS();
    }

    if (_totalCards == 0) {
        loadDefaultCards();
    }
}

void FlashcardManager::loadDefaultCards() {
    _totalCards = 6;
    _cards[0] = { "Ley de Ohm", "V = I * R\n(Voltaje = Intensidad x Resistencia)", "Fisica" };
    _cards[1] = { "Energia Cinetica", "Ec = 1/2 * m * v^2\n(Medida en Julios [J])", "Fisica" };
    _cards[2] = { "Tecnica Feynman", "1. Elige concepto\n2. Explicalo facil\n3. Detecta huecos\n4. Simplifica", "Metodo" };
    _cards[3] = { "Regla 80/20 (Pareto)", "El 80% de los resultados\nproviene del 20% del esfuerzo clave.", "Productividad" };
    _cards[4] = { "Curva del Olvido", "Ebbinghaus: Repasar a las\n24h, 3 dias y 7 dias fija la memoria.", "Memoria" };
    _cards[5] = { "Ciclo de Krebs", "Ruta en la matriz mitocondrial\npara generar ATP y transportadores.", "Bioquimica" };
}

void FlashcardManager::loadFromSD() {
    if (!SD.exists("/flashcards.txt")) {
        // Crear plantilla en MicroSD para facilitar al usuario agregar tarjetas en PC
        File f = SD.open("/flashcards.txt", FILE_WRITE);
        if (f) {
            f.println("Ley de Ohm|V = I * R (Voltaje = Intensidad x R)|Fisica");
            f.println("Energia Cinetica|Ec = 1/2 * m * v^2 [Julios]|Fisica");
            f.println("Tecnica Feynman|1.Concepto 2.Explica 3.Corrige|Metodo");
            f.close();
            Serial.println("[Flashcards] Plantilla /flashcards.txt creada en MicroSD.");
        }
        return;
    }

    File f = SD.open("/flashcards.txt", FILE_READ);
    if (!f) return;

    int loaded = 0;
    while (f.available() && loaded < MAX_CARDS) {
        String line = f.readStringUntil('\n');
        line.trim();
        if (line.length() < 3 || line.startsWith("#")) continue;

        int p1 = line.indexOf('|');
        if (p1 == -1) continue;
        int p2 = line.indexOf('|', p1 + 1);

        String q = line.substring(0, p1);
        String a = (p2 != -1) ? line.substring(p1 + 1, p2) : line.substring(p1 + 1);
        String c = (p2 != -1) ? line.substring(p2 + 1) : "General";

        _cards[loaded++] = { q, a, c };
    }
    f.close();

    if (loaded > 0) {
        _totalCards = loaded;
        Serial.printf("[Flashcards] %d tarjetas cargadas desde MicroSD.\n", loaded);
    }
}

void FlashcardManager::loadCustomFromNVS() {
    int count = _prefs.getInt("card_count", 0);
    if (count <= 0) return;

    if (count > MAX_CARDS) count = MAX_CARDS;
    _totalCards = count;

    for (int i = 0; i < count; i++) {
        char kQ[16], kA[16], kC[16];
        snprintf(kQ, sizeof(kQ), "q_%d", i);
        snprintf(kA, sizeof(kA), "a_%d", i);
        snprintf(kC, sizeof(kC), "c_%d", i);

        _cards[i].question = _prefs.getString(kQ, "Pregunta");
        _cards[i].answer   = _prefs.getString(kA, "Respuesta");
        _cards[i].category = _prefs.getString(kC, "General");
    }
}

void FlashcardManager::saveCustomToNVS() {
    _prefs.putInt("card_count", _totalCards);
    for (int i = 0; i < _totalCards; i++) {
        char kQ[16], kA[16], kC[16];
        snprintf(kQ, sizeof(kQ), "q_%d", i);
        snprintf(kA, sizeof(kA), "a_%d", i);
        snprintf(kC, sizeof(kC), "c_%d", i);

        _prefs.putString(kQ, _cards[i].question);
        _prefs.putString(kA, _cards[i].answer);
        _prefs.putString(kC, _cards[i].category);
    }
}

Flashcard FlashcardManager::getCurrentCard() const {
    if (_totalCards == 0) {
        return { "Sin tarjetas", "Pulsa [+ NUEVA] para crear una.", "Aviso" };
    }
    if (_currentIndex >= 0 && _currentIndex < _totalCards) {
        return _cards[_currentIndex];
    }
    return { "Fin del mazo", "Has repasado todas las tarjetas.", "Info" };
}

void FlashcardManager::flip() {
    _isFlipped = !_isFlipped;
}

void FlashcardManager::nextCard(bool wasEasy) {
    if (wasEasy) _easyCount++;
    else _hardCount++;

    if (_totalCards > 0) {
        _currentIndex = (_currentIndex + 1) % _totalCards;
    }
    _isFlipped = false;
}

void FlashcardManager::previousCard() {
    if (_totalCards > 0) {
        _currentIndex = (_currentIndex > 0) ? (_currentIndex - 1) : (_totalCards - 1);
    }
    _isFlipped = false;
}

void FlashcardManager::reset() {
    _currentIndex = 0;
    _isFlipped = false;
    _easyCount = 0;
    _hardCount = 0;
}

bool FlashcardManager::addCard(const String &question, const String &answer, const String &category) {
    if (question.length() == 0 || answer.length() == 0) return false;
    if (_totalCards < MAX_CARDS) {
        _cards[_totalCards] = { question, answer, category };
        _totalCards++;
        saveCustomToNVS();
        return true;
    }
    return false;
}
