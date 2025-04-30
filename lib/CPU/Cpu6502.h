#ifndef _CPU6502_H
#define _CPU6502_H

#include "SRAM_23LC512.h"

void setInitVector(SRAM_23LC512 memory, uint16_t addr);

#endif // _CPU6502_H