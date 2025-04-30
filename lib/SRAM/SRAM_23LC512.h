#ifndef SRAM_23LC512_H
#define SRAM_23LC512_H

#include <Arduino.h>
#include <SPI.h>

// whether to use 16 or 24 bit addresses, default to 16
#ifndef SRAM_ADDRESS_SIZE
#define SRAM_ADDRESS_SIZE 2
#endif

class SRAM_23LC512 {
private:
  uint8_t _cs;

public:
  SRAM_23LC512(uint8_t cs) {
    _cs = cs;

    digitalWrite(_cs, HIGH);
    pinMode(_cs, OUTPUT);
  }

  // initialize SRAM read/write modes
  void init();

  // SRAM data transfer
  void writeSPIByte(unsigned long addr, uint8_t data);
  uint8_t readSPIByte(unsigned long addr);
  void writeSPIWord(unsigned long addr, uint16_t data);
  uint16_t readSPIWord(unsigned long addr);

  // SRAM memory clear
  void fillMemory(uint8_t data);
  void fillMemory(unsigned long addr, unsigned long count, uint8_t data);

  // SRAM memory test
  bool verifyMemory(uint8_t data);
  bool verifyMemory(unsigned long addr, unsigned long count, uint8_t data);

  // SRAM fast block writes
  void writeBlock(uint16_t addr, uint8_t *data, uint16_t length);
  void writeBlock(uint16_t addr, uint16_t *data, uint16_t length);

  bool verifyBlock(uint16_t addr, uint8_t *data, uint16_t length);

private:
  bool verifyMemorySlow(unsigned long addr, unsigned long count, uint8_t data);
  void fillMemorySlow(unsigned long addr, unsigned long count, uint8_t data);
};

#endif // SRAM_23LC512H
