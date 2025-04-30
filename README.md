# RAM provider for 65C02

This is a simple Arduino Mega2560 RAM "provider". There's a driver for a pair of 23LC512 SPI RAM chips (only one is supplied to the 6502 right now, because, well, it's a 64kb CPU).

The RAM chips are set to run in SEQUENTIAL mode which means reads and writes automatically increment the address "pointer". For byte-based operations like the 6502 access this isn't all that important. However, for the block-based operations, like clearing memory and loading the program, it speeds up tremendously (I didn't time it, but it's noticeably faster).

The platformio-based build will automatically assemble the 6502 "program" in `asm/pgm.S`. For now it's assumed that the program should be loaded at $300 and makes no attempt to do otherwise.

The pin connections for memory are as follows:

| Use               | Arduino pin / port |
| ----------------- | ------------------ |
| SRAM1 Chip Select | pin 10             |
| SRAM2 Chip Select | pin 9              |

The pin connections for the 6502 are as follows:

| Use                                | Arduino pin / port |
| ---------------------------------- | ------------------ |
| $\texttt{PHI2}$                    | pin 2              |
| $\texttt{R}/\overline{\texttt{W}}$ | pin 3              |
| $\texttt{SYNC}$                    | pin 4              |
| Data Bus $\texttt{D0-D7}$          | $\texttt{PORTL}$   |
| Addr Bus $\texttt{A0-A7}$          | $\texttt{PORTA}$   |
| Addr Bus $\texttt{A8-A15}$         | $\texttt{PORTC}$   |

## Details

The clock for this is an INPUT and is driven by an external clock module much like the Ben Eater module. There are plenty of pins on the 2560 to provide the clock directly to the 6502 and turn the interrupt inside-out, but I wanted to be able to single step through hardware but also fall back to a free-running mode.

Almost all of the pins are on the expansion header. One thing to note when connecting the SRAM chips is that you want to pull a second GND from the mega dev board over to the GND side of the SRAM. If you don't this simply won't work otherwise. Another thing is to keep the SRAM connections as short as possible, both between the dev board and breadboard, but also between chips on the breadboard. I dropped a $0.1 \mu\textsf{f}$ cap directly across each SRAM's VSS/VCC (and by direct and across I mean they straddle the DIP).
