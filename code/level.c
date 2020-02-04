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
#include "char_state.h"

#define MAX_SCREEN_CHARACTERS 16
#define MAX_SCREENS 16

typedef struct level_screen level_screen;
struct level_screen {
    char_state* characters[MAX_SCREEN_CHARACTERS];
};

typedef struct level_state level_state;
struct level_state {
    unsigned char* snz; // Pointer to the loaded sounds
    unsigned char screen_index; // Which screen are we looking at.
    char_state* guy; // State for our guy gets put here so it's easier
                    // To find him, since we deal with him a lot.
    level_screen* screens[MAX_SCREENS];
};

level_state* state;
unsigned int now; 

unsigned char process_input(void) {
    unsigned char joyval = joy_read(0x01);
    char_state* guy = state->guy;
    guy->action_flags &= ~CHAR_ACTION_MOVING;

    if(joyval & CHAR_ACTION_DIRECTION_MASK) {
        guy->action_flags &= ~CHAR_ACTION_DIRECTION_MASK;
        if(joyval & JOY_BTN_1_MASK) {
            guy->action_flags |= CHAR_ACTION_ATTACKING;
        }

        if(joyval & JOY_UP_MASK) {
            guy->action_flags |= CHAR_ACTION_MOVING | CHAR_ACTION_DIRECTION_UP;
        }
        else if(joyval & JOY_DOWN_MASK) {
            guy->action_flags |= CHAR_ACTION_MOVING | CHAR_ACTION_DIRECTION_DOWN;
        }

        if(joyval & JOY_LEFT_MASK) {
            guy->action_flags |= CHAR_ACTION_MOVING | CHAR_ACTION_DIRECTION_LEFT;
        }
        else if(joyval & JOY_RIGHT_MASK) {
            guy->action_flags |= CHAR_ACTION_MOVING | CHAR_ACTION_DIRECTION_RIGHT;
        }
    }

    if(joyval & JOY_BTN_1_MASK) {
        guy->action_flags |= CHAR_ACTION_ATTACKING;
    }

    return EXIT_SUCCESS;
}

unsigned char process_cpu_input(void) {
    unsigned char i;
    char_state* me;
    level_screen* screen = state->screens[state->screen_index];

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(me->char_type == CHAR_TYPE_MOOSE) {
            me->action_flags |= CHAR_ACTION_MOVING;
            if(rand() % 16 == 0) {
                me->action_flags &= ~CHAR_ACTION_DIRECTION_MASK;
                me->action_flags |= rand() % CHAR_ACTION_DIRECTION_MASK;
            }
        }
        else {
            continue;
        }
    }

    return EXIT_SUCCESS;
}

unsigned char update(void) {
    unsigned char i;
    unsigned char j;
    unsigned int action_time, status_time;
    char_state* other;
    char_state* me;
    level_screen* screen = state->screens[state->screen_index];

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(!me) continue;

        // Reconcile flags and last_flags
        if(me->action_flags != me->last_action_flags) {
            me->action_start = now;

            if(me->action_flags & CHAR_ACTION_ATTACKING && !(me->last_action_flags & CHAR_ACTION_ATTACKING)) {
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

        if((me->status_flags & CHAR_STATUS_INVINCIBLE) && status_time > 90) {
            me->status_flags &= ~CHAR_STATUS_INVINCIBLE;
        }

        if((me->action_flags & CHAR_ACTION_DYING) && action_time > 60) {
            sprite_hide(me->sprite_slot);
            screen->characters[i] = NULL;
            free(me);
        }

        if(me->action_flags & CHAR_ACTION_ATTACKING) {
            if(action_time > 10 && action_time < 20) {
                for(j = 0; j < MAX_SCREEN_CHARACTERS; j++) {
                    other = screen->characters[j];
                    if(!other || other == me) {
                        continue;
                    }

                    if(
                            !(other->action_flags & CHAR_ACTION_DYING) && !(other->status_flags & CHAR_STATUS_INVINCIBLE)
                            && other->path_x - 20 < me->path_x && me->path_x < other->path_x + 20
                            && other->path_y - 20 < me->path_y && me->path_y < other->path_y + 20
                            ) {
                        other->hitpoints--;
                        other->status_flags |= CHAR_STATUS_INVINCIBLE;

                        if(!other->hitpoints) {
                            other->action_flags &= ~CHAR_ACTION_MASK;
                            other->action_flags |= CHAR_ACTION_DYING;
                        }
                    }
                }
            }
            else if(action_time > 30) {
                me->action_flags &= ~CHAR_ACTION_ATTACKING;
            }
        }

        if(me->action_flags & CHAR_ACTION_MOVING) {
            if(me->action_flags & CHAR_ACTION_DIRECTION_RIGHT) {
                me->path_x += me->movement_speed;
            }
            else if(me->action_flags & CHAR_ACTION_DIRECTION_LEFT) {
                if(me->path_x >= me->movement_speed) {
                    me->path_x -= me->movement_speed;
                }
                else {
                    me->path_x = 0;
                }
            }
            else if(me->action_flags & CHAR_ACTION_DIRECTION_UP) {
                if(me->path_y >= me->movement_speed) {
                    me->path_y -= me->movement_speed;
                }
                else {
                    me->path_y = 0;
                }
            }
            else if(me->action_flags & CHAR_ACTION_DIRECTION_DOWN) {
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
    unsigned char i, sheet_idx, char_type, action_flags, kbchar;
    bool animate = true;
    sprite_sequence* selected_sprite;
    char_sprite_group* selected_char;
    char_state* me;
    unsigned int action_time;

    level_screen* screen = state->screens[state->screen_index];

    // BEGIN DEBUG STUFF
    gotoxy(0,0);

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

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(!me) continue;

        selected_char = SPRITES[me->char_type];

        if(now % 5 == 0) {
            printf("X: %x Y: %x HP: %x A: %x S: %x C: %x\n", me->path_x, me->path_y, me->hitpoints, me->action_flags, me->sprite_slot, me->char_type);
        }

        action_flags = me->action_flags;

        action_time = now - me->action_start;

        if(action_flags & CHAR_ACTION_DIRECTION_RIGHT) {
            if(action_flags & CHAR_ACTION_ATTACKING) {
                selected_sprite = selected_char->attack_right;
            }
            else if(action_flags & CHAR_ACTION_MOVING) {
                selected_sprite = selected_char->walk_right;
            }
            else {
                selected_sprite = selected_char->walk_right;
                animate = false;
            }
        }
        else if(action_flags & CHAR_ACTION_DIRECTION_LEFT) {
            if(action_flags & CHAR_ACTION_ATTACKING) {
                selected_sprite = selected_char->attack_left;
            }
            else if(action_flags & CHAR_ACTION_MOVING) {
                selected_sprite = selected_char->walk_left;
            }
            else {
                selected_sprite = selected_char->walk_left;
                animate = false;
            }
        }
        else {
            selected_sprite = selected_char->neutral;
        }

        // In case we don't have a sprite defined for this
        // action, default to the neutral position.
        if(!selected_sprite) {
            selected_sprite = selected_char->neutral;
        }

        if(!animate) {
            sheet_idx = selected_sprite->start_index;
        }
        else {
            sheet_idx = spritesheet_animation_next(action_time, selected_sprite->frame_duration, selected_sprite->start_index, selected_sprite->length);
        }

        spritesheet_set_image(me->sprite_slot, sheet_idx);
        //sprite_move(me->sprite_slot, me->path_x, me->path_y);
        sprite_move(me->sprite_slot, SCREEN_SPRITE_BORDER_X_START + me->path_x + ((SCREEN_SPRITE_BORDER_HEIGHT / 2 - me->path_y) / 4), (me->path_y / 2) + (SCREEN_SPRITE_BORDER_HEIGHT * 3 / 4));
    }

    return EXIT_SUCCESS;
}

void level_raster_irq(void) {
    now++;

    sid_play_frame();
    update();
}

unsigned char level_irq_handler(void) {
    return consume_raster_irq(&level_raster_irq);
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

    state->guy = char_state_init(CHAR_TYPE_GUY, 0);

    state->screen_index = 0;

    screen->characters[0] = state->guy;
    screen->characters[1] = char_state_init(CHAR_TYPE_MOOSE, 1);

    state->screens[0] = screen;

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(!me) continue;

        me->sprite_slot = i;

        // FIXME These xy calcs are wrong.
        spritesheet_show(me->sprite_slot, me->default_sprite, me->path_x, me->path_y, true, true);
    }

    setup_irq_handler(&level_irq_handler);

    while (true)
    {
        // World update was originally in this loop, but doing it here
        // blew up the complexity of the program too much.
        // Interrupts are simpler and guaranteed, and force the game
        // logic to be simple since no standard library functions can
        // be called.
        process_input();
        process_cpu_input();
        if(err = render()) {
            printf("Error rendering the game state: %x\n", err);
            return EXIT_FAILURE;
        }
    }
}

