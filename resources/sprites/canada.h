typedef struct sprite_sequence sprite_sequence;
struct sprite_sequence {
    unsigned char start_index;
    unsigned char length;
    unsigned char frame_duration; // In jiffies
};

typedef struct char_sprite_group char_sprite_group;
struct char_sprite_group {
    sprite_sequence* neutral;
    sprite_sequence* walk_right;
    sprite_sequence* walk_left;
    sprite_sequence* attack_right;
    sprite_sequence* attack_left;
};

sprite_sequence _sprites_guy[] = {
    {0x00, 1, 1}, // neutral
    {0x01, 3, 5}, // walk_right
    {0x04, 3, 5}, // walk_left
    {0x0A, 3, 10}, // attack_right FIXME sprites swapped in file
    {0x07, 3, 10}, // attack_left
};

char_sprite_group _sprites_group_guy = {
    &(_sprites_guy[0]),
    &(_sprites_guy[1]),
    &(_sprites_guy[2]),
    &(_sprites_guy[3]),
    &(_sprites_guy[4]),
};

sprite_sequence _sprites_moose[] = {
    {0x0d, 1, 1} // neutral
};

char_sprite_group _sprites_group_moose = {
    &(_sprites_moose[0]),
};

// Index is CHAR_TYPE enum. See level.h
char_sprite_group *SPRITES[] = {
    &_sprites_group_guy,
    &_sprites_group_moose,
};