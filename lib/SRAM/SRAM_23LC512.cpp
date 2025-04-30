#include <Arduino.h>
#include <SPI.h>

#include "SRAM_23LC512.h"

// SRAM register commands
#define SRAM_READ_MODE_REGISTER 0x05
#define SRAM_WRITE_MODE_REGISTER 0x01

// SRAM commands
#define SRAM_READ_COMMAND 0x03
#define SRAM_WRITE_COMMAND 0x02

// SRAM data access modes
#define SRAM_BYTE_MODE 0x00
#define SRAM_SEQUENTIAL_MODE 0x40
#define SRAM_PAGE_MODE 0x80

const uint8_t dummy_byte = 0x00;

// F_CPU works on UNO
SPISettings settings = SPISettings(F_CPU / 8, MSBFIRST, SPI_MODE0);

void SRAM_23LC512::init() {
  SPI.beginTransaction(settings);

  // set write mode to sequential
  digitalWrite(_cs, LOW);                 // select SPI Ram
  SPI.transfer(SRAM_WRITE_MODE_REGISTER); // write to Mode Register
  SPI.transfer(SRAM_SEQUENTIAL_MODE);     // set sequential mode
  digitalWrite(_cs, HIGH);                // deselect SPI ram

  SPI.endTransaction();
}

void SRAM_23LC512::writeSPIByte(unsigned long addr, uint8_t data) {
  SPI.beginTransaction(settings);
  digitalWrite(_cs, LOW);           // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  SPI.transfer(data);

  digitalWrite(_cs, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

uint8_t SRAM_23LC512::readSPIByte(unsigned long addr) {
  SPI.beginTransaction(settings);
  digitalWrite(_cs, LOW);          // select SPI Ram
  SPI.transfer(SRAM_READ_COMMAND); // read command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  uint8_t result = SPI.transfer(dummy_byte);

  digitalWrite(_cs, HIGH); // deselect SPI ram
  SPI.endTransaction();
  return result;
}

void SRAM_23LC512::writeSPIWord(unsigned long addr, uint16_t data) {
  SPI.beginTransaction(settings);
  digitalWrite(_cs, LOW);           // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  SPI.transfer((uint8_t)((data & 0xff)));
  SPI.transfer((uint8_t)((data >> 8) & 0xff));

  digitalWrite(_cs, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

uint16_t SRAM_23LC512::readSPIWord(unsigned long addr) {
  SPI.beginTransaction(settings);
  digitalWrite(_cs, LOW);          // select SPI Ram
  SPI.transfer(SRAM_READ_COMMAND); // read command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  uint8_t low = SPI.transfer(dummy_byte);
  uint8_t high = SPI.transfer(dummy_byte);

  digitalWrite(_cs, HIGH); // deselect SPI ram
  SPI.endTransaction();
  return (high << 8) | low;
}

void SRAM_23LC512::fillMemory(uint8_t data) {
  fillMemory(0x0000, 0xffff, data);
}

void SRAM_23LC512::fillMemory(unsigned long addr, unsigned long count,
                              uint8_t data) {
  SPI.beginTransaction(settings);
  digitalWrite(_cs, LOW);           // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  for (unsigned long i = 0; i < count; i++) {
    SPI.transfer(data);
  }

  digitalWrite(_cs, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

bool SRAM_23LC512::verifyMemory(uint8_t data) {
  return verifyMemory(0x0000, 0xffff, data);
}

bool SRAM_23LC512::verifyMemory(unsigned long addr, unsigned long count,
                                uint8_t data) {
  char buf[50];

  SPI.beginTransaction(settings);
  digitalWrite(_cs, LOW);          // select SPI Ram
  SPI.transfer(SRAM_READ_COMMAND); // read command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  bool ok = true;

  for (unsigned long i = 0; i < count; i++) {
    uint8_t result = SPI.transfer(dummy_byte);
    if (result != data) {
      sprintf(buf, "(%02d) 0x%04X : read 0x%02X expect 0x%02X", _cs,
              (uint16_t)addr, result, data);

      Serial.println(buf);
      ok = false;
      break;
    }
  }

  digitalWrite(_cs, HIGH); // deselect SPI ram
  SPI.endTransaction();
  return ok;
}

void SRAM_23LC512::fillMemorySlow(unsigned long addr, unsigned long count,
                                  uint8_t data) {
  for (unsigned long i = 0; i < count; i++) {
    writeSPIByte(addr + i, data);
  }
}

bool SRAM_23LC512::verifyMemorySlow(unsigned long addr, unsigned long count,
                                    uint8_t data) {
  char buf[50];

  bool ok = true;

  for (unsigned long i = 0; i < count; i++) {
    uint8_t result = readSPIByte(addr + i);

    if (result != data) {
      sprintf(buf, "(%02d) 0x%04X : read 0x%02X expect 0x%02X", _cs,
              (uint16_t)addr, result, data);

      Serial.println(buf);
      ok = false;
      break;
    }
  }

  return ok;
}

void SRAM_23LC512::writeBlock(uint16_t addr, uint8_t *data, uint16_t length) {
  SPI.beginTransaction(settings);
  digitalWrite(_cs, LOW);           // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  for (unsigned long i = 0; i < length; i++) {
    SPI.transfer(data[i]);
  }

  digitalWrite(_cs, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

void SRAM_23LC512::writeBlock(uint16_t addr, uint16_t *data, uint16_t length) {
  SPI.beginTransaction(settings);
  digitalWrite(_cs, LOW);           // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  for (unsigned long i = 0; i < length; i++) {
    SPI.transfer16(data[i]);
  }

  digitalWrite(_cs, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

bool SRAM_23LC512::verifyBlock(uint16_t addr, uint8_t *data, uint16_t length) {
  char buf[50];

  SPI.beginTransaction(settings);
  digitalWrite(_cs, LOW);          // select SPI Ram
  SPI.transfer(SRAM_READ_COMMAND); // read command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  bool ok = true;

  for (unsigned long i = 0; i < length; i++) {
    uint8_t result = SPI.transfer(dummy_byte);
    if (result != data[i]) {
      sprintf(buf, "(%02d) 0x%04X : read 0x%02X expect 0x%02X", _cs,
              (uint16_t)addr, result, data[i]);

      Serial.println(buf);
      ok = false;
      break;
    }
  }

  digitalWrite(_cs, HIGH); // deselect SPI ram
  SPI.endTransaction();
  return ok;
}
