#include <joystick.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>
#include "utils.h"
#include "c64.h"
#include "sprite.h"
#include "../resources/sprites/canada.h"

enum {
    CHAR_TYPE_GUY,
    CHAR_TYPE_MOOSE,
};
typedef unsigned char char_type;

enum {

    CHAR_FLAG_DIRECTION_UP = JOY_UP_MASK,       // 0x01
    CHAR_FLAG_DIRECTION_DOWN = JOY_DOWN_MASK,   // 0x02
    CHAR_FLAG_DIRECTION_LEFT = JOY_LEFT_MASK,   // 0x04
    CHAR_FLAG_DIRECTION_RIGHT = JOY_RIGHT_MASK, // 0x08

    CHAR_FLAG_DIRECTION = 0x0F,

    CHAR_FLAG_ATTACKING = JOY_BTN_1_MASK,    // 0x10

    CHAR_FLAG_MOVING = 0x20,

    CHAR_FLAG_ACTION = 0x30, // directions and attack

    CHAR_FLAG_INVINCIBLE = 0x40, // Can't be hit
    CHAR_FLAG_TOXIC = 0x80, // Can deal damage by touch alone
    CHAR_FLAG_INCAPACITATED = 0x100, // Dazed or stunned
};
typedef unsigned int char_flag;

typedef struct char_state char_state;
struct char_state {
    char_type char_type; // The type of the character

    unsigned int path_x; // X position within the path
    unsigned char path_y; // Y position within the path
    unsigned char movement_speed; // pixels per jiffy
    unsigned char sprite_no; // The sprite number if one already exists,

    unsigned char default_sprite; // The default sprite in the sheet.

    unsigned char hitpoints; // How many hits until the character dies
    unsigned char attackpoints; // How many hits a character deals. Most of the time this is 1.
    unsigned char action_start; // Jiffy time the action started.

    char_flag flags; // Flags now
    char_flag last_flags; // Flags at the last tick
};

typedef struct level_screen level_screen;
struct level_screen {
    char_state* characters[16];
};

typedef struct level_state level_state;
struct level_state {
    unsigned char screen_index; // Which screen are we looking at.
    char_state* guy; // State for our guy gets put here so it's easier
                    // To find him, since we deal with him a lot.
    level_screen* screens[16];
};

char_state* char_state_init(char_type c) {
    char_state* state = calloc(1, sizeof(char_state));
    if(c == CHAR_TYPE_GUY) {
        state->sprite_no = 0;
        state->path_x = 25;
        state->path_y = 25;
        state->movement_speed = 2;
        state->default_sprite = SPRITES_GUY_FRONT;
    }
    else if(c == CHAR_TYPE_MOOSE) {
        state->default_sprite = SPRITES_MOOSE;
        state->movement_speed = 2;
        state->sprite_no = 1;
        state->path_x = 160;
        state->path_y = 25;
    }

    return state;
}

unsigned char process_input(level_state* state) {
    unsigned char joyval = joy_read(0x01);
    char_state* guy = state->guy;
    guy->flags &= ~CHAR_FLAG_MOVING;

    if(joyval & CHAR_FLAG_DIRECTION) {
        guy->flags &= ~CHAR_FLAG_DIRECTION;
        if(joyval & JOY_UP_MASK) {
            guy->flags |= CHAR_FLAG_MOVING | CHAR_FLAG_DIRECTION_UP;
        }
        else if(joyval & JOY_DOWN_MASK) {
            guy->flags |= CHAR_FLAG_MOVING | CHAR_FLAG_DIRECTION_DOWN;
        }

        if(joyval & JOY_LEFT_MASK) {
            guy->flags |= CHAR_FLAG_MOVING | CHAR_FLAG_DIRECTION_LEFT;
        }
        else if(joyval & JOY_RIGHT_MASK) {
            guy->flags |= CHAR_FLAG_MOVING | CHAR_FLAG_DIRECTION_RIGHT;
        }
    }

    if(joyval & JOY_BTN_1_MASK) {
        guy->flags |= CHAR_FLAG_MOVING | CHAR_FLAG_ATTACKING;
    }

    return EXIT_SUCCESS;
}

unsigned char update(level_state* state, unsigned int now) {
    unsigned char i;
    level_screen* screen = state->screens[state->screen_index];

    for(i = 0; i < 16; i++) {
        char_state* me = screen->characters[i];

        // Reconcile flags and last_flags
        if((me->flags & CHAR_FLAG_ACTION) != (me->last_flags & CHAR_FLAG_ACTION)) {
            me->action_start = now;
        }

        me->last_flags = me->flags;

        if(me->flags & CHAR_FLAG_MOVING) {
            if(me->flags & CHAR_FLAG_DIRECTION_RIGHT) {
                me->path_x += me->movement_speed;
            }
            else if(me->flags & CHAR_FLAG_DIRECTION_LEFT) {
                if(me->path_x >= me->movement_speed) {
                    me->path_x -= me->movement_speed;
                }
                else {
                    me->path_x = 0;
                }
            }
            else if(me->flags & CHAR_FLAG_DIRECTION_UP) {
                if(me->path_y >= me->movement_speed) {
                    me->path_y -= me->movement_speed;
                }
                else {
                    me->path_y = 0;
                }
            }
            else if(me->flags & CHAR_FLAG_DIRECTION_DOWN) {
                me->path_y += me->movement_speed;
            }

            if(me->path_y > SCREEN_BITMAP_HEIGHT / 2) {
                me->path_y = SCREEN_BITMAP_HEIGHT / 2;
            }

            if(me->path_x > SCREEN_BITMAP_WIDTH) {
                me->path_x = SCREEN_BITMAP_WIDTH;
            }
        }
    }

    return EXIT_SUCCESS;
}

unsigned char render(level_state* state) {
    unsigned char i, sheet_idx, flags;
    char_state* me;
    unsigned int action_time;
    unsigned int now = clock();

    level_screen* screen = state->screens[state->screen_index];

    for(i = 0; i < 16; i++) {
        me = screen->characters[i];

        if(!me) break;

        sheet_idx = me->default_sprite;

        flags = me->flags;

        action_time = now - me->action_start;

        if(flags & CHAR_FLAG_DIRECTION_RIGHT) {
            if(flags & CHAR_FLAG_MOVING) {
                sheet_idx = spritesheet_animation_next(action_time, 5, SPRITES_GUY_WALK_RIGHT_0, SPRITES_GUY_WALK_RIGHT_END);
            }
            else {
                sheet_idx = SPRITES_GUY_WALK_RIGHT_0;
            }
        }
        else if(flags & CHAR_FLAG_DIRECTION_LEFT) {
            if(flags & CHAR_FLAG_MOVING) {
                sheet_idx = spritesheet_animation_next(action_time, 5, SPRITES_GUY_WALK_LEFT_0, SPRITES_GUY_WALK_LEFT_END);
            }
            else {
                sheet_idx = SPRITES_GUY_WALK_LEFT_0;
            }
        }

        spritesheet_set_image(me->sprite_no, sheet_idx);
        sprite_move(me->sprite_no, me->path_x + (((SCREEN_BITMAP_HEIGHT / 2) - me->path_y) / 4), (me->path_y / 2) + 150);
    }


    return EXIT_SUCCESS;
}

unsigned char play_level (void) {
    unsigned int now, elapsed;
    char_state* me;
    unsigned char err, i;

    unsigned int previous = clock();
    unsigned int lag = 0;

    level_state* state = calloc(1, sizeof(level_state));

    level_screen* screen = calloc(1, sizeof(level_screen));

    screen_init(false);
    clrscr();

    printf("lame\n");

    state->guy = char_state_init(CHAR_TYPE_GUY);

    state->screen_index = 0;

    screen->characters[0] = state->guy;
    screen->characters[1] = char_state_init(CHAR_TYPE_MOOSE);

    state->screens[0] = screen;

    for(i = 0; i < 16; i++) {
        me = screen->characters[i];

        if(!me) break;

        me->sprite_no = i;

        // FIXME These xy calcs are wrong.
        spritesheet_show(me->sprite_no, me->default_sprite, me->path_x, me->path_y, true, true);
    }

    while (true)
    {
        now = clock();
        elapsed = now - previous;

        previous = now;
        lag += elapsed;

        process_input(state);

        while (lag >= 1)
        {
            if(err = update(state, now)) {
                printf("Error updating the game state: %x\n", err);
                return EXIT_FAILURE;
            }
            lag--;
        }

        render(state);
    }

}

