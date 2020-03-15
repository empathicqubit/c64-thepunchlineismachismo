#include <stdio.h>
#include <string.h>
#include <conio.h>
#include <stdlib.h>
#include "exo.h"
#include "c64.h"

unsigned char ocp_load (char filename[]) {
    unsigned int seek, unpacked_end;
    unsigned char err;

    if(err = exo_open(filename, &unpacked_end)) {
        return EXIT_FAILURE;
    }

    // FIXME Switch off/on I/O bank. See CHAR ROM code in utils
    if(err = exo_unpack()) {
        exo_close();
        return err;
    }

    *(unsigned char *)VIC_CTRL1 |= VIC_CTRL1_BITMAP_ON;
    *(unsigned char *)VIC_CTRL2 |= VIC_CTRL2_MULTICOLOR_ON;

    *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_CHAR_PTR_MASK);
    *(unsigned char *)VIC_VIDEO_ADR |= ((BITMAP_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_CHAR_DIVISOR) << 1;

    // FIXME Copy Color RAM to correct place after write
    /*
    seek = 0;
    memcpy(BITMAP_START, data + seek, SCREEN_BITMAP_SIZE);
    seek += SCREEN_BITMAP_SIZE;
    memcpy(SCREEN_START, data + seek, SCREEN_BYTES);
    seek += SCREEN_BYTES;
    memcpy(VIC_BORDERCOLOR, data + seek, 1);
    seek += 1 + 14; // 14 bytes of trash
    memcpy(COLOR_RAM, data + seek, COLOR_RAM_SIZE);

    free(data);
    */

    return EXIT_SUCCESS;
}
