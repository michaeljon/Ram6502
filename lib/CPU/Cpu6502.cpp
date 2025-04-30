#include <Arduino.h>

#include "Cpu6502.h"

void setInitVector(SRAM_23LC512 memory, uint16_t addr) {
  memory.writeSPIWord(0, 0xfffc, addr);
}
