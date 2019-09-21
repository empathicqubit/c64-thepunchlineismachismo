.export _updatepalntsc
 ; Reliable PAL/NTSC-Detector by Ninja/The Dreams/TempesT
 ; for Go64!/CW issue 06/2000 (detailed description there)
 ; This routine can't be fooled like $02a6 and works also with a SCPU

.include "c64.inc"

nmivec        = $0318                     ; NMI-vector

_updatepalntsc:
    jsr palntsc                 ; perform check
    sta PALFLAG                   ; update KERNAL-variable
    rts
  
palntsc:
    sei                         ; disable interrupts
    ldx nmivec
    ldy nmivec+1                ; remember old NMI-vector
    lda #<(done)
    sta nmivec
    lda #>(done)                ; let NMI-vector point to
    sta nmivec+1                ; a RTI
wait:
    lda VIC_HLINE
    bne wait                    ; wait for rasterline 0 or 256
    lda #$37
    sta VIC_HLINE
    lda #$9b                    ; write testline $137 to the
    sta VIC_CTRL1
    lda #$01
    sta VIC_IRR                   ; clear IMR-Bit 0
wait1:
    lda VIC_CTRL1                   ; Is rasterbeam in the area
    bpl wait1                   ; 0-255? if yes, wait
wait2:
    lda VIC_CTRL1                   ; Is rasterbeam in the area
    bmi wait2                   ; 256 to end? if yes, wait
    lda VIC_IRR                   ; read IMR
    and #$01                    ; mask Bit 0
    sta VIC_IRR                   ; clear IMR-Bit 0
    stx nmivec
    sty nmivec+1                ; restore old NMI-vector
    cli                         ; enable interrupts
    rts                         ; return

done:
    rti
