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

    // Entrada genérica para tareas, contraseñas, o textos
    void reset(const String &title, bool isPassword = false, const String &initialText = "");
    void drawFull();
    void updateInputBox();
    KeyAction handleTouch(int x, int y);

    String getText() const { return _inputText; }
    void setText(const String &txt) { _inputText = txt; }
    String getTitle() const { return _promptTitle; }

private:
    TFT_eSPI &_tft;
    String _promptTitle = "";
    String _inputText = "";
    KeyboardMode _mode = KB_MODE_LOWERCASE;
    bool _isPassword = false;
    bool _hidePassword = false;

    void drawHeader();
    void drawKeys();
};

#endif // KEYBOARD_UI_H
