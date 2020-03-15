#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <6502.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "c64.h"

#define IRQ_STACK_SIZE 128

extern void updatepalntsc(void);

/* Check if system is PAL
 * @return true if PAL
 */
void pal_system(void) {
    updatepalntsc();
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
    *(unsigned char *)CIA1_CRA &= ~CIA1_CR_START_STOP;

    // Inverse - make character ROM visible
    *(unsigned char *)CPU_PORT &= ~CPU_PORT_IO_VISIBLE_CHARACTER_ROM_INVISIBLE;

    memcpy(CHARACTER_START, CHARACTER_ROM, CHARACTER_ROM_SIZE);

    *(unsigned char *)CPU_PORT |= CPU_PORT_IO_VISIBLE_CHARACTER_ROM_INVISIBLE;

    *(unsigned char *)CIA1_CRA |= CIA1_CR_START_STOP;
}

static unsigned char _utils_lfn = 0x1f;

/** Get an unused logical file number for setlfs
 * @return The next unused LFN
 */
unsigned char utils_get_unused_lfn(void) {
    return _utils_lfn--;
}

/** Reset the screen to VIC bank #3
 * @param use_graphics_charset - Use fancy graphics chars with no lowercase
 * @param clear - Clear the screen before switching to it
 */
void __fastcall__ screen_init (bool use_graphics_charset, bool clear) {
    unsigned char screen_ptr = ((SCREEN_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_SCREEN_DIVISOR) << 4;

    // Update kernal
    *(unsigned char *)SCREEN_IO_HI_PTR = SCREEN_START >> 8;

    // Switch to bank 3
    *(unsigned char *)CIA2_PRA &= ~3;

    // Switch to screen memory
    *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_SCREEN_PTR_MASK);
    *(unsigned char *)VIC_VIDEO_ADR |= screen_ptr;

    // Switch to character memory
    *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_CHAR_PTR_MASK);
    *(unsigned char *)VIC_VIDEO_ADR |= (((CHARACTER_START + (use_graphics_charset * VIC_VIDEO_ADR_CHAR_DIVISOR)) % VIC_BANK_SIZE) / VIC_VIDEO_ADR_CHAR_DIVISOR) << 1;

    // Fix HLINE IRQ
    *(unsigned char*)VIC_HLINE = SCREEN_SPRITE_BORDER_Y_END;
    *(unsigned char*)VIC_CTRL1 &= ~VIC_CTRL1_HLINE_MSB;

    // Switch off bitmap mode
    *(unsigned char *)VIC_CTRL1 &= ~VIC_CTRL1_BITMAP_ON;
    *(unsigned char *)VIC_CTRL2 &= ~VIC_CTRL2_MULTICOLOR_ON;

    if(clear) {
        clrscr();
        bgcolor(COLOR_BLACK);
    }
}

char _itoabuf[10];

/** Write a value to stdout
 * @param value - The value to write
 */
void cputs_hex_value(int value) {
    cputs(itoa(value, _itoabuf, 16));
}

/** Get the file size in a stupid way.
 * @param filename - The filename to check.
 */
int get_filesize(char filename[]) {
    int total;
    int chunk;
    unsigned char* trash;
    FILE* fp;

    if(!(trash = malloc(256))) {
        return -1;
    }

    fp = fopen(filename, "rb");

    if(!(total = fread(trash, 1, 1, fp))) {
        fclose(fp);
        return -1;
    }

    while(chunk = fread(trash, 1, 800000, fp)) {
        total += chunk;
    }

    fclose(fp);

    return total;
}

unsigned char* irq_stack;

unsigned int raster_clock = 0;

unsigned char consume_raster_irq(void (*raster_handler)(void)) {
    if(*(unsigned char *)VIC_IRR & VIC_IRQ_RASTER) {
        *(unsigned char*)VIC_IRR |= VIC_IRQ_RASTER;

        raster_clock++;

        // If we're NTSC then we skip every 6th frame.
        if(!get_tv() && !(raster_clock % 6)) {
            return IRQ_HANDLED;
        }

        raster_handler();

        return IRQ_HANDLED;
    }

    return IRQ_NOT_HANDLED;
}

unsigned char setup_irq_handler(unsigned char (*handler)(void)) {
    if(!irq_stack) {
        if(!(irq_stack = malloc(IRQ_STACK_SIZE))) {
            return EXIT_FAILURE;
        }
    }

    set_irq(handler, irq_stack, IRQ_STACK_SIZE);

    return EXIT_SUCCESS;
}
