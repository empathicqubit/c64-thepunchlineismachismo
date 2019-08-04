.export _sid_play
.export _sid_init

_sid_init:
    sei
    lda #$00
    tax
    tay
    jsr $8000
    rts

_sid_play:
    lda #$40
    cmp $d012
    bne *-3
    inc $d020
    jsr $8003
    dec $d020
    rts
