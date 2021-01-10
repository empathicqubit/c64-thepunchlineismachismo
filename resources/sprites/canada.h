#ifndef __CANADA_H__
#define __CANADA_H__

typedef struct sprite_grid sprite_grid;
struct sprite_grid {
    unsigned char width;
    unsigned char height;
    unsigned char indices[8];
};

typedef struct sprite_sequence sprite_sequence;
struct sprite_sequence {
    unsigned char length;
    // Time for each frame in jiffies
    unsigned char frame_duration;
    sprite_grid frames[8];
};

typedef struct char_sprite_group char_sprite_group;
struct char_sprite_group {
    sprite_sequence* neutral;

    sprite_sequence* walk_right;
    sprite_sequence* walk_left;

    sprite_sequence* attack_right;
    sprite_sequence* attack_left;

    sprite_sequence* oof_right;
    sprite_sequence* oof_left;
};

// Index is CHAR_TYPE enum. See level.h
extern char_sprite_group *SPRITES[];
#endif