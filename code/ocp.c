#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include "exo.h"
#include "c64.h"
#include "utils.h"

unsigned char ocp_load (char filename[]) {
    unsigned int seek, size;
    unsigned char* color_temp;
    unsigned char err, len;

    len = strlen(filename);
    if(len < 4) {
        return EXIT_FAILURE;
    }

    memcpy(&(filename[len - 4]), ".ocb", 4);

    if(err = exo_open(filename, &size)) {
        return EXIT_FAILURE;
    }

    if(err = exo_unpack(BITMAP_START)) {
        exo_close();
        return err;
    }

    exo_close();

    memcpy(&(filename[len - 4]), ".ocs", 4);

    if(err = exo_open(filename, &size)) {
        return EXIT_FAILURE;
    }

    if(err = exo_unpack(SCREEN_START)) {
        exo_close();
        return err;
    }

    exo_close();

    if(!(color_temp = malloc(COLOR_RAM_SIZE))) {
        return EXIT_FAILURE;
    }

    *(unsigned char *)VIC_CTRL1 |= VIC_CTRL1_BITMAP_ON;
    *(unsigned char *)VIC_CTRL2 |= VIC_CTRL2_MULTICOLOR_ON;

    *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_CHAR_PTR_MASK);
    *(unsigned char *)VIC_VIDEO_ADR |= ((BITMAP_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_CHAR_DIVISOR) << 1;

    seek = SCREEN_BYTES;
    *(unsigned char *)VIC_BORDERCOLOR = ((unsigned char*)SCREEN_START)[seek];
    seek += 1;
    *(unsigned char *)VIC_BG_COLOR0 = ((unsigned char*)SCREEN_START)[seek];
    seek += 15;

    memcpy(color_temp, SCREEN_START + seek, COLOR_RAM_SIZE);

    memcpy(COLOR_RAM, color_temp, COLOR_RAM_SIZE);

    free(color_temp);

    return EXIT_SUCCESS;
}
