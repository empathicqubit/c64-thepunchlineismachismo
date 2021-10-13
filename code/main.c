#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

void main_raster_irq(void) {
    sid_play_frame();
}

unsigned char intro_screen() {
    static unsigned char err, joyval;
    static unsigned int data_size;
    static unsigned char* data;

    screen_init(true);

    puts("charging moose...");

    if(err = sid_load("intro.sid")) {
        printf("sid load error: %x\n", err);
        return EXIT_FAILURE;
    }

    puts("reticulating goggles...");

    if(err = ocp_load("intro.ocx")) {
        printf("bitmap load error: %x\n", err);
        return EXIT_FAILURE;
    }

    setup_irq_handler();

    while(1) {
        joyval = joy_read(0x01);
        if(joyval & JOY_ANY_MASK) {
            break;
        }
    }

    destroy_irq_handler();

    sid_stop();

    return EXIT_SUCCESS;
}

unsigned char main (void) {
    static unsigned char err;

    character_init(true);

    screen_init(true);

    pal_system(); // Reset the system's PAL flag as it could be garbage in various cases.

    srand(clock());

    joy_install(&joy_static_stddrv);

#if !DEBUG
    if(err = intro_screen()) {
        screen_init(false);
        while(1);
    }
#endif

    if(err = play_level()) {
        printf("level error: %x\n", err);
        screen_init(false);
        while(1);
    }

    screen_init(true);

    return EXIT_SUCCESS;
}
