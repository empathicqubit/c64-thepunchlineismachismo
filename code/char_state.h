#include <joystick.h>
#include "c64.h"
#include "../resources/sprites/canada.h"
enum {
    CHAR_TYPE_GUY = 0,
    CHAR_TYPE_MOOSE = 1,
};
typedef unsigned char char_type;

enum {
    // 0 is up, 1 is down
    CHAR_ACTION_DIRECTION_UPDOWN = 0x01,
    // 0 is right, 1 is left
    CHAR_ACTION_DIRECTION_RIGHTLEFT = 0x02,
    CHAR_ACTION_DIRECTION_MASK = 0x03,

    // Are we moving along the x axis?
    CHAR_ACTION_MOVING_RIGHTLEFT = 0x04,
    // Are we moving along the y axis?
    CHAR_ACTION_MOVING_UPDOWN = 0x08,
    CHAR_ACTION_MOVING_MASK = 0x0C,

    CHAR_ACTION_ATTACKING = JOY_BTN_1_MASK,    // 0x10

    CHAR_ACTION_DYING = 0x40,

    CHAR_ACTION_MASK = CHAR_ACTION_MOVING_MASK | CHAR_ACTION_ATTACKING | CHAR_ACTION_DYING, // directions and attack
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

    /** X position within the path. The path width is the same as the screen,
      * but because the path is at an angle points further away from the screen
      * get moved farther to the right.
      */
    unsigned int path_x;
    /** Y position within the path. The path height is the same as the screen,
      * but gets divided by half and pushed to the bottom half when being rendered
      */
    unsigned char path_y;
    unsigned char movement_speed; // pixels per jiffy

    sprite_sequence *default_sprite; // The default sprite in the sheet.

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
*/
char_state* char_state_init(char_type c);