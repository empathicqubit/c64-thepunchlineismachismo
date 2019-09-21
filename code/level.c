#include <joystick.h>
#include <stdio.h>
#include <stdlib.h>
#include <6502.h>
#include <conio.h>
#include <time.h>
#include <stdbool.h>
#include "utils.h"
#include "c64.h"
#include "sprite.h"
#include "sid.h"
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

    CHAR_FLAG_DYING = 0x40,

    CHAR_FLAG_ACTION = CHAR_FLAG_MOVING | CHAR_FLAG_ATTACKING | CHAR_FLAG_DYING, // directions and attack
};
typedef unsigned char char_action_flag;

enum {
    CHAR_FLAG_INVINCIBLE = 0x01, // Can't be hit
    CHAR_FLAG_TOXIC = 0x02, // Can deal damage by touch alone
    CHAR_FLAG_INCAPACITATED = 0x04, // Dazed or stunned
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

typedef struct level_screen level_screen;
struct level_screen {
    char_state* characters[16];
};

typedef struct level_state level_state;
struct level_state {
    unsigned char* snz; // Pointer to the loaded sounds
    unsigned char screen_index; // Which screen are we looking at.
    char_state* guy; // State for our guy gets put here so it's easier
                    // To find him, since we deal with him a lot.
    level_screen* screens[16];
};

level_state* state;
unsigned int now; 

char_state* char_state_init(char_type c) {
    char_state* state = calloc(1, sizeof(char_state));
    if(c == CHAR_TYPE_GUY) {
        state->sprite_slot = 0;
        state->path_x = 25;
        state->path_y = 25;
        state->movement_speed = 2;
        state->default_sprite = SPRITES_GUY_FRONT;
    }
    else if(c == CHAR_TYPE_MOOSE) {
        state->default_sprite = SPRITES_MOOSE;
        state->movement_speed = 2;
        state->hitpoints = 5;
        state->sprite_slot = 1;
        state->path_x = 160;
        state->path_y = 25;
    }

    return state;
}

unsigned char process_input() {
    unsigned char joyval = joy_read(0x01);
    char_state* guy = state->guy;
    guy->action_flags &= ~CHAR_FLAG_MOVING;

    if(joyval & CHAR_FLAG_DIRECTION) {
        guy->action_flags &= ~CHAR_FLAG_DIRECTION;
        if(joyval & JOY_BTN_1_MASK) {
            guy->action_flags |= CHAR_FLAG_ATTACKING;
        }

        if(joyval & JOY_UP_MASK) {
            guy->action_flags |= CHAR_FLAG_MOVING | CHAR_FLAG_DIRECTION_UP;
        }
        else if(joyval & JOY_DOWN_MASK) {
            guy->action_flags |= CHAR_FLAG_MOVING | CHAR_FLAG_DIRECTION_DOWN;
        }

        if(joyval & JOY_LEFT_MASK) {
            guy->action_flags |= CHAR_FLAG_MOVING | CHAR_FLAG_DIRECTION_LEFT;
        }
        else if(joyval & JOY_RIGHT_MASK) {
            guy->action_flags |= CHAR_FLAG_MOVING | CHAR_FLAG_DIRECTION_RIGHT;
        }
    }

    if(joyval & JOY_BTN_1_MASK) {
        guy->action_flags |= CHAR_FLAG_ATTACKING;
    }

    return EXIT_SUCCESS;
}

unsigned char update(void) {
    unsigned char i;
    unsigned char j;
    unsigned int action_time, status_time;
    char_state* other;
    level_screen* screen = state->screens[state->screen_index];

    for(i = 0; i < 16; i++) {
        char_state* me = screen->characters[i];

        if(!me) continue;

        // Reconcile flags and last_flags
        if(me->action_flags != me->last_action_flags) {
            me->action_start = now;

            if(me->action_flags & CHAR_FLAG_ATTACKING && !(me->last_action_flags & CHAR_FLAG_ATTACKING)) {
                sid_play_sound(state->snz, 0, 2);
            }
        }

        me->last_action_flags = me->action_flags;

        if(me->status_flags != me->last_status_flags) {
            me->status_start = now;
        }

        me->last_status_flags = me->status_flags;

        action_time = now - me->action_start;
        status_time = now - me->status_start;

        if((me->status_flags & CHAR_FLAG_INVINCIBLE) && status_time > 90) {
            me->status_flags &= ~CHAR_FLAG_INVINCIBLE;
        }

        if((me->action_flags & CHAR_FLAG_DYING) && action_time > 60) {
            sprite_hide(me->sprite_slot);
            screen->characters[i] = NULL;
            free(me);
        }

        if(me->action_flags & CHAR_FLAG_ATTACKING) {
            if(action_time > 10 && action_time < 20) {
                for(j = 0; j < 16; j++) {
                    other = screen->characters[j];
                    if(!other || other == me) {
                        continue;
                    }

                    if(
                            !(other->action_flags & CHAR_FLAG_DYING) && !(other->status_flags & CHAR_FLAG_INVINCIBLE)
                            && other->path_x - 20 < me->path_x && me->path_x < other->path_x + 20
                            && other->path_y - 20 < me->path_y && me->path_y < other->path_y + 20
                            ) {
                        other->hitpoints--;
                        other->status_flags |= CHAR_FLAG_INVINCIBLE;

                        if(!other->hitpoints) {
                            other->action_flags &= ~CHAR_FLAG_ACTION;
                            other->action_flags |= CHAR_FLAG_DYING;
                        }
                    }
                }
            }
            else if(action_time > 30) {
                me->action_flags &= ~CHAR_FLAG_ATTACKING;
            }
        }

        if(me->action_flags & CHAR_FLAG_MOVING) {
            if(me->action_flags & CHAR_FLAG_DIRECTION_RIGHT) {
                me->path_x += me->movement_speed;
            }
            else if(me->action_flags & CHAR_FLAG_DIRECTION_LEFT) {
                if(me->path_x >= me->movement_speed) {
                    me->path_x -= me->movement_speed;
                }
                else {
                    me->path_x = 0;
                }
            }
            else if(me->action_flags & CHAR_FLAG_DIRECTION_UP) {
                if(me->path_y >= me->movement_speed) {
                    me->path_y -= me->movement_speed;
                }
                else {
                    me->path_y = 0;
                }
            }
            else if(me->action_flags & CHAR_FLAG_DIRECTION_DOWN) {
                me->path_y += me->movement_speed;
            }

            if(me->path_y > SCREEN_SPRITE_BORDER_HEIGHT / 2) {
                me->path_y = SCREEN_SPRITE_BORDER_HEIGHT / 2;
            }

            if(me->path_x > SCREEN_SPRITE_BORDER_WIDTH) {
                me->path_x = SCREEN_SPRITE_BORDER_WIDTH;
            }
        }
    }

    return EXIT_SUCCESS;
}

unsigned char render() {
    unsigned char i, sheet_idx, action_flags, kbchar;
    char_state* me;
    unsigned int action_time;

    level_screen* screen = state->screens[state->screen_index];

    // BEGIN DEBUG STUFF
    gotoxy(0,20);

    kbchar = 0x00;
    if(kbhit()) {
        kbchar = cgetc();
    }

    if(kbchar) {
        if(kbchar == 'p') {
            sid_play_sound(state->snz, rand() % 2, 2);
        }
    }
    // END DEBUG STUFF

    for(i = 0; i < 16; i++) {
        me = screen->characters[i];

        if(!me) continue;

        if(me != state->guy) {
            printf("X: %x Y: %x HP: %x\n", me->path_x, me->path_y, me->hitpoints);
        }

        sheet_idx = me->default_sprite;

        action_flags = me->action_flags;

        action_time = now - me->action_start;

        if(action_flags & CHAR_FLAG_DIRECTION_RIGHT) {
            if(action_flags & CHAR_FLAG_ATTACKING) {
                sheet_idx = spritesheet_animation_next(action_time, 10, SPRITES_GUY_ATTACK_RIGHT_0, SPRITES_GUY_ATTACK_RIGHT_END);
            }
            else if(action_flags & CHAR_FLAG_MOVING) {
                sheet_idx = spritesheet_animation_next(action_time, 5, SPRITES_GUY_WALK_RIGHT_0, SPRITES_GUY_WALK_RIGHT_END);
            }
            else {
                sheet_idx = SPRITES_GUY_WALK_RIGHT_0;
            }
        }
        else if(action_flags & CHAR_FLAG_DIRECTION_LEFT) {
            if(action_flags & CHAR_FLAG_ATTACKING) {
                sheet_idx = spritesheet_animation_next(action_time, 10, SPRITES_GUY_ATTACK_LEFT_0, SPRITES_GUY_ATTACK_LEFT_END);
            }
            else if(action_flags & CHAR_FLAG_MOVING) {
                sheet_idx = spritesheet_animation_next(action_time, 5, SPRITES_GUY_WALK_LEFT_0, SPRITES_GUY_WALK_LEFT_END);
            }
            else {
                sheet_idx = SPRITES_GUY_WALK_LEFT_0;
            }
        }

        spritesheet_set_image(me->sprite_slot, sheet_idx);
        //sprite_move(me->sprite_slot, me->path_x, me->path_y);
        sprite_move(me->sprite_slot, SCREEN_SPRITE_BORDER_X_START + me->path_x + ((SCREEN_SPRITE_BORDER_HEIGHT / 2 - me->path_y) / 4), (me->path_y / 2) + (SCREEN_SPRITE_BORDER_HEIGHT * 3 / 4));
    }

    return EXIT_SUCCESS;
}

unsigned char my_irq_handler(void) {
    unsigned char err;
    unsigned char i;

    if(*(unsigned char *)VIC_IRR & VIC_IRQ_RASTER) {
        *(unsigned char*)VIC_IRR |= VIC_IRQ_RASTER;

        now++;

        sid_play_frame();
        update();

        return IRQ_HANDLED;
    }

    return IRQ_NOT_HANDLED;
}

unsigned char play_level (void) {
    char_state* me;
    unsigned char err, i;

    level_screen* screen = calloc(1, sizeof(level_screen));

    state = calloc(1, sizeof(level_state));

    if(!(state->snz = snz_load("canada.snz", &err))) {
        printf("Sound load error: %x\n", err);
        return EXIT_FAILURE;
    }

    if(err = sid_load("empty.sid")) {
        printf("SID load error: %x\n", err);
        return EXIT_FAILURE;
    }

    screen_init(false);
    clrscr();

    state->guy = char_state_init(CHAR_TYPE_GUY);

    state->screen_index = 0;

    screen->characters[0] = state->guy;
    screen->characters[1] = char_state_init(CHAR_TYPE_MOOSE);

    state->screens[0] = screen;

    for(i = 0; i < 16; i++) {
        me = screen->characters[i];

        if(!me) continue;

        me->sprite_slot = i;

        // FIXME These xy calcs are wrong.
        spritesheet_show(me->sprite_slot, me->default_sprite, me->path_x, me->path_y, true, true);
    }

    setup_irq_handler(&my_irq_handler);

    while (true)
    {
        // World update was originally in this loop, but doing it here
        // blew up the complexity of the program too much.
        // Interrupts are simpler and guaranteed.
        process_input();
        if(err = render()) {
            printf("Error rendering the game state: %x\n", err);
            return EXIT_FAILURE;
        }
    }

}

