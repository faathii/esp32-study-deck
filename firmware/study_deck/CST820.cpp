#include "CST820.h"

CST820::CST820(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin) {
    _sda = sda_pin;
    _scl = scl_pin;
    _rst = rst_pin;
    _int = int_pin;
}

bool CST820::begin(void) {
    // Secuencia de reinicio por hardware oficial para CST820
    if (_rst != -1) {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(300); // Pausa necesaria para el firmware interno del chip
    }

    // Configuración del pin de interrupción
    if (_int != -1) {
        pinMode(_int, OUTPUT);
        digitalWrite(_int, HIGH);
        delay(1);
        digitalWrite(_int, LOW);
        delay(1);
        pinMode(_int, INPUT); // Devolver a entrada para permitir interrupciones
    }

    // Inicializar bus I2C a 400 kHz
    if (_sda != -1 && _scl != -1) {
        Wire.begin(_sda, _scl, 400000);
    } else {
        Wire.begin();
    }

    // Comprobar presencia del chip en 0x15
    Wire.beginTransmission(I2C_ADDR_CST820);
    uint8_t err = Wire.endTransmission();
    if (err != 0) {
        Serial.printf("[CST820] Error: chip no responde en 0x15 (I2C error: %d)\n", err);
        return false;
    }

    // Desactivar modo de bajo consumo automático
    i2c_write(0xFE, 0xFF);
    Serial.println("[CST820] Táctil capacitivo iniciado correctamente en 0x15.");
    return true;
}

bool CST820::getRawTouch(uint16_t *x, uint16_t *y, uint8_t *gesture) {
    uint8_t fingerCount = i2c_read(0x02);
    if (!fingerCount) {
        return false;
    }

    uint8_t gest = i2c_read(0x01);
    if (gesture) {
        if (gest == GESTURE_SLIDE_UP || gest == GESTURE_SLIDE_DOWN ||
            gest == GESTURE_SLIDE_LEFT || gest == GESTURE_SLIDE_RIGHT) {
            *gesture = gest;
        } else {
            *gesture = GESTURE_NONE;
        }
    }

    uint8_t data[4];
    if (i2c_read_continuous(0x03, data, 4) != 0) {
        return false;
    }

    *x = ((data[0] & 0x0F) << 8) | data[1];
    *y = ((data[2] & 0x0F) << 8) | data[3];

    return true;
}

bool CST820::getTouch(int *screenX, int *screenY) {
    uint16_t rawX = 0, rawY = 0;
    if (!getRawTouch(&rawX, &rawY)) {
        return false;
    }

    // Transformación para orientación horizontal (Landscape, Rotation 1: 320x240)
    // El CST820 reporta nativamente en vertical (rawX: 0..240, rawY: 0..320)
    int sx = rawY;
    int sy = 240 - rawX;

    // Clamping de seguridad
    if (sx < 0) sx = 0;
    if (sx >= 320) sx = 319;
    if (sy < 0) sy = 0;
    if (sy >= 240) sy = 239;

    *screenX = sx;
    *screenY = sy;
    return true;
}

uint8_t CST820::i2c_read(uint8_t addr) {
    uint8_t rdData = 0;
    uint8_t retries = 5;
    while (retries--) {
        Wire.beginTransmission(I2C_ADDR_CST820);
        Wire.write(addr);
        if (Wire.endTransmission(false) == 0) {
            if (Wire.requestFrom((uint8_t)I2C_ADDR_CST820, (uint8_t)1) > 0) {
                rdData = Wire.read();
                break;
            }
        }
        delayMicroseconds(100);
    }
    return rdData;
}

uint8_t CST820::i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length) {
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    if (Wire.endTransmission(true) != 0) return 1;
    if (Wire.requestFrom((uint8_t)I2C_ADDR_CST820, (uint8_t)length) != length) return 1;
    for (uint32_t i = 0; i < length; i++) {
        *data++ = Wire.read();
    }
    return 0;
}

void CST820::i2c_write(uint8_t addr, uint8_t data) {
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    Wire.write(data);
    Wire.endTransmission();
}

uint8_t CST820::i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length) {
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    for (uint32_t i = 0; i < length; i++) {
        Wire.write(*data++);
    }
    if (Wire.endTransmission(true) != 0) return 1;
    return 0;
}

