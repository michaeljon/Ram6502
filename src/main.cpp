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

bool tick = false;
void onClock() { tick = true; }

void setup() {
#ifndef SERIAL_DIAGNOSTICS
  pinMode(LED_BUILTIN, OUTPUT);
#else
  Serial.begin(115200);
#endif

  digitalWrite(SRAM1_CS, HIGH);
  pinMode(SRAM1_CS, OUTPUT);

  digitalWrite(SRAM2_CS, HIGH);
  pinMode(SRAM2_CS, OUTPUT);

  SPI.begin();

#if defined(SERIAL_DIAGNOSTICS)
  Serial.println("Initializing RAM");
#endif
  initRAM(SRAM1_CS);
  initRAM(SRAM2_CS);

#if defined(TEST_SRAM1_MEM)
  Serial.println("Testing SRAM1");
  fillMemory(SRAM1_CS, 0x12);
  Serial.println("Verifying SRAM1");
  if (verifyMemory(SRAM1_CS, 0x12) == true) {
    Serial.println("SRAM1 OK");
  } else {
    Serial.println("Failed to verify SRAM1");
    while (1)
      ;
  }
#endif

#if defined(TEST_SRAM2_MEM)
  Serial.println("Testing SRAM2");
  fillMemory(SRAM2_CS, 0x34);
  Serial.println("Verifying SRAM2");
  if (verifyMemory(SRAM2_CS, 0x34) == true) {
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
  fillMemory(SRAM1_CS, 0xEA);
  fillMemory(SRAM2_CS, 0xEA);

#if defined(SERIAL_DIAGNOSTICS)
  Serial.println("Setting init vector");
#endif
  setInitVector(SRAM1_CS, 0x0300);

#if defined(SERIAL_DIAGNOSTICS)
  Serial.println("Loading program");
#endif
  writeBlock(SRAM1_CS, 0x0300, asm_pgm, asm_pgm_len);

  Serial.println("Verifying program");
  if (verifyBlock(SRAM1_CS, 0x0300, asm_pgm, asm_pgm_len) == true) {
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
      data = readSPIByte(SRAM1_CS, address);
      // RWB=1=read (CPU is reading so we are writing to pins)
      PORTL = data;
      DDRL = 0xFF;
    } else {
      // RWB=0=write (CPU is writing so we are reading pins)
      DDRL = 0x00;
      data = PINL;
      // RWB=0=write write data to RAM
      writeSPIByte(SRAM1_CS, address, data);
    }

#if defined(SERIAL_DIAGNOSTICS)
    sprintf(output, "   $%04X %c $%02X %c", address, (syncPin ? '*' : ' '),
            data, (readWritePin ? 'R' : 'W'));
    Serial.println(output);
#endif

    tick = false;
  }
}
