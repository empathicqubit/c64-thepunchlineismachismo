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
    guy->action_flags &= ~CHAR_ACTION_MOVING_MASK;

    if(joyval & JOY_ANY_MASK) {
        if(joyval & JOY_BTN_1_MASK) {
            guy->action_flags |= CHAR_ACTION_ATTACKING;
        }

        if(joyval & JOY_UP_MASK) {
            guy->action_flags &= ~CHAR_ACTION_DIRECTION_UPDOWN;
            guy->action_flags |= CHAR_ACTION_MOVING_UPDOWN;
        }
        else if(joyval & JOY_DOWN_MASK) {
            guy->action_flags |= CHAR_ACTION_DIRECTION_UPDOWN;
            guy->action_flags |= CHAR_ACTION_MOVING_UPDOWN;
        }

        if(joyval & JOY_RIGHT_MASK) {
            guy->action_flags &= ~CHAR_ACTION_DIRECTION_RIGHTLEFT;
            guy->action_flags |= CHAR_ACTION_MOVING_RIGHTLEFT;
        }
        else if(joyval & JOY_LEFT_MASK) {
            guy->action_flags |= CHAR_ACTION_DIRECTION_RIGHTLEFT;
            guy->action_flags |= CHAR_ACTION_MOVING_RIGHTLEFT;
        }
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
            if(rand() % 16 == 0) {
                me->action_flags |= CHAR_ACTION_MOVING_MASK;
                if(rand() % 2 == 0) {
                    me->action_flags &= ~CHAR_ACTION_MOVING_UPDOWN;
                }
                else {
                    me->action_flags &= ~CHAR_ACTION_MOVING_RIGHTLEFT;
                }

                if(rand() % 2 == 0) {
                    me->action_flags |= CHAR_ACTION_DIRECTION_MASK;
                }
                else {
                    me->action_flags &= ~CHAR_ACTION_DIRECTION_MASK;
                }
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
    char_action_flag action_flags;
    char_state* me;
    level_screen* screen = state->screens[state->screen_index];

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(!me) continue;

        action_flags = me->action_flags;

        // Reconcile flags and last_flags
        if(action_flags != me->last_action_flags) {
            me->action_start = now;

            if(action_flags & CHAR_ACTION_ATTACKING && !(me->last_action_flags & CHAR_ACTION_ATTACKING)) {
                sid_play_sound(state->snz, 0, 2);
            }
        }

        me->last_action_flags = action_flags;

        if(me->status_flags != me->last_status_flags) {
            me->status_start = now;
        }

        me->last_status_flags = me->status_flags;

        action_time = now - me->action_start;
        status_time = now - me->status_start;

        if((me->status_flags & CHAR_STATUS_INVINCIBLE) && status_time > 90) {
            me->status_flags &= ~CHAR_STATUS_INVINCIBLE;
        }

        if((action_flags & CHAR_ACTION_DYING) && action_time > 60) {
            sprite_hide(me->sprite_slot);
            screen->characters[i] = NULL;
            free(me);
        }

        // Dying precludes attacking and moving
        if(action_flags & CHAR_ACTION_DYING) {
            continue;
        }

        if(action_flags & CHAR_ACTION_ATTACKING) {
            if(action_time > 10 && action_time < 20) {
                for(j = 0; j < MAX_SCREEN_CHARACTERS; j++) {
                    other = screen->characters[j];
                    if(!other || other == me) {
                        continue;
                    }

                    if(
                        !(other->action_flags & CHAR_ACTION_DYING) && !(other->status_flags & CHAR_STATUS_INVINCIBLE)
                        && (
                            (action_flags & CHAR_ACTION_DIRECTION_RIGHTLEFT)
                            // Attacker facing left
                            && (
                                (me->path_x < other->path_x + 50)
                                && (me->path_x > other->path_x - 10) 
                            )
                            // Attacker facing right
                            || (
                                (me->path_x + 50 > other->path_x) 
                                && (me->path_x + 50 < other->path_x + 60)
                            )
                        )
                        && (me->path_y > other->path_y - 25)
                        && (me->path_y < other->path_y + 25)
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

        if(action_flags & CHAR_ACTION_MOVING_MASK) {
            if(action_flags & CHAR_ACTION_MOVING_RIGHTLEFT) {
                if(action_flags & CHAR_ACTION_DIRECTION_RIGHTLEFT) {
                    if(me->path_x >= me->movement_speed) {
                        me->path_x -= me->movement_speed;
                    }
                    else {
                        me->path_x = 0;
                    }
                }
                else {
                    me->path_x += me->movement_speed;
                }
            }
            else {
                if(action_flags & CHAR_ACTION_DIRECTION_UPDOWN) {
                    me->path_y += me->movement_speed;
                }
                else {
                    if(me->path_y >= me->movement_speed) {
                        me->path_y -= me->movement_speed;
                    }
                    else {
                        me->path_y = 0;
                    }
                }
            }

            if(me->path_y > SCREEN_SPRITE_BORDER_HEIGHT / 2) {
                me->path_y = SCREEN_SPRITE_BORDER_HEIGHT / 2;
            }

            if(me->path_x > SCREEN_SPRITE_BORDER_WIDTH - 40) {
                me->path_x = SCREEN_SPRITE_BORDER_WIDTH - 40;
            }
        }
    }

    return EXIT_SUCCESS;
}

void cycle_marker(void) {
}

unsigned char render() {
    unsigned char i, sheet_idx, char_type, action_flags, kbchar;
    bool animate = true;
    sprite_sequence* selected_sprite;
    char_sprite_group* selected_char;
    char_state* me;
    unsigned int animation_time;

    sprite_sequence* after_finish = NULL;
    level_screen* screen = state->screens[state->screen_index];

    gotoxy(0,0);

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(!me) continue;

        selected_char = SPRITES[me->char_type];

        if(now % 5 == 0) {
            cycle_marker();
            cputs("x: ");
            cputs_hex_value(me->path_x);
            cputs(" y: ");
            cputs_hex_value(me->path_y);
            cputs(" hp: ");
            cputs_hex_value(me->hitpoints);
            cputs(" a: ");
            cputs_hex_value(me->action_flags);
            cputs(" s: ");
            cputs_hex_value(me->sprite_slot);
            cputs(" c: ");
            cputs_hex_value(me->char_type);
            puts("");
            cycle_marker();
        }

        action_flags = me->action_flags;

        animation_time = now - me->action_start;

        if(action_flags & CHAR_ACTION_DIRECTION_RIGHTLEFT) {
            if(me->status_flags & CHAR_STATUS_INVINCIBLE) {
                selected_sprite = selected_char->oof_left;
                animation_time = now - me->status_start;
                after_finish = selected_char->walk_left;
            }
            else if(action_flags & CHAR_ACTION_ATTACKING) {
                selected_sprite = selected_char->attack_left;
            }
            else if(action_flags & CHAR_ACTION_MOVING_MASK) {
                selected_sprite = selected_char->walk_left;
            }
            else {
                selected_sprite = selected_char->walk_left;
                animate = false;
            }
        }
        else {
            if(me->status_flags & CHAR_STATUS_INVINCIBLE) {
                selected_sprite = selected_char->oof_right;
                animation_time = now - me->status_start;
                after_finish = selected_char->walk_right;
            }
            else if(action_flags & CHAR_ACTION_ATTACKING) {
                selected_sprite = selected_char->attack_right;
            }
            else if(action_flags & CHAR_ACTION_MOVING_MASK) {
                selected_sprite = selected_char->walk_right;
            }
            else {
                selected_sprite = selected_char->walk_right;
                animate = false;
            }
        }

        // In case we don't have a sprite defined for this
        // action, default to an obviously wrong sprite.
        if(!selected_sprite) {
            selected_sprite = SPRITES[CHAR_TYPE_GUY]->neutral;
        }

        if(!animate) {
            sheet_idx = selected_sprite->start_index;
        }
        else {
            if(after_finish) {
                sheet_idx = spritesheet_animation_next(animation_time, selected_sprite->frame_duration, selected_sprite->start_index, selected_sprite->length, false, after_finish->start_index);
            }
            else {
                sheet_idx = spritesheet_animation_next(animation_time, selected_sprite->frame_duration, selected_sprite->start_index, selected_sprite->length, true, NULL);
            }
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

