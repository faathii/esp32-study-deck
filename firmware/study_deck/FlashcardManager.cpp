#include "FlashcardManager.h"

void FlashcardManager::begin() {
    _currentIndex = 0;
    _isFlipped = false;
    _easyCount = 0;
    _hardCount = 0;
    _totalCards = 0;

    _prefs.begin("study_cards", false);

    // Purgar fichas de ejemplo de versiones previas en NVS
    if (!_prefs.getBool("real_only_v2", false)) {
        _prefs.clear();
        _prefs.putBool("real_only_v2", true);
        _totalCards = 0;
    } else {
        loadCustomFromNVS();
    }

    // Si hay archivo de fichas en MicroSD, cargarlas
    if (SD.exists("/flashcards.txt")) {
        loadFromSD();
    }
}

void FlashcardManager::loadDefaultCards() {
    _totalCards = 0; // Cero fichas por defecto. Solo las que cree el usuario.
}

void FlashcardManager::loadFromSD() {
    if (!SD.exists("/flashcards.txt")) {
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
