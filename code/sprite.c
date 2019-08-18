#include <stdlib.h>
#include <stdbool.h>
#include "c64.h"

// Start a character page after the VIC hole, to allow room for another
// character set.
unsigned char* _sprite_addr = SCREEN_START + 0x400;

unsigned char* sprite_next_addr() {
    unsigned char* current = _sprite_addr;

    _sprite_addr += VIC_SPR_SIZE; // bytes per sprite

    if(_sprite_addr >= (unsigned char *)(SCREEN_START + 0x1000)) {
        _sprite_addr = SCREEN_START + 0x400;
    }

    return current;
}

void sprite_move(unsigned char idx, unsigned int x, unsigned char y) {
    char hi_mask = (1<<idx);

    unsigned char* sprite_x = VIC_SPR0_X + idx * 2;
    unsigned char* sprite_y = VIC_SPR0_Y + idx * 2;

    char* hi = VIC_SPR_HI_X;
    if(x>>8) {
        *hi |= hi_mask;
    }
    else {
        *hi &= ~hi_mask;
    }

    *sprite_x = (unsigned char)x;
    *sprite_y = y;
}

unsigned char sprite_load(unsigned char* sprite_data, unsigned char idx, unsigned int x, unsigned char y, bool mcolor, unsigned char sprite_color) {
    char hi_mask = (1<<idx);
    unsigned char* spr_pointers;

    if(idx > 7) return EXIT_FAILURE;
    if((unsigned int)sprite_data % VIC_SPR_SIZE) return EXIT_FAILURE;

    sprite_move(idx, x, y);

    spr_pointers = SCREEN_START + 0x03F8;

    spr_pointers[idx] = (unsigned char)(((unsigned int)sprite_data / VIC_SPR_SIZE));

    if(mcolor) {
        *(char *)VIC_SPR_MCOLOR |= hi_mask;
    }
    else {
        *(char *)VIC_SPR_MCOLOR &= ~hi_mask;
    }

    ((char *)VIC_SPR0_COLOR)[idx] = sprite_color;

    *(char *)VIC_SPR_ENA |= hi_mask;

    return EXIT_SUCCESS;
}
