#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include "utils.h"

FILE* _crunched_fp = NULL;
extern void __fastcall__ decrunch(void);

unsigned char* exo_loadaddroffs = 0xffff;

unsigned char __fastcall__ get_crunched_byte(void) {
    bordercolor((bordercolor(0) + 1) % 16);
    return fgetc(_crunched_fp);
}

unsigned char exo_unpack(unsigned char* dest) {
    exo_loadaddroffs = dest;
    hide_io();
    decrunch();
    show_io();

    return EXIT_SUCCESS;
}

void exo_close() {
    fclose(_crunched_fp);
    _crunched_fp = NULL;
    exo_loadaddroffs = 0xffff;
}

unsigned char exo_open(unsigned char* filename, unsigned int* unpacked_end) {
    if(!(_crunched_fp = fopen(filename, "rb"))) {
        return EXIT_FAILURE;
    }

    if(!fread(&((unsigned char*)unpacked_end)[1], 1, 1, _crunched_fp)
        || !fread(unpacked_end, 1, 1, _crunched_fp)) {
        return EXIT_FAILURE;
    }

    fclose(_crunched_fp);

    if(!(_crunched_fp = fopen(filename, "rb"))) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}