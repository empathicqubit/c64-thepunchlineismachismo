#include <joystick.h>
#include "c64.h"
enum {
    CHAR_TYPE_GUY = 0,
    CHAR_TYPE_MOOSE = 1,
};
typedef unsigned char char_type;

enum {

    CHAR_ACTION_DIRECTION_UP = JOY_UP_MASK,       // 0x01
    CHAR_ACTION_DIRECTION_DOWN = JOY_DOWN_MASK,   // 0x02
    CHAR_ACTION_DIRECTION_LEFT = JOY_LEFT_MASK,   // 0x04
    CHAR_ACTION_DIRECTION_RIGHT = JOY_RIGHT_MASK, // 0x08

    CHAR_ACTION_DIRECTION_MASK = 0x0F,

    CHAR_ACTION_ATTACKING = JOY_BTN_1_MASK,    // 0x10

    CHAR_ACTION_MOVING = 0x20,

    CHAR_ACTION_DYING = 0x40,

    CHAR_ACTION_MASK = CHAR_ACTION_MOVING | CHAR_ACTION_ATTACKING | CHAR_ACTION_DYING, // directions and attack
};
typedef unsigned char char_action_flag;

enum {
    CHAR_STATUS_INVINCIBLE = 0x01, // Can't be hit
    CHAR_STATUS_TOXIC = 0x02, // Can deal damage by touch alone
    CHAR_STATUS_INCAPACITATED = 0x04, // Dazed or stunned
};
typedef unsigned char char_status_flag;

typedef struct char_state char_state;
struct char_state {
    char_type char_type; // The type of the character

    unsigned int path_x; // X position within the path
    unsigned char path_y; // Y position within the path
    unsigned char movement_speed; // pixels per jiffy
    unsigned char sprite_slot; // The sprite number if one already exists,

    unsigned char default_sprite; // The default sprite in the sheet.

    unsigned char hitpoints; // How many hits until the character dies
    unsigned char attackpoints; // How many hits a character deals. Most of the time this is 1.
    unsigned int action_start; // Jiffy time the action started.
    unsigned int status_start; // Jiffy time the status started.

    char_action_flag action_flags; // Flags now
    char_action_flag last_action_flags; // Flags at the last tick

    char_status_flag status_flags; // Flags now
    char_status_flag last_status_flags; // Flags at the last tick
};

/*
 * Initializes a character with the default options
 * @param c - The character type value enum. All of these start with CHAR_TYPE_
 * @param sprite_slot - The C64 sprite slot to assign to the character.
*/
char_state* char_state_init(char_type c, unsigned char sprite_slot);