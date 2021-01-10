#include "canada.h"
#include "../../code/utils.h"
#include <stddef.h>

extern sprite_sequence sprites_guy[];

char_sprite_group _sprites_group_guy = {
    &(sprites_guy[0]),

    &(sprites_guy[1]),
    &(sprites_guy[2]),

    &(sprites_guy[3]),
    &(sprites_guy[4]),

    &(sprites_guy[5]),
    &(sprites_guy[6]),
};

extern sprite_sequence sprites_moose[];

char_sprite_group _sprites_group_moose = {
    &(sprites_moose[0]),

    &(sprites_moose[1]),
    &(sprites_moose[2]),

    &(sprites_moose[3]),
    &(sprites_moose[4]),

    &(sprites_moose[5]),
    &(sprites_moose[6]),
};


char_sprite_group *SPRITES[] = {
    &_sprites_group_guy,
    &_sprites_group_moose,
};