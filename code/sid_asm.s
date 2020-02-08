.export _sid_play
.export _sid_init
.export _sid_play_sound_int

.include "c64.inc"

.import popa, popax
.importzp ptr1

_sid_init:
    sei
    lda #$00
    tax
    tay
    jsr SID_START
    rts

_sid_play:
;    This is busy-wait for the "correct" raster line. We don't need it.
;    I hate whoever put this here.
;    lda #$40 
;    cmp $d012
;    bne *-3
    jsr SID_START+3
    rts

; void __fastcall__ sid_sound_play_int(unsigned char idx, unsigned char* startaddress)
; AX startaddress
; idx on stack

_sid_play_sound_int:
    sta ptr1
    stx ptr1+1
    jsr popa
    tax
    lda ptr1
    ldy ptr1+1
    ; channel X
    ; sound data lo/hi AY
    jsr SID_START+6
    rts
