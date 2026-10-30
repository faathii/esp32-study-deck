#ifndef FLASHCARD_MANAGER_H
#define FLASHCARD_MANAGER_H

#include <Arduino.h>

struct Flashcard {
    String question;
    String answer;
    String category;
};

class FlashcardManager {
public:
    void begin();

    int getTotalCards() const { return _totalCards; }
    int getCurrentIndex() const { return _currentIndex; }
    bool isFlipped() const { return _isFlipped; }

    Flashcard getCurrentCard() const;
    void flip();
    void nextCard(bool wasEasy);
    void previousCard();
    void reset();

    int getEasyCount() const { return _easyCount; }
    int getHardCount() const { return _hardCount; }

private:
    int _totalCards = 0;
    int _currentIndex = 0;
    bool _isFlipped = false;
    int _easyCount = 0;
    int _hardCount = 0;

    static const int MAX_CARDS = 8;
    Flashcard _cards[MAX_CARDS];
};

#endif // FLASHCARD_MANAGER_H

