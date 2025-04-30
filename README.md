# RAM provider for 65C02

This is a simple Arduino Mega2560 RAM "provider". There's a driver for a pair of 23LC512 SPI RAM chips (only one is supplied to the 6502 right now, because, well, it's a 64kb CPU).

The platformio-based build will automatically assemble the 6502 "program" in `asm/pgm.S`. For now it's assumed that the program should be loaded at `$300` and makes no attempt to do otherwise.