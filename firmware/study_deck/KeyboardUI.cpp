#include "KeyboardUI.h"

// Colores del teclado
#define KB_COLOR_BG       0x0841 // Pizarra oscuro
#define KB_COLOR_KEY      0x1CE7 // Gris tecla
#define KB_COLOR_KEY_TXT  0xFFFF // Blanco
#define KB_COLOR_SPECIAL  0x2A6B // Azul grisáceo para teclas especiales
#define KB_COLOR_GREEN    0x05E6 // Verde conectar
#define KB_COLOR_RED      0xC8A2 // Rojo cancelar
#define KB_COLOR_BORDER   0x4208

KeyboardUI::KeyboardUI(TFT_eSPI &tft) : _tft(tft) {}

void KeyboardUI::reset(const String &title, bool isPassword, const String &initialText) {
    _promptTitle = title;
    _isPassword = isPassword;
    _inputText = initialText;
    _mode = KB_MODE_LOWERCASE;
    _hidePassword = isPassword;
}

void KeyboardUI::drawFull() {
    _tft.fillScreen(KB_COLOR_BG);
    drawHeader();
    updateInputBox();
    drawKeys();
}

void KeyboardUI::drawHeader() {
    _tft.fillRect(0, 0, 320, 28, KB_COLOR_BG);
    _tft.setTextColor(TFT_WHITE, KB_COLOR_BG);
    
    // Título descriptivo (recortar si es muy largo)
    String title = _promptTitle;
    if (title.length() > 26) {
        title = title.substring(0, 24) + "...";
    }
    _tft.drawString(title, 12, 6, 2);
}

void KeyboardUI::updateInputBox() {
    // Caja de texto (x: 10, y: 28, w: 255, h: 30)
    _tft.fillRoundRect(10, 28, 255, 30, 4, 0x10A2);
    _tft.drawRoundRect(10, 28, 255, 30, 4, 0x3BFF);

    // Texto de contraseña o texto plano
    String displayStr = "";
    if (_hidePassword) {
        for (size_t i = 0; i < _inputText.length(); i++) {
            displayStr += "*";
        }
    } else {
        displayStr = _inputText;
    }
    if (displayStr.length() > 22) {
        displayStr = displayStr.substring(displayStr.length() - 22);
    }
    displayStr += "_"; // Cursor

    _tft.setTextColor(TFT_WHITE, 0x10A2);
    _tft.drawString(displayStr, 18, 35, 2);

    // Botón Ojo (Ver / Ocultar) x: 270, y: 28, w: 40, h: 30
    _tft.fillRoundRect(270, 28, 40, 30, 4, KB_COLOR_SPECIAL);
    _tft.drawRoundRect(270, 28, 40, 30, 4, KB_COLOR_BORDER);
    _tft.setTextColor(TFT_WHITE, KB_COLOR_SPECIAL);
    _tft.drawCentreString(_hidePassword ? "***" : "ABC", 290, 35, 2);
}

void KeyboardUI::drawKeys() {
    // FILA 1: Números / Símbolos superiores (y: 64, h: 28)
    const char *row1_letters = "1234567890";
    const char *row1_symbols = "!@#$%^&*()";
    const char *r1 = (_mode == KB_MODE_SYMBOLS) ? row1_symbols : row1_letters;

    for (int i = 0; i < 10; i++) {
        int kx = 10 + i * 30;
        _tft.fillRoundRect(kx, 64, 28, 28, 4, KB_COLOR_KEY);
        _tft.drawRoundRect(kx, 64, 28, 28, 4, KB_COLOR_BORDER);
        _tft.setTextColor(KB_COLOR_KEY_TXT, KB_COLOR_KEY);
        char c[2] = { r1[i], 0 };
        _tft.drawCentreString(c, kx + 14, 70, 2);
    }

    // FILA 2: Q - P / Símbolos (y: 96, h: 28)
    const char *row2_lower = "qwertyuiop";
    const char *row2_upper = "QWERTYUIOP";
    const char *row2_sym   = "-_=+[]{}/\\";
    const char *r2 = (_mode == KB_MODE_SYMBOLS) ? row2_sym : ((_mode == KB_MODE_UPPERCASE) ? row2_upper : row2_lower);

    for (int i = 0; i < 10; i++) {
        int kx = 10 + i * 30;
        _tft.fillRoundRect(kx, 96, 28, 28, 4, KB_COLOR_KEY);
        _tft.drawRoundRect(kx, 96, 28, 28, 4, KB_COLOR_BORDER);
        _tft.setTextColor(KB_COLOR_KEY_TXT, KB_COLOR_KEY);
        char c[2] = { r2[i], 0 };
        _tft.drawCentreString(c, kx + 14, 102, 2);
    }

    // FILA 3: A - L + Borrar (y: 128, h: 28)
    const char *row3_lower = "asdfghjkl";
    const char *row3_upper = "ASDFGHJKL";
    const char *row3_sym   = ":;\"'<>,.?";
    const char *r3 = (_mode == KB_MODE_SYMBOLS) ? row3_sym : ((_mode == KB_MODE_UPPERCASE) ? row3_upper : row3_lower);

    for (int i = 0; i < 9; i++) {
        int kx = 10 + i * 29;
        _tft.fillRoundRect(kx, 128, 27, 28, 4, KB_COLOR_KEY);
        _tft.drawRoundRect(kx, 128, 27, 28, 4, KB_COLOR_BORDER);
        _tft.setTextColor(KB_COLOR_KEY_TXT, KB_COLOR_KEY);
        char c[2] = { r3[i], 0 };
        _tft.drawCentreString(c, kx + 13, 134, 2);
    }

    // Tecla Borrar [⌫] en columna 9 (x: 271, w: 39, h: 28)
    _tft.fillRoundRect(271, 128, 39, 28, 4, KB_COLOR_SPECIAL);
    _tft.drawRoundRect(271, 128, 39, 28, 4, KB_COLOR_BORDER);
    _tft.setTextColor(TFT_WHITE, KB_COLOR_SPECIAL);
    _tft.drawCentreString("DEL", 290, 134, 2);

    // FILA 4: Shift + Z - M + 123/SYM (y: 160, h: 28)
    // Tecla Shift / Mayúsculas (x: 10, w: 42, h: 28)
    uint16_t shiftBg = (_mode == KB_MODE_UPPERCASE) ? 0x05E6 : KB_COLOR_SPECIAL;
    _tft.fillRoundRect(10, 160, 42, 28, 4, shiftBg);
    _tft.drawRoundRect(10, 160, 42, 28, 4, KB_COLOR_BORDER);
    _tft.setTextColor(TFT_WHITE, shiftBg);
    _tft.drawCentreString("MAY", 31, 166, 2);

    // Letras Z - M (7 teclas)
    const char *row4_lower = "zxcvbnm";
    const char *row4_upper = "ZXCVBNM";
    const char *row4_sym   = "~`|#%&*";
    const char *r4 = (_mode == KB_MODE_SYMBOLS) ? row4_sym : ((_mode == KB_MODE_UPPERCASE) ? row4_upper : row4_lower);

    for (int i = 0; i < 7; i++) {
        int kx = 56 + i * 28;
        _tft.fillRoundRect(kx, 160, 26, 28, 4, KB_COLOR_KEY);
        _tft.drawRoundRect(kx, 160, 26, 28, 4, KB_COLOR_BORDER);
        _tft.setTextColor(KB_COLOR_KEY_TXT, KB_COLOR_KEY);
        char c[2] = { r4[i], 0 };
        _tft.drawCentreString(c, kx + 13, 166, 2);
    }

    // Tecla [?123 / SYM] (x: 254, w: 56, h: 28)
    uint16_t symBg = (_mode == KB_MODE_SYMBOLS) ? 0x05E6 : KB_COLOR_SPECIAL;
    _tft.fillRoundRect(254, 160, 56, 28, 4, symBg);
    _tft.drawRoundRect(254, 160, 56, 28, 4, KB_COLOR_BORDER);
    _tft.setTextColor(TFT_WHITE, symBg);
    _tft.drawCentreString((_mode == KB_MODE_SYMBOLS) ? "ABC" : "123", 282, 166, 2);

    // FILA 5: Barra de control inferior (y: 196, h: 36)
    // 1. [ ATRÁS ] (x: 10, w: 70, h: 36)
    _tft.fillRoundRect(10, 196, 70, 36, 6, KB_COLOR_RED);
    _tft.drawRoundRect(10, 196, 70, 36, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, KB_COLOR_RED);
    _tft.drawCentreString("ATRAS", 45, 206, 2);

    // 2. [ ESPACIO ] (x: 88, w: 114, h: 36)
    _tft.fillRoundRect(88, 196, 114, 36, 6, KB_COLOR_KEY);
    _tft.drawRoundRect(88, 196, 114, 36, 6, KB_COLOR_BORDER);
    _tft.setTextColor(KB_COLOR_KEY_TXT, KB_COLOR_KEY);
    _tft.drawCentreString("ESPACIO", 145, 206, 2);

    // 3. [ CONECTAR ] (x: 210, w: 100, h: 36)
    _tft.fillRoundRect(210, 196, 100, 36, 6, KB_COLOR_GREEN);
    _tft.drawRoundRect(210, 196, 100, 36, 6, TFT_WHITE);
    _tft.setTextColor(TFT_WHITE, KB_COLOR_GREEN);
    _tft.drawCentreString("CONECTAR", 260, 206, 2);
}

KeyAction KeyboardUI::handleTouch(int x, int y) {
    // 1. Botón Ojo (Ver / Ocultar) en x: 265..315, y: 24..60
    if (x >= 265 && x <= 315 && y >= 24 && y <= 60) {
        _hidePassword = !_hidePassword;
        updateInputBox();
        return KEY_ACTION_TOGGLE_HIDE;
    }

    // 2. FILA 1: (y: 60..94)
    if (y >= 60 && y <= 94) {
        const char *row1_letters = "1234567890";
        const char *row1_symbols = "!@#$%^&*()";
        const char *r1 = (_mode == KB_MODE_SYMBOLS) ? row1_symbols : row1_letters;

        for (int i = 0; i < 10; i++) {
            int kx = 10 + i * 30;
            if (x >= kx - 2 && x <= kx + 30) {
                _inputText += r1[i];
                updateInputBox();
                return KEY_ACTION_CHAR_TYPED;
            }
        }
    }

    // 3. FILA 2: (y: 95..126)
    if (y >= 95 && y <= 126) {
        const char *row2_lower = "qwertyuiop";
        const char *row2_upper = "QWERTYUIOP";
        const char *row2_sym   = "-_=+[]{}/\\";
        const char *r2 = (_mode == KB_MODE_SYMBOLS) ? row2_sym : ((_mode == KB_MODE_UPPERCASE) ? row2_upper : row2_lower);

        for (int i = 0; i < 10; i++) {
            int kx = 10 + i * 30;
            if (x >= kx - 2 && x <= kx + 30) {
                _inputText += r2[i];
                updateInputBox();
                return KEY_ACTION_CHAR_TYPED;
            }
        }
    }

    // 4. FILA 3: (y: 127..158)
    if (y >= 127 && y <= 158) {
        // Tecla Borrar [DEL] en x: 265..315
        if (x >= 265 && x <= 315) {
            if (_inputText.length() > 0) {
                _inputText.remove(_inputText.length() - 1);
                updateInputBox();
                return KEY_ACTION_DELETE;
            }
            return KEY_ACTION_NONE;
        }

        // Letras Fila 3
        const char *row3_lower = "asdfghjkl";
        const char *row3_upper = "ASDFGHJKL";
        const char *row3_sym   = ":;\"'<>,.?";
        const char *r3 = (_mode == KB_MODE_SYMBOLS) ? row3_sym : ((_mode == KB_MODE_UPPERCASE) ? row3_upper : row3_lower);

        for (int i = 0; i < 9; i++) {
            int kx = 10 + i * 29;
            if (x >= kx - 2 && x <= kx + 29) {
                _inputText += r3[i];
                updateInputBox();
                return KEY_ACTION_CHAR_TYPED;
            }
        }
    }

    // 5. FILA 4: (y: 159..192)
    if (y >= 159 && y <= 192) {
        // Tecla Shift / Mayúsculas (x: 10..52)
        if (x >= 8 && x <= 53) {
            if (_mode == KB_MODE_LOWERCASE) {
                _mode = KB_MODE_UPPERCASE;
            } else if (_mode == KB_MODE_UPPERCASE) {
                _mode = KB_MODE_LOWERCASE;
            } else {
                _mode = KB_MODE_LOWERCASE;
            }
            drawKeys();
            return KEY_ACTION_NONE;
        }

        // Tecla [?123 / SYM] (x: 250..315)
        if (x >= 250 && x <= 315) {
            if (_mode == KB_MODE_SYMBOLS) {
                _mode = KB_MODE_LOWERCASE;
            } else {
                _mode = KB_MODE_SYMBOLS;
            }
            drawKeys();
            return KEY_ACTION_NONE;
        }

        // Letras Z - M (x: 54..249)
        const char *row4_lower = "zxcvbnm";
        const char *row4_upper = "ZXCVBNM";
        const char *row4_sym   = "~`|#%&*";
        const char *r4 = (_mode == KB_MODE_SYMBOLS) ? row4_sym : ((_mode == KB_MODE_UPPERCASE) ? row4_upper : row4_lower);

        for (int i = 0; i < 7; i++) {
            int kx = 56 + i * 28;
            if (x >= kx - 2 && x <= kx + 28) {
                _inputText += r4[i];
                updateInputBox();
                return KEY_ACTION_CHAR_TYPED;
            }
        }
    }

    // 6. FILA 5: Controles inferiores (y: 193..238)
    if (y >= 193 && y <= 238) {
        // [ ATRÁS ] (x: 8..82)
        if (x >= 8 && x <= 82) {
            return KEY_ACTION_BACK;
        }

        // [ ESPACIO ] (x: 85..205)
        if (x >= 85 && x <= 205) {
            _inputText += " ";
            updateInputBox();
            return KEY_ACTION_CHAR_TYPED;
        }

        // [ CONECTAR ] (x: 208..315)
        if (x >= 208 && x <= 315) {
            return KEY_ACTION_SUBMIT;
        }
    }

    return KEY_ACTION_NONE;
}

