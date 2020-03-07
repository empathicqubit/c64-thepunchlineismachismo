// From cc65/cc65/asminc/c64.inc
//
// C64 generic definitions. Stolen from Elite128
//
// I prefer using this over some of the builtin structs because it is easier to 
// match to assembly code docs.

// ---------------------------------------------------------------------------
// Zero page, Commodore stuff

#define CPU_PORT                                    0x01
#define CPU_PORT_BASIC_ROM_VISIBLE_KERNAL_VISIBLE   0x03
#define CPU_PORT_IO_VISIBLE_CHARACTER_ROM_INVISIBLE 0x04

#define VARTAB 0x2D
#define MEMSIZE          0x37          // Pointer to highest BASIC RAM location (+1)
#define TXTPTR           0x7A          // Pointer into BASIC source code
#define TIME             0xA0          // 60 HZ clock
#define FNAM_LEN         0xB7          // Length of filename
#define SECADR           0xB9          // Secondary address
#define DEVNUM           0xBA          // Device number
#define FNAM             0xBB          // Pointer to filename
#define KEY_COUNT        0xC6          // Number of keys in input buffer
#define RVS              0xC7          // Reverse flag
#define CURS_FLAG        0xCC          // 1  cursor off
#define CURS_BLINK       0xCD          // Blink counter
#define CURS_CHAR        0xCE          // Character under the cursor
#define CURS_STATE       0xCF          // Cursor blink state
#define SCREEN_PTR       0xD1          // Pointer to current char in text screen
#define CURS_X           0xD3          // Cursor column
#define CURS_Y           0xD6          // Cursor row
#define CRAM_PTR         0xF3          // Pointer to current char in color RAM
#define FREKZP           0xFB          // Five unused bytes

#define SCREEN_BYTES 1000

#define SCREEN_BITMAP_WIDTH 320
#define SCREEN_BITMAP_HEIGHT 200
#define SCREEN_BITMAP_SIZE 8000

#define SCREEN_SPRITE_WIDTH 512
#define SCREEN_SPRITE_HEIGHT 255

#define SCREEN_SPRITE_BORDER_Y_START 50
#define SCREEN_SPRITE_BORDER_Y_END 250

#define SCREEN_SPRITE_BORDER_HEIGHT (SCREEN_SPRITE_BORDER_Y_END - SCREEN_SPRITE_BORDER_Y_START)

#define SCREEN_SPRITE_BORDER_X_START 20
#define SCREEN_SPRITE_BORDER_X_END 350

#define SCREEN_SPRITE_BORDER_WIDTH (SCREEN_SPRITE_BORDER_X_END - SCREEN_SPRITE_BORDER_X_START)

#define BASIC_BUF        0x200         // Location of command-line
#define BASIC_BUF_LEN    89            // Maximum length of command-line

#define CHARCOLOR        0x286
#define CURS_COLOR       0x287         // Color under the cursor
#define SCREEN_IO_HI_PTR 0x288
#define PALFLAG          0x2A6         // 0x01  PAL, 0x00  NTSC

#define KBDREPEAT        0x28a
#define KBDREPEATRATE    0x28b
#define KBDREPEATDELAY   0x28c

// ---------------------------------------------------------------------------
// Vector and other locations

#define IRQVec           0x0314
#define BRKVec           0x0316
#define NMIVec           0x0318

#define IRQVec_DEFAULT_ISR   0xEA31

// ---------------------------------------------------------------------------
// Screen size

#define XSIZE            40
#define YSIZE            25

// ---------------------------------------------------------------------------
// I/O: VIC

#define CHARACTER_ROM    0xD000

#define VIC_SPR0_X       0xD000
#define VIC_SPR0_Y       0xD001
#define VIC_SPR1_X       0xD002
#define VIC_SPR1_Y       0xD003
#define VIC_SPR2_X       0xD004
#define VIC_SPR2_Y       0xD005
#define VIC_SPR3_X       0xD006
#define VIC_SPR3_Y       0xD007
#define VIC_SPR4_X       0xD008
#define VIC_SPR4_Y       0xD009
#define VIC_SPR5_X       0xD00A
#define VIC_SPR5_Y       0xD00B
#define VIC_SPR6_X       0xD00C
#define VIC_SPR6_Y       0xD00D
#define VIC_SPR7_X       0xD00E
#define VIC_SPR7_Y       0xD00F
#define VIC_SPR_HI_X     0xD010
#define VIC_SPR_ENA      0xD015
#define VIC_SPR_EXP_Y    0xD017
#define VIC_SPR_EXP_X    0xD01D
#define VIC_SPR_MCOLOR   0xD01C
#define VIC_SPR_BG_PRIO  0xD01B

#define VIC_SPR_MCOLOR0  0xD025
#define VIC_SPR_MCOLOR1  0xD026

#define VIC_SPR0_COLOR   0xD027
#define VIC_SPR1_COLOR   0xD028
#define VIC_SPR2_COLOR   0xD029
#define VIC_SPR3_COLOR   0xD02A
#define VIC_SPR4_COLOR   0xD02B
#define VIC_SPR5_COLOR   0xD02C
#define VIC_SPR6_COLOR   0xD02D
#define VIC_SPR7_COLOR   0xD02E

#define VIC_CTRL1        0xD011
#define VIC_CTRL2        0xD016

#define VIC_SPR_SIZE     0x40
#define VIC_SPR_COUNT     0x08

#define JOY_ANY_MASK (JOY_UP_MASK | JOY_DOWN_MASK | JOY_LEFT_MASK | JOY_RIGHT_MASK | JOY_BTN_1_MASK)

#define VIC_CTRL1_BITMAP_ON     0x20
#define VIC_CTRL1_HLINE_MSB     0x80
#define VIC_CTRL2_MULTICOLOR_ON 0x10

#define VIC_HLINE        0xD012

#define VIC_LPEN_X       0xD013
#define VIC_LPEN_Y       0xD014

#define VIC_VIDEO_ADR    0xD018

// Character memory pointer bits MSB-LSB
#define VIC_VIDEO_ADR_CHAR_PTR_MASK 0x0E
#define VIC_VIDEO_ADR_CHAR_PTR0 0x08
#define VIC_VIDEO_ADR_CHAR_PTR1 0x04
#define VIC_VIDEO_ADR_CHAR_PTR2 0x02

#define VIC_VIDEO_ADR_CHAR_DIVISOR 0x800
#define VIC_VIDEO_ADR_SCREEN_DIVISOR 0x400

#define VIC_VIDEO_ADR_CHAR_BANK_GRAPHICS_OFFSET 0x1000
#define VIC_VIDEO_ADR_CHAR_BANK_SMALLCASE_OFFSET 0x1800

// Screen memory pointer bits MSB-LSB
#define VIC_VIDEO_ADR_SCREEN_PTR_MASK 0xF0
#define VIC_VIDEO_ADR_SCREEN_PTR0 0x80
#define VIC_VIDEO_ADR_SCREEN_PTR1 0x40
#define VIC_VIDEO_ADR_SCREEN_PTR2 0x20
#define VIC_VIDEO_ADR_SCREEN_PTR3 0x10

#define VIC_BANK_SIZE 0x4000

#define VIC_IRR          0xD019        // Interrupt request register
#define VIC_IMR          0xD01A        // Interrupt mask register

#define VIC_IRQ_RASTER 0x01
#define VIC_IRQ_SPRITE_BG 0x02
#define VIC_IRQ_SPRITE_SPRITE 0x04
#define VIC_IRQ_LIGHT_PEN 0x08

#define COLOR_RAM_SIZE 1000

#define VIC_BORDERCOLOR  0xD020
#define VIC_BG_COLOR0    0xD021
#define VIC_BG_COLOR1    0xD022
#define VIC_BG_COLOR2    0xD023
#define VIC_BG_COLOR3    0xD024
// 128 stuff:
#define VIC_KBD_128      0xD02F        // Extended kbd bits (visible in 64 mode)
#define VIC_CLK_128      0xD030        // Clock rate register (visible in 64 mode)

// I/O: SID

#define SID_S1Lo         0xD400
#define SID_S1Hi         0xD401
#define SID_PB1Lo        0xD402
#define SID_PB1Hi        0xD403
#define SID_Ctl1         0xD404
#define SID_AD1          0xD405
#define SID_SUR1         0xD406

#define SID_S2Lo         0xD407
#define SID_S2Hi         0xD408
#define SID_PB2Lo        0xD409
#define SID_PB2Hi        0xD40A
#define SID_Ctl2         0xD40B
#define SID_AD2          0xD40C
#define SID_SUR2         0xD40D

#define SID_S3Lo         0xD40E
#define SID_S3Hi         0xD40F
#define SID_PB3Lo        0xD410
#define SID_PB3Hi        0xD411
#define SID_Ctl3         0xD412
#define SID_AD3          0xD413
#define SID_SUR3         0xD414

#define SID_FltLo        0xD415
#define SID_FltHi        0xD416
#define SID_FltCtl       0xD417
#define SID_Amp          0xD418
#define SID_ADConv1      0xD419
#define SID_ADConv2      0xD41A
#define SID_Noise        0xD41B
#define SID_Read3        0xD41C

// I/O: VDC (128 only)

#define VDC_INDEX        0xD600
#define VDC_DATA         0xD601

// I/O: Complex Interface Adapters

#define CIA1_PRA         0xDC00        // Port A
#define CIA1_PRB         0xDC01        // Port B
#define CIA1_DDRA        0xDC02        // Data direction register for port A
#define CIA1_DDRB        0xDC03        // Data direction register for port B
#define CIA1_TA          0xDC04        // 16-bit timer A
#define CIA1_TB          0xDC06        // 16-bit timer B
#define CIA1_TOD10       0xDC08        // Time-of-day tenths of a second
#define CIA1_TODSEC      0xDC09        // Time-of-day seconds
#define CIA1_TODMIN      0xDC0A        // Time-of-day minutes
#define CIA1_TODHR       0xDC0B        // Time-of-day hours
#define CIA1_SDR         0xDC0C        // Serial data register
#define CIA1_ICR         0xDC0D        // Interrupt control register
#define CIA1_CRA         0xDC0E        // Control register for timer A
#define CIA1_CRB         0xDC0F        // Control register for timer B

#define CIA1_CR_START_STOP 0x01

#define CIA2_PRA_VIC_BANK0 0x02
#define CIA2_PRA_VIC_BANK1 0x01

#define CIA2_PRA         0xDD00
#define CIA2_PRB         0xDD01
#define CIA2_DDRA        0xDD02
#define CIA2_DDRB        0xDD03
#define CIA2_TA          0xDD04
#define CIA2_TB          0xDD06
#define CIA2_TOD10       0xDD08
#define CIA2_TODSEC      0xDD09
#define CIA2_TODMIN      0xDD0A
#define CIA2_TODHR       0xDD0B
#define CIA2_SDR         0xDD0C
#define CIA2_ICR         0xDD0D
#define CIA2_CRA         0xDD0E
#define CIA2_CRB         0xDD0F

// Super CPU
#define SCPU_VIC_Bank1   0xD075
#define SCPU_Slow        0xD07A
#define SCPU_Fast        0xD07B
#define SCPU_EnableRegs  0xD07E
#define SCPU_DisableRegs 0xD07F
#define SCPU_Detect      0xD0BC

// Processor Port at 0x01

#define LORAM            0x01           // Enable the basic rom
#define HIRAM            0x02           // Enable the kernal rom
#define IOEN             0x04           // Enable I/O
#define CASSDATA         0x08           // Cassette data
#define CASSPLAY         0x10           // Cassette: Play
#define CASSMOT          0x20           // Cassette motor on
#define TP_FAST          0x80           // Switch Rossmoeller TurboProcess to fast mode

#define RAMONLY          0xF8           // (~(LORAM | HIRAM | IOEN)) & 0xFF
