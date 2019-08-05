#include <stdio.h>
#include <stdlib.h>
#include <tgi.h>
#include <joystick.h>
#include <conio.h>
#include "koala.h"

extern const char text[];

extern void sid_init (void);

extern void sid_play (void);

void sid_loop (void) {
    while(1) sid_play();
}

int main (void) {
    FILE* fp;

    printf("%s\n", text);

    fp = fopen("intro.sid", "rb");

    fread(SID_START, 256, SID_SIZE / 256 + 1, fp);
    fclose(fp);

    koala_load("intro.koa");

    sid_init();
    sid_loop();

    return EXIT_SUCCESS;
}
