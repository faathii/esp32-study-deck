#include "FlashcardManager.h"

void FlashcardManager::begin() {
    _currentIndex = 0;
    _isFlipped = false;
    _easyCount = 0;
    _hardCount = 0;
    _totalCards = 0;

    _prefs.begin("study_cards", false);

    // Purgar fichas antiguas de versiones previas en NVS
    if (!_prefs.getBool("real_uned_v3", false)) {
        _prefs.clear();
        _prefs.putBool("real_uned_v3", true);
        loadDefaultCards();
    } else {
        loadCustomFromNVS();
        if (_totalCards == 0) {
            loadDefaultCards();
        }
    }

    // Si hay archivo de fichas en MicroSD, complementar con SD
    if (SD.exists("/flashcards.txt")) {
        loadFromSD();
    }
}

void FlashcardManager::loadDefaultCards() {
    _cards[0] = { "Operador para acceder a miembros via puntero?", "El operador flecha -> (ej. p->campo)", "Prog C" };
    _cards[1] = { "Diferencia entre *p y &var en C?", "*p obtiene el valor; &var obtiene la direccion", "Prog C" };
    _cards[2] = { "Que es un TAD en lenguaje C?", "Tipo Abstracto: interfaz (.h) e implementacion privada (.c)", "Prog C" };
    
    _cards[3] = { "Para que sirve un mapa de Karnaugh?", "Simplificar algebra booleana agrupando 1s adyacentes", "Sist. Digitales" };
    _cards[4] = { "Funcion de un Multiplexor 4 a 1?", "Selecciona 1 de 4 entradas segun 2 lineas de control", "Sist. Digitales" };
    _cards[5] = { "Combinacional vs Secuencial?", "Combinacional: depende de entradas; Secuencial: tiene memoria (flip-flops)", "Sist. Digitales" };
    
    _cards[6] = { "Leyes de De Morgan en logica?", "!(P y Q) <=> (!P o !Q)  y  !(P o Q) <=> (!P y !Q)", "Logica Discreta" };
    _cards[7] = { "Regla de inferencia Modus Ponens?", "De 'P -> Q' y 'P' se deduce necesariamente 'Q'", "Logica Discreta" };
    _cards[8] = { "Negacion de universal !(Para todo x, P(x))?", "Existe x tal que !P(x)", "Logica Discreta" };
    
    _totalCards = 9;
    saveCustomToNVS();
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
