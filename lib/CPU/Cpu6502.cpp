#include <Arduino.h>

#include "Cpu6502.h"
#include "SRAM_23LC512.h"

void setInitVector(uint8_t csPin, uint16_t addr) {
  writeSPIWord(csPin, 0xfffc, addr);
}
