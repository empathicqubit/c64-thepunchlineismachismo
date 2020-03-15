#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <tgi.h>
#include <time.h>
#include <joystick.h>
#include <conio.h>
#include <6502.h>
#include "ocp.h"
#include "sid.h"
#include "c64.h"
#include "utils.h"
#include "sprite.h"
#include "level.h"
#include "seq.h"

//#define DEBUG 1

extern const unsigned char r_text_loading[];
extern const unsigned char r_text_loading2[];
extern const unsigned char r_text_loading3[];

void main_raster_irq(void) {
    sid_play_frame();
}

unsigned char main_irq_handler(void) {
    return consume_raster_irq(&main_raster_irq);
}

#if !DEBUG
unsigned char intro_screen() {
    unsigned char err, joyval;
    unsigned int data_size;
    unsigned char* data;

    screen_init(true);

    puts(r_text_loading);

    if(err = sid_load("intro.sid")) {
        printf("sid load error: %x\n", err);
        return EXIT_FAILURE;
    }

    puts(r_text_loading2);

    puts(r_text_loading3);

    if(err = ocp_load("intro.ocx")) {
        printf("bitmap load error: %x\n", err);
        return EXIT_FAILURE;
    }

    setup_irq_handler(&main_irq_handler);

    while(1) {
        joyval = joy_read(0x01);
        if(joyval & JOY_ANY_MASK) {
            break;
        }
    }

    setup_irq_handler(NULL);

    sid_stop();

    return EXIT_SUCCESS;
}
#endif

unsigned char main (void) {
    unsigned char err;

    screen_init(true);

    pal_system(); // Reset the system's PAL flag as it could be garbage in various cases.

    srand(clock());

    joy_install(&joy_static_stddrv);

    puts(r_text_loading2);

    character_init(true);

    puts(r_text_loading3);

#if !DEBUG
    if(err = intro_screen()) {
        screen_init(false);
        while(1);
    }
#endif

    if(err = spritesheet_load("canada.spd")) {
        printf("sprite load error: %x\n", err);
        while(1);
    }

    if(err = play_level()) {
        printf("level error: %x\n", err);
        screen_init(false);
        while(1);
    }

    screen_init(true);

    return EXIT_SUCCESS;
}
