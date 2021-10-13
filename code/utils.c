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

/** Disable the IO page
 */
void __fastcall__ hide_io(void) {
    *(unsigned char *)CIA1_CRA &= ~CIA1_CR_START_STOP;

    *(unsigned char *)CPU_PORT &= ~CPU_PORT_BANK_IO_VISIBLE_CHARACTER_ROM_INVISIBLE;
}

/** Enable the IO page
 */
void __fastcall__ show_io(void) {
    *(unsigned char *)CPU_PORT |= CPU_PORT_BANK_IO_VISIBLE_CHARACTER_ROM_INVISIBLE;

    *(unsigned char *)CIA1_CRA |= CIA1_CR_START_STOP;
}

/** Wait a number of milliseconds
 * @param duration - Milliseconds to wait
 */
void wait (unsigned int duration) {
    static unsigned int start;
    static unsigned int end;
    start = clock();
    end = start + duration * 6 / 100;
    while(clock() < end);
}

/** Copies character ROM to RAM.
 * @param use_graphics_charset - Use fancy graphics chars with no lowercase
 */
void character_init(bool use_graphics_charset) {
    hide_io();
    memcpy(CHARACTER_START, CHARACTER_ROM + (!use_graphics_charset * VIC_VIDEO_ADR_CHAR_DIVISOR), CHARACTER_ROM_SIZE);
    show_io();
}

static unsigned char _utils_lfn = 0x1f;

/** Get an unused logical file number for setlfs
 * @return The next unused LFN
 */
unsigned char utils_get_unused_lfn(void) {
    return _utils_lfn--;
}

/** Reset the screen to VIC bank #3
 * @param clear - Clear the screen before switching to it
 */
void __fastcall__ screen_init (bool clear) {
        unsigned char screen_ptr = ((SCREEN_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_SCREEN_DIVISOR) << 4;

    // Update kernal
    *(unsigned char *)SCREEN_IO_HI_PTR = SCREEN_START >> 8;

    // Switch to bank 3
    *(unsigned char *)CIA2_PRA &= ~3;

    // Switch to screen memory
    VIC.addr &= ~(VIC_VIDEO_ADR_SCREEN_PTR_MASK);
    VIC.addr |= screen_ptr;

    // Switch to character memory
    VIC.addr &= ~(VIC_VIDEO_ADR_CHAR_PTR_MASK);
    VIC.addr |= ((CHARACTER_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_CHAR_DIVISOR) << 1;

    // Switch off bitmap mode
    VIC.ctrl1 &= ~VIC_CTRL1_BITMAP_ON;
    VIC.ctrl2 &= ~VIC_CTRL2_MULTICOLOR_ON;

    if(clear) {
        clrscr();
        printf("hallo!");

        VIC.bgcolor0 = COLOR_BLACK;
        VIC.bgcolor1 = COLOR_BLACK;
        VIC.bgcolor2 = COLOR_BLACK;
        VIC.bgcolor3 = COLOR_BLACK;
        VIC.bordercolor = COLOR_BLACK;
        bordercolor(COLOR_BLACK);
    }

    // Fix HLINE IRQ
    VIC.rasterline = 0xff;
    VIC.ctrl1 &= ~VIC_CTRL1_HLINE_MSB;

    // Enable raster interrupts
    VIC.imr |= VIC_IRQ_RASTER;
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
    static int total;
    static int chunk;
    static unsigned char* trash;
    static FILE* fp;

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

bool irq_setup_done = false;
unsigned char setup_irq_handler() {
    irq_setup_done = true;
    return EXIT_SUCCESS;
}

unsigned char destroy_irq_handler() {
    irq_setup_done = false;
    return EXIT_SUCCESS;
}

unsigned char game_clock = 0;
bool is_pal = false;