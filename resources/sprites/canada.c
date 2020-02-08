#include "canada.h"
#include "../../code/utils.h"

sprite_sequence _sprites_guy[] = {
    {0x00, 1, 1}, // neutral

    {0x01, 3, FRAMES_PER_SEC/10}, // walk_right
    {0x04, 3, FRAMES_PER_SEC/10}, // walk_left

    {0x07, 2, FRAMES_PER_SEC/4}, // attack_right
    {0x09, 2, FRAMES_PER_SEC/4}, // attack_left

    {0x0b, 1, FRAMES_PER_SEC}, // oof_right
    {0x0c, 1, FRAMES_PER_SEC}, // oof_left
};

char_sprite_group _sprites_group_guy = {
    &(_sprites_guy[0]),

    &(_sprites_guy[1]),
    &(_sprites_guy[2]),

    &(_sprites_guy[3]),
    &(_sprites_guy[4]),

    &(_sprites_guy[5]),
    &(_sprites_guy[6]),
};

sprite_sequence _sprites_moose[] = {
    {0x0d, 1, 1}, // neutral

    {0x0e, 1, 1}, // walk_right
    {0x0f, 1, 1}, // walk_left

    {0x10, 1, 1}, // attack_right
    {0x11, 1, 1}, // attack_left

    {0x12, 1, FRAMES_PER_SEC/4}, // oof_right
    {0x13, 1, FRAMES_PER_SEC/4}, // oof_left
};

char_sprite_group _sprites_group_moose = {
    &(_sprites_moose[0]),

    &(_sprites_moose[1]),
    &(_sprites_moose[2]),

    &(_sprites_moose[3]),
    &(_sprites_moose[4]),

    &(_sprites_moose[5]),
    &(_sprites_moose[6]),
};


char_sprite_group *SPRITES[] = {
    &_sprites_group_guy,
    &_sprites_group_moose,
};