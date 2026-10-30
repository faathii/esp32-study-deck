#ifndef FLASHCARD_MANAGER_H
#define FLASHCARD_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>
#include <Preferences.h>

struct Flashcard {
    String question;
    String answer;
    String category;
};

class FlashcardManager {
public:
    void begin();
    void loadFromSD();

    int getTotalCards() const { return _totalCards; }
    int getCurrentIndex() const { return _currentIndex; }
    bool isFlipped() const { return _isFlipped; }

    Flashcard getCurrentCard() const;
    void flip();
    void nextCard(bool wasEasy);
    void previousCard();
    void reset();
    bool addCard(const String &question, const String &answer, const String &category = "Estudio");

    int getEasyCount() const { return _easyCount; }
    int getHardCount() const { return _hardCount; }

private:
    int _totalCards = 0;
    int _currentIndex = 0;
    bool _isFlipped = false;
    int _easyCount = 0;
    int _hardCount = 0;

    static const int MAX_CARDS = 16;
    Flashcard _cards[MAX_CARDS];
    Preferences _prefs;

    void loadDefaultCards();
    void loadCustomFromNVS();
    void saveCustomToNVS();
};

#endif // FLASHCARD_MANAGER_H
