#ifndef CST820_H
#define CST820_H

#include <Arduino.h>
#include <Wire.h>

#define I2C_ADDR_CST820 0x15

enum GestureType {
    GESTURE_NONE        = 0x00,
    GESTURE_SLIDE_DOWN  = 0x01,
    GESTURE_SLIDE_UP    = 0x02,
    GESTURE_SLIDE_LEFT  = 0x03,
    GESTURE_SLIDE_RIGHT = 0x04,
    GESTURE_SINGLE_TAP  = 0x05,
    GESTURE_DOUBLE_TAP  = 0x0B,
    GESTURE_LONG_PRESS  = 0x0C
};

class CST820 {
public:
    CST820(int8_t sda_pin = 33, int8_t scl_pin = 32, int8_t rst_pin = 25, int8_t int_pin = 21);

    bool begin(void);
    bool getRawTouch(uint16_t *x, uint16_t *y, uint8_t *gesture = nullptr);
    bool getTouch(int *screenX, int *screenY); // Adaptado a modo horizontal 320x240

private:
    int8_t _sda, _scl, _rst, _int;

    uint8_t i2c_read(uint8_t addr);
    uint8_t i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length);
    void i2c_write(uint8_t addr, uint8_t data);
    uint8_t i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length);
};

#endif // CST820_H

