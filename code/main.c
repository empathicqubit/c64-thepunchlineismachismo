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
#include "../resources/sprites/canada.h"

extern const unsigned char r_text_loading[];
extern const unsigned char r_text_loading2[];
extern const unsigned char r_text_loading3[];

void wait (unsigned int duration) {
    unsigned int start = clock();
    unsigned int end = start + duration * 6 / 100;
    while(clock() < end);
}

void screen_init (bool use_graphics_charset) {
    unsigned char screen_ptr = ((SCREEN_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_SCREEN_DIVISOR) << 4;

    // Switch to bank 2
    *(unsigned char *)CIA2_PRA &= ~(CIA2_PRA_VIC_BANK0);
    *(unsigned char *)CIA2_PRA |= CIA2_PRA_VIC_BANK1;

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
}

unsigned char intro_screen() {
    unsigned char err;
    unsigned int spritex = 75;
    unsigned char spritey = 75;
    unsigned int last = 0;
    unsigned int now = clock();
    unsigned char textrepeat = 0;
    unsigned int loadtextlen = strlen(r_text_loading);
    unsigned char joyval;

    screen_init(false);
    clrscr();

    puts(r_text_loading);

    if(err = sid_load("intro.sid")) {
        printf("There was a problem loading the SID: %x\n", err);
        return EXIT_FAILURE;
    }

    puts(r_text_loading2);

    if(err = spritesheet_load("canada.spd")) {
        printf("There was a problem loading the sprites: %x\n", err);
        return EXIT_FAILURE;
    }

    if(err = spritesheet_show(0, SPRITES_GUY_WALK_RIGHT_1, spritex, spritey, true, true)) {
        printf("There was a problem loading the sprites: %x\n", err);
        return EXIT_FAILURE;
    }

    puts(r_text_loading3);

    if(err = koala_load("intro.koa")) {
        printf("There was a problem loading the intro bitmap: %x\n", err);
        return EXIT_FAILURE;
    }

    while(1) {
        sid_play_frame();

        now = clock();
        if(now > last + 1) {
            last = now;
            spritex++;
            if(spritex > SCREEN_BITMAP_WIDTH) {
                spritex = 0;
            }

            *(unsigned int*)BITMAP_START = spritex;

            if(
                (err = sprite_move(0, spritex, spritey))
                || (err = spritesheet_next_image(0, SPRITES_GUY_WALK_RIGHT_0, SPRITES_GUY_WALK_RIGHT_END))
                ) {
                printf("There was a problem updating the sprite: %x\n", err);
                return EXIT_FAILURE;
            }
        }

        joyval = joy_read(0x01);
        if(joyval & JOY_ANY_MASK) {
            break;
        }
    }

    sid_stop();

    return EXIT_SUCCESS;
}

unsigned char main (void) {
    unsigned char err;

    srand(clock());

    joy_install(&joy_static_stddrv);

    if(err = intro_screen()) {
        screen_init(false);
        while(1);
    }

    screen_init(true);
    clrscr();

    return EXIT_SUCCESS;
}
