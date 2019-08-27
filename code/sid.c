#include <stdio.h>
#include <stdlib.h>
#include "c64.h"

extern void sid_init (void);

extern void sid_play (void);

unsigned char sid_load (char filename[]) {
    FILE* fp;

    fp = fopen(filename, "rb");

    if(!fread(SID_START, 256, SID_SIZE / 256 + 1, fp)) {
        return EXIT_FAILURE;
    }

    fclose(fp);

    sid_init();

    return EXIT_SUCCESS;
}

unsigned char sid_stop(void) {
    *(unsigned char *)SID_Ctl1 = 0x00;
    *(unsigned char *)SID_Ctl2 = 0x00;
    *(unsigned char *)SID_Ctl3 = 0x00;

    return EXIT_SUCCESS;
}

void sid_play_frame (void) {
    sid_play();
}

