.export _sid_play
.export _sid_init
.export _sid_play_sound_int

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
    lda #$40
    cmp $d012
    bne *-3
    inc $d020
    jsr SID_START+3
    dec $d020
    rts

; void __fastcall__ sid_sound_play_int(unsigned char idx, unsigned char* startaddress)

_sid_play_sound_int:
    sta ptr1
    stx ptr1+1
    jsr popa
    tax
    lda ptr1
    ldy ptr1+1
    jsr SID_START+6
    rts
