#ifndef SRAM_23LC512_H
#define SRAM_23LC512_H

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

// whether to use 16 or 24 bit addresses, default to 16
#ifndef SRAM_ADDRESS_SIZE
#define SRAM_ADDRESS_SIZE 2
#endif

// initialize SRAM read/write modes
void initRAM(uint8_t csPin);

// SRAM data transfer
void writeSPIByte(uint8_t csPin, unsigned long addr, uint8_t data);
uint8_t readSPIByte(uint8_t csPin, unsigned long addr);
void writeSPIWord(uint8_t csPin, unsigned long addr, uint16_t data);
uint16_t readSPIWord(uint8_t csPin, unsigned long addr);

// SRAM memory clear
void fillMemory(uint8_t csPin, uint8_t data);
void fillMemory(uint8_t csPin, unsigned long addr, unsigned long count,
                uint8_t data);

// SRAM memory test
bool verifyMemory(uint8_t csPin, uint8_t data);
bool verifyMemory(uint8_t csPin, unsigned long addr, unsigned long count,
                  uint8_t data);

// SRAM fast block writes
void writeBlock(uint8_t csPin, uint16_t addr, uint8_t *data, uint16_t length);
void writeBlock(uint8_t csPin, uint16_t addr, uint16_t *data, uint16_t length);

bool verifyBlock(uint8_t csPin, uint16_t addr, uint8_t *data, uint16_t length);

#endif // SRAM_23LC512H
