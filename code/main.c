#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <tgi.h>
#include <string.h>
#include <time.h>
#include <joystick.h>
#include <conio.h>
#include "koala.h"
#include "sid.h"
#include "c64.h"
#include "sprite.h"

extern const unsigned char r_text_loading[];
extern const unsigned char r_text_loading2[];

extern const unsigned char r_sprites_heart[];

void wait (unsigned int duration) {
    unsigned int start = clock();
    unsigned int end = start + duration * 6 / 100;
    while(clock() < end);
}

void screen_init (void) {
    unsigned char screen_ptr = ((SCREEN_START % VIC_BANK_SIZE) / 0x400) << 4;

    // Switch to bank 2
    *(unsigned char *)CIA2_PRA &= ~(CIA2_PRA_VIC_BANK0);
    *(unsigned char *)CIA2_PRA |= CIA2_PRA_VIC_BANK1;

    // Switch to screen memory
    *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_SCREEN_PTR_MASK);
    *(unsigned char *)VIC_VIDEO_ADR |= screen_ptr;

    // Update kernal
    *(unsigned char *)SCREEN_IO_HI_PTR = SCREEN_START >> 8;

    clrscr();
}

unsigned char main (void) {
    unsigned int last = -1;
    unsigned int spritex = 75;
    unsigned char spritey = 75;
    unsigned int now = clock();
    unsigned char textrepeat = 0;
    unsigned int loadtextlen = strlen(r_text_loading);
    unsigned char* heart_addr = sprite_next_addr();
    unsigned char err;

    srand(clock());

    screen_init();

    puts(r_text_loading);

    //40x25
    if(err = sid_load("intro.sid")) {
        printf("There was a problem loading the SID: %x\n", err);
    }

    puts(r_text_loading2);

    memcpy(heart_addr, r_sprites_heart, VIC_SPR_SIZE);

    puts(r_text_loading2);

    *(char *)VIC_SPR_MCOLOR0 = VIC_COLOR_GREEN;
    *(char *)VIC_SPR_MCOLOR1 = VIC_COLOR_BLUE;

    if(err = sprite_load(heart_addr, 0, spritex, spritey, true, COLOR_RED)) {
        printf("There was a problem loading the sprite: %x\n", err);
    }

    if(err = koala_load("intro.koa")) {
        printf("There was a problem loading the intro bitmap: %x\n", err);
    }

    while(1) {
        sid_play_frame();
        now = clock();
        if(now > last + 30) {
            spritex++;
            sprite_move(0, spritex, spritey);
        }
    }

    return EXIT_SUCCESS;
}
