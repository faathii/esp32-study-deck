#ifndef KEYBOARD_UI_H
#define KEYBOARD_UI_H

#include <Arduino.h>
#include <TFT_eSPI.h>

enum KeyboardMode {
    KB_MODE_LOWERCASE,
    KB_MODE_UPPERCASE,
    KB_MODE_SYMBOLS
};

enum KeyAction {
    KEY_ACTION_NONE,
    KEY_ACTION_CHAR_TYPED,
    KEY_ACTION_DELETE,
    KEY_ACTION_BACK,
    KEY_ACTION_SUBMIT,
    KEY_ACTION_TOGGLE_HIDE
};

class KeyboardUI {
public:
    KeyboardUI(TFT_eSPI &tft);

    void reset(const String &ssid);
    void drawFull();
    void updateInputBox();
    KeyAction handleTouch(int x, int y);

    String getText() const { return _inputText; }
    void setText(const String &txt) { _inputText = txt; }
    String getSSID() const { return _targetSSID; }

private:
    TFT_eSPI &_tft;
    String _targetSSID;
    String _inputText = "";
    KeyboardMode _mode = KB_MODE_LOWERCASE;
    bool _hidePassword = false;

    void drawHeader();
    void drawKeys();
};

#endif // KEYBOARD_UI_H

