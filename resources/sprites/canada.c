#include "canada.h"

sprite_sequence _sprites_guy[] = {
    {0x00, 1, 1}, // neutral

    {0x01, 3, 5}, // walk_right
    {0x04, 3, 5}, // walk_left

    {0x07, 2, 15}, // attack_right
    {0x09, 2, 15}, // attack_left

    {0x0b, 1, 60}, // oof_right
    {0x0c, 1, 60}, // oof_left
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
    {0x0b, 1, 1} // neutral // FIXME
};

char_sprite_group _sprites_group_moose = {
    &(_sprites_moose[0]),
};


char_sprite_group *SPRITES[] = {
    &_sprites_group_guy,
    &_sprites_group_moose,
};