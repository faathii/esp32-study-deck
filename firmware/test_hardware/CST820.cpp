#include <Arduino.h>
#include "CST820.h"

CST820::CST820(int8_t sda_pin, int8_t scl_pin, int8_t rst_pin, int8_t int_pin)
{
    _sda = sda_pin;
    _scl = scl_pin;
    _rst = rst_pin;
    _int = int_pin;
}

bool CST820::begin(void)
{
    // Reset Pin Configuration (secuencia oficial de fábrica)
    if (_rst != -1)
    {
        pinMode(_rst, OUTPUT);
        digitalWrite(_rst, LOW);
        delay(10);
        digitalWrite(_rst, HIGH);
        delay(300); // El chip CST820 necesita 300ms para arrancar
    }

    // Int Pin Configuration
    if (_int != -1)
    {
        pinMode(_int, OUTPUT);
        digitalWrite(_int, HIGH);
        delay(1);
        digitalWrite(_int, LOW);
        delay(1);
        pinMode(_int, INPUT); // Devolver a INPUT para que el chip pueda emitir interrupciones
    }

    // Inicializar I2C
    if (_sda != -1 && _scl != -1)
    {
        Wire.begin(_sda, _scl, 400000);
    }
    else
    {
        Wire.begin();
    }

    // Comprobar si el chip responde en 0x15
    Wire.beginTransmission(I2C_ADDR_CST820);
    uint8_t err = Wire.endTransmission();
    if (err != 0) {
        Serial.printf("[CST820] No responde en 0x15 (error I2C: %d)\n", err);
        return false;
    }

    // Desactivar modo bajo consumo automático
    i2c_write(0xFE, 0xFF);
    Serial.println("[CST820] Controlador Capacitivo Inicializado Correctamente en 0x15");
    return true;
}

bool CST820::getTouch(uint16_t *x, uint16_t *y, uint8_t *gesture)
{
    uint8_t fingerIndex = i2c_read(0x02);
    if (!fingerIndex)
    {
        return false;
    }

    *gesture = i2c_read(0x01);
    if (!(*gesture == SlideUp || *gesture == SlideDown))
    {
        *gesture = None;
    }

    uint8_t data[4];
    if (i2c_read_continuous(0x03, data, 4) != 0)
    {
        return false;
    }
    
    *x = ((data[0] & 0x0F) << 8) | data[1];
    *y = ((data[2] & 0x0F) << 8) | data[3];

    return true;
}

uint8_t CST820::i2c_read(uint8_t addr)
{
    uint8_t rdData = 0;
    uint8_t rdDataCount = 0;
    uint8_t retries = 5;
    while (retries--)
    {
        Wire.beginTransmission(I2C_ADDR_CST820);
        Wire.write(addr);
        if (Wire.endTransmission(false) == 0)
        {
            rdDataCount = Wire.requestFrom((uint8_t)I2C_ADDR_CST820, (uint8_t)1);
            if (rdDataCount > 0)
            {
                rdData = Wire.read();
                break;
            }
        }
        delayMicroseconds(100);
    }
    return rdData;
}

uint8_t CST820::i2c_read_continuous(uint8_t addr, uint8_t *data, uint32_t length)
{
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    if (Wire.endTransmission(true) != 0) return 1;
    if (Wire.requestFrom((uint8_t)I2C_ADDR_CST820, (uint8_t)length) != length) return 1;
    for (uint32_t i = 0; i < length; i++) {
        *data++ = Wire.read();
    }
    return 0;
}

void CST820::i2c_write(uint8_t addr, uint8_t data)
{
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    Wire.write(data);
    Wire.endTransmission();
}

uint8_t CST820::i2c_write_continuous(uint8_t addr, const uint8_t *data, uint32_t length)
{
    Wire.beginTransmission(I2C_ADDR_CST820);
    Wire.write(addr);
    for (uint32_t i = 0; i < length; i++) {
        Wire.write(*data++);
    }
    if (Wire.endTransmission(true) != 0) return 1;
    return 0;
}
