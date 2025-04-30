#include <Arduino.h>
#include <SPI.h>

#include "Cpu6502.h"
#include "SRAM_23LC512.h"

#include "pgm.h"

// SRAM chip select
#define SRAM1_CS 10
#define SRAM2_CS 9

// 6502 pins
#define CLOCK 2
#define READ_WRITE 3
#define SYNC 4

// serial debug
#define SERIAL_DIAGNOSTICS

#define TEST_SRAM1_MEM
#define TEST_SRAM2_MEM

SRAM_23LC512 lowMemory(SRAM1_CS);
SRAM_23LC512 highMemory(SRAM2_CS);

bool tick = false;
void onClock() { tick = true; }

void setup() {
#ifndef SERIAL_DIAGNOSTICS
  pinMode(LED_BUILTIN, OUTPUT);
#else
  Serial.begin(115200);
  while (!Serial)
    ;
#endif

  SPI.begin();

#if defined(SERIAL_DIAGNOSTICS)
  Serial.println("Initializing RAM");
#endif
  lowMemory.init();
  highMemory.init();

#if defined(TEST_SRAM1_MEM)
  Serial.println("Testing SRAM1");
  lowMemory.fillMemory(0x12);
  Serial.println("Verifying SRAM1");
  if (lowMemory.verifyMemory(0x12) == true) {
    Serial.println("SRAM1 OK");
  } else {
    Serial.println("Failed to verify SRAM1");
    while (1)
      ;
  }
#endif

#if defined(TEST_SRAM2_MEM)
  Serial.println("Testing SRAM2");
  highMemory.fillMemory(0x34);
  Serial.println("Verifying SRAM2");
  if (highMemory.verifyMemory(0x34) == true) {
    Serial.println("SRAM2 OK");
  } else {
    Serial.println("Failed to verify SRAM2");
    while (1)
      ;
  }
#endif

#if defined(SERIAL_DIAGNOSTICS)
  Serial.println("Setting RAM to 0xEA");
#endif
  lowMemory.fillMemory(0xEA);
  highMemory.fillMemory(0xEA);

#if defined(SERIAL_DIAGNOSTICS)
  Serial.println("Setting init vector");
#endif
  setInitVector(lowMemory, 0x0300);

#if defined(SERIAL_DIAGNOSTICS)
  Serial.println("Loading program");
#endif
  lowMemory.writeBlock(0x0300, asm_pgm, asm_pgm_len);

  Serial.println("Verifying program");
  if (lowMemory.verifyBlock(0x0300, asm_pgm, asm_pgm_len) == true) {
    Serial.println("Program OK");
  } else {
    Serial.println("Failed to load program");
    while (1)
      ;
  }

  attachInterrupt(digitalPinToInterrupt(CLOCK), onClock, RISING);
}

void loop() {
  if (tick == true) {
    char output[15];

    uint16_t address = PINA + (PINC << 8);

    //int readWrite = digitalRead(READ_WRITE);
    //int sync = digitalRead(SYNC);
    int readWritePin = PINE & (1 << PE5); // READ_WRITE: PIN 3 is PE5
    int syncPin = PING & (1 << PG5);      // SYNC: PIN 4 is PG5

    unsigned char data = 0;

    if (readWritePin) {
      // RWB=1=read get data from RAM
      data = lowMemory.readSPIByte(address);
      // RWB=1=read (CPU is reading so we are writing to pins)
      PORTL = data;
      DDRL = 0xFF;
    } else {
      // RWB=0=write (CPU is writing so we are reading pins)
      DDRL = 0x00;
      data = PINL;
      // RWB=0=write write data to RAM
      lowMemory.writeSPIByte(address, data);
    }

#if defined(SERIAL_DIAGNOSTICS)
    sprintf(output, "   $%04X %c $%02X %c", address, (syncPin ? '*' : ' '),
            data, (readWritePin ? 'R' : 'W'));
    Serial.println(output);
#endif

    tick = false;
  }
}
