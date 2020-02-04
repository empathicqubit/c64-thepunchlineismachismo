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

    sprite_sequence* oof_right;
    sprite_sequence* oof_left;
};

// Index is CHAR_TYPE enum. See level.h
extern char_sprite_group *SPRITES[];