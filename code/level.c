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

void debug_marker() {}

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

void depth_sort(char_state **characters) {
    int i, j; 
    char_state *key;
    for (i = 1; i < MAX_SCREEN_CHARACTERS; i++) { 
        key = characters[i]; 
        j = i - 1; 
  
        /* Move elements of arr[0..i-1], that are 
          greater than key, to one position ahead 
          of their current position */
        while (j >= 0 && characters[j] && key && characters[j]->path_y < key->path_y) { 
            characters[j + 1] = characters[j]; 
            j = j - 1; 
        } 
        characters[j + 1] = key; 
    } 
}

unsigned char update(void) {
    unsigned char i;
    unsigned char j;
    unsigned int action_time, status_time;
    char_state* other;
    char_action_flag action_flags;
    char_state* me;

    level_screen* screen = state->screens[state->screen_index];
    bool my_last_resort = false;

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

        if((me->status_flags & CHAR_STATUS_INVINCIBLE) && status_time > (FRAMES_PER_SEC*3/2)) {
            me->status_flags &= ~CHAR_STATUS_INVINCIBLE;
        }

        if((action_flags & CHAR_ACTION_DYING) && action_time > FRAMES_PER_SEC) {
            screen->characters[i] = NULL;
            free(me);
        }

        // Dying precludes attacking and moving
        if(action_flags & CHAR_ACTION_DYING) {
            continue;
        }

        if(action_flags & CHAR_ACTION_ATTACKING) {
            if(action_time > (FRAMES_PER_SEC/6) && action_time < (FRAMES_PER_SEC/3)) {
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
            else if(action_time > (FRAMES_PER_SEC/2)) {
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

                my_last_resort = true;
            }

            if(me->path_y > SCREEN_SPRITE_BORDER_HEIGHT / 2) {
                me->path_y = SCREEN_SPRITE_BORDER_HEIGHT / 2;
            }

            if(me->path_x > SCREEN_SPRITE_BORDER_WIDTH - 40) {
                me->path_x = SCREEN_SPRITE_BORDER_WIDTH - 40;
            }
        }
    }

    if(my_last_resort && now % 2 == 0) {
        depth_sort(screen->characters);
    }

    return EXIT_SUCCESS;
}

unsigned char render() {
    unsigned char i, sheet_idx, action_flags;
    bool animate = true;
    sprite_sequence* selected_sprite;
    char_sprite_group* selected_char;
    char_state *me;
    unsigned int animation_time;

    sprite_sequence* after_finish = NULL;
    level_screen* screen = state->screens[state->screen_index];

    gotoxy(0,0);

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(!me) continue;

        selected_char = SPRITES[me->char_type];

        if(now % 5 == 0) {
            cputs("x: ");
            cputs_hex_value(me->path_x);
            cputs(" y: ");
            cputs_hex_value(me->path_y);
            cputs(" i: ");
            cputs_hex_value(i);
            cputs(" hp: ");
            cputs_hex_value(me->hitpoints);
            cputs(" a: ");
            cputs_hex_value(me->action_flags);
            cputs(" c: ");
            cputs_hex_value(me->char_type);
            puts("");
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

        spritesheet_show(i % VIC_SPR_COUNT, sheet_idx, SCREEN_SPRITE_BORDER_X_START + me->path_x + ((SCREEN_SPRITE_BORDER_HEIGHT / 2 - me->path_y) / 4), (me->path_y / 2) + (SCREEN_SPRITE_BORDER_HEIGHT * 3 / 4), true, true);
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

unsigned char level_screen_add_character(level_screen* screen, char_state* chara) {
    unsigned char i;
    char_state* cur;
    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        cur = screen->characters[i];
        if(!cur) {
            break;
        }
        
        if(chara->path_y > cur->path_y) {
            break;
        }
    }

    screen->characters[i] = chara;
    i++;

    for(; i < MAX_SCREEN_CHARACTERS; i++) {
        screen->characters[i] = cur;
        cur = screen->characters[i+1];
    }
}

unsigned char play_level (void) {
    unsigned char err;

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

    state->guy = char_state_init(CHAR_TYPE_GUY);

    state->screen_index = 0;

    level_screen_add_character(screen, state->guy);
    level_screen_add_character(screen, char_state_init(CHAR_TYPE_MOOSE));

    gotoxy(0, 20);

    state->screens[0] = screen;

    setup_irq_handler(&level_irq_handler);

    screen_init(false, true);

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

