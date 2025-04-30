#include <Arduino.h>
#include <SPI.h>

#include "SRAM_23LC512.h"

const uint8_t dummy_byte = 0x00;

// F_CPU works on UNU
SPISettings settings = SPISettings(F_CPU / 8, MSBFIRST, SPI_MODE0);

bool verifyMemorySlow(uint8_t csPin, unsigned long addr, unsigned long count,
                      uint8_t data);
void fillMemorySlow(uint8_t csPin, unsigned long addr, unsigned long count,
                    uint8_t data);

void initRAM(uint8_t csPin) {
  SPI.beginTransaction(settings);

  // set write mode to sequential
  digitalWrite(csPin, LOW);               // select SPI Ram
  SPI.transfer(SRAM_WRITE_MODE_REGISTER); // write to Mode Register
  SPI.transfer(SRAM_SEQUENTIAL_MODE);     // set sequential mode
  digitalWrite(csPin, HIGH);              // deselect SPI ram

  SPI.endTransaction();
}

void writeSPIByte(uint8_t csPin, unsigned long addr, uint8_t data) {
  SPI.beginTransaction(settings);
  digitalWrite(csPin, LOW);         // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  SPI.transfer(data);

  digitalWrite(csPin, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

uint8_t readSPIByte(uint8_t csPin, unsigned long addr) {
  SPI.beginTransaction(settings);
  digitalWrite(csPin, LOW);        // select SPI Ram
  SPI.transfer(SRAM_READ_COMMAND); // read command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  uint8_t result = SPI.transfer(dummy_byte);

  digitalWrite(csPin, HIGH); // deselect SPI ram
  SPI.endTransaction();
  return result;
}

void writeSPIWord(uint8_t csPin, unsigned long addr, uint16_t data) {
  SPI.beginTransaction(settings);
  digitalWrite(csPin, LOW);         // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  SPI.transfer((uint8_t)((data & 0xff)));
  SPI.transfer((uint8_t)((data >> 8) & 0xff));

  digitalWrite(csPin, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

uint16_t readSPIWord(uint8_t csPin, unsigned long addr) {
  SPI.beginTransaction(settings);
  digitalWrite(csPin, LOW);        // select SPI Ram
  SPI.transfer(SRAM_READ_COMMAND); // read command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  uint8_t low = SPI.transfer(dummy_byte);
  uint8_t high = SPI.transfer(dummy_byte);

  digitalWrite(csPin, HIGH); // deselect SPI ram
  SPI.endTransaction();
  return (high << 8) | low;
}

void fillMemory(uint8_t csPin, uint8_t data) {
  fillMemory(csPin, 0x0000, 0xffff, data);
}

void fillMemory(uint8_t csPin, unsigned long addr, unsigned long count,
                uint8_t data) {
  SPI.beginTransaction(settings);
  digitalWrite(csPin, LOW);         // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  for (unsigned long i = 0; i < count; i++) {
    SPI.transfer(data);
  }

  digitalWrite(csPin, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

bool verifyMemory(uint8_t csPin, uint8_t data) {
  return verifyMemory(csPin, 0x0000, 0xffff, data);
}

bool verifyMemory(uint8_t csPin, unsigned long addr, unsigned long count,
                  uint8_t data) {
  char buf[50];

  SPI.beginTransaction(settings);
  digitalWrite(csPin, LOW);        // select SPI Ram
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
      sprintf(buf, "(%02d) 0x%04X : read 0x%02X expect 0x%02X", csPin,
              (uint16_t)addr, result, data);

      Serial.println(buf);
      ok = false;
      break;
    }
  }

  digitalWrite(csPin, HIGH); // deselect SPI ram
  SPI.endTransaction();
  return ok;
}

void fillMemorySlow(uint8_t csPin, unsigned long addr, unsigned long count,
                    uint8_t data) {
  for (unsigned long i = 0; i < count; i++) {
    writeSPIByte(csPin, addr + i, data);
  }
}

bool verifyMemorySlow(uint8_t csPin, unsigned long addr, unsigned long count,
                      uint8_t data) {
  char buf[50];

  bool ok = true;

  for (unsigned long i = 0; i < count; i++) {
    uint8_t result = readSPIByte(csPin, addr + i);

    if (result != data) {
      sprintf(buf, "(%02d) 0x%04X : read 0x%02X expect 0x%02X", csPin,
              (uint16_t)addr, result, data);

      Serial.println(buf);
      ok = false;
      break;
    }
  }

  return ok;
}

void writeBlock(uint8_t csPin, uint16_t addr, uint8_t *data, uint16_t length) {
  SPI.beginTransaction(settings);
  digitalWrite(csPin, LOW);         // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  for (unsigned long i = 0; i < length; i++) {
    SPI.transfer(data[i]);
  }

  digitalWrite(csPin, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

void writeBlock(uint8_t csPin, uint16_t addr, uint16_t *data, uint16_t length) {
  SPI.beginTransaction(settings);
  digitalWrite(csPin, LOW);         // select SPI Ram
  SPI.transfer(SRAM_WRITE_COMMAND); // write command

#if SRAM_ADDRESS_SIZE == 3
  SPI.transfer((uint8_t)((addr >> 16) & 0xff));
#endif
  SPI.transfer((uint8_t)((addr >> 8) & 0xff));
  SPI.transfer((uint8_t)(addr & 0xff));

  for (unsigned long i = 0; i < length; i++) {
    SPI.transfer16(data[i]);
  }

  digitalWrite(csPin, HIGH); // deselect SPI ram
  SPI.endTransaction();
}

bool verifyBlock(uint8_t csPin, uint16_t addr, uint8_t *data, uint16_t length) {
  char buf[50];

  SPI.beginTransaction(settings);
  digitalWrite(csPin, LOW);        // select SPI Ram
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
      sprintf(buf, "(%02d) 0x%04X : read 0x%02X expect 0x%02X", csPin,
              (uint16_t)addr, result, data[i]);

      Serial.println(buf);
      ok = false;
      break;
    }
  }

  digitalWrite(csPin, HIGH); // deselect SPI ram
  SPI.endTransaction();
  return ok;
}
