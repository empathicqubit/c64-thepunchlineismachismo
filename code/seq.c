#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "exo.h"
#include "c64.h"

/** Load a SEQ file, a text file in PETSCII format.
 * Bear in mind this is different from screen codes!
 * This will also wipe out the screen since the files
 * use it as a temporary storage area.
 * @param filename The filename of the RLE packed SEQ file
 * @param size The size of the returned data
 * @return If we were sucessful.
 */
unsigned char seq_load (unsigned char* filename, unsigned int* size) {
    unsigned char err;
    unsigned int end;

    if(err = exo_open(filename, &end)) {
        return EXIT_FAILURE;
    }

    *size = (end - SCREEN_START) + 1;

    if(err = exo_unpack()) {
        exo_close();
        return EXIT_FAILURE;
    }

    // make sure first byte is clear screen
    if (*(unsigned char*)SCREEN_START != 0x93) {
        exo_close();
        return EXIT_FAILURE;
    }

    exo_close();

    return EXIT_SUCCESS;
}