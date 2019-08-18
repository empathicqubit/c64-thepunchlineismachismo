#include <stdio.h>
#include <stdlib.h>

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

void sid_play_frame (void) {
    sid_play();
}

