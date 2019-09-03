.export _sid_play
.export _sid_init

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
