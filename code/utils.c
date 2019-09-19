#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <conio.h>
#include "c64.h"

extern void updatepalntsc(void);

/* Check if system is PAL
 * @return true if PAL
 */
bool pal_system(void) {
    updatepalntsc();
    return *(bool *)PALFLAG;
}

/* Wait a number of milliseconds
 * @param duration - Milliseconds to wait
 */
void wait (unsigned int duration) {
    unsigned int start = clock();
    unsigned int end = start + duration * 6 / 100;
    while(clock() < end);
}

/* Copies character ROM to RAM.
 */
void character_init(void) {
    unsigned char i;

    *(unsigned char *)CIA1_CRA &= ~CIA1_CR_START_STOP;

    // Inverse - make character ROM visible
    *(unsigned char *)CPU_PORT &= ~CPU_PORT_IO_VISIBLE_CHARACTER_ROM_INVISIBLE;

    memcpy(CHARACTER_ROM, CHARACTER_ROM, 0x1000);

    *(unsigned char *)CPU_PORT |= CPU_PORT_IO_VISIBLE_CHARACTER_ROM_INVISIBLE;

    *(unsigned char *)CIA1_CRA |= CIA1_CR_START_STOP;
}

/* Reset the screen to VIC bank #3
 * @param use_graphics_charset - Use fancy graphics chars with no lowercase
 */
void screen_init (bool use_graphics_charset) {
    unsigned char screen_ptr = ((SCREEN_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_SCREEN_DIVISOR) << 4;

    // Switch to bank 3
    *(unsigned char *)CIA2_PRA &= 0x00;

    // Switch to screen memory
    *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_SCREEN_PTR_MASK);
    *(unsigned char *)VIC_VIDEO_ADR |= screen_ptr;

    // Switch to character memory (0x1800)
    *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_CHAR_PTR_MASK);
    *(unsigned char *)VIC_VIDEO_ADR |= ((VIC_VIDEO_ADR_CHAR_BANK_SMALLCASE_OFFSET - (use_graphics_charset * VIC_VIDEO_ADR_CHAR_DIVISOR)) / VIC_VIDEO_ADR_CHAR_DIVISOR) << 1;

    // Switch off bitmap mode
    *(unsigned char *)VIC_CTRL1 &= ~VIC_CTRL1_BITMAP_ON;
    *(unsigned char *)VIC_CTRL2 &= ~VIC_CTRL2_MULTICOLOR_ON;

    // Update kernal
    *(unsigned char *)SCREEN_IO_HI_PTR = SCREEN_START >> 8;

    bgcolor(COLOR_BLACK);
}

