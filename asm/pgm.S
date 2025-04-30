         ORG   $0300

* definitions

LEN      EQU   $08        ; number of bytes in number
ZERO     EQU   $50        ; initial value
ONE      EQU   $60        ; addend

* initialize the values

         LDA   #$00       ; zero the values
         LDX   #LEN
ZEROOUT  STA   ONE,X
         STA   ZERO,X
         DEX
         BNE   ZEROOUT
INITONE  LDA   #$01       ; set the lsb in ONE to 1
         LDX   #LEN
         STA   ONE,X

* run the loop

START    LDX   #LEN       ; set up the loop
         CLC

ADDW     LDA   ZERO,X     ; load the current value
         ADC   ONE,X      ; add the matching byte (0 or 1)
         STA   ZERO,X     ; store it
         DEX
         BNE   ADDW       ; continue as long as there are bytes
         JMP   START      ; go for another round
