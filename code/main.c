#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <tgi.h>
#include <time.h>
#include <joystick.h>
#include <conio.h>
#include "koala.h"
#include "sid.h"
#include "c64.h"
#include "utils.h"
#include "sprite.h"
#include "level.h"

extern const unsigned char r_text_loading[];
extern const unsigned char r_text_loading2[];
extern const unsigned char r_text_loading3[];

unsigned char intro_screen() {
    unsigned char err;
    unsigned int now = clock();
    unsigned char joyval;
    unsigned char kbchar;
    unsigned char* snz_pointer;

    screen_init(false);
    clrscr();

    puts(r_text_loading);

    if(err = sid_load("intro.sid")) {
        printf("There was a problem loading the SID: %x\n", err);
        return EXIT_FAILURE;
    }

    puts(r_text_loading2);

    if(!(snz_pointer = snz_load("canada.snz", &err))) {
        printf("There was a problem loading the sounds: %x\n", err);
        return EXIT_FAILURE;
    }

    puts(r_text_loading3);

    if(err = koala_load("intro.koa")) {
        printf("There was a problem loading the intro bitmap: %x\n", err);
        return EXIT_FAILURE;
    }

    while(1) {
        sid_play_frame();

        kbchar = 0x00;
        if(kbhit()) {
            kbchar = cgetc();
        }

        if(kbchar) {
            if(kbchar == 'p') {
                sid_play_sound(snz_pointer, 0, 2);
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

    puts(r_text_loading2);

    character_init();

    puts(r_text_loading3);

    if(err = spritesheet_load("canada.spd")) {
        printf("There was a problem loading the sprites: %x\n", err);
        while(1);
    }

    if(err = intro_screen()) {
        screen_init(false);
        while(1);
    }

    if(err = play_level()) {
        printf("There was a problem with the level %x\n", err);
        screen_init(false);
        while(1);
    }

    screen_init(true);
    clrscr();

    return EXIT_SUCCESS;
}
