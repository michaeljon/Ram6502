#!/usr/bin/env python

from subprocess import DEVNULL, STDOUT, check_call


print("Assembling asm/pgm.S")
check_call(["merlin32", "-V", "/usr/local/lib/merlin32", "asm/pgm.S"], stdout=DEVNULL, stderr=DEVNULL)

print("Converting binary to header")
with open("include/pgm.h", "wt") as header:
    check_call(["xxd", "-i", "asm/pgm"], stdout=header, stderr=DEVNULL)
