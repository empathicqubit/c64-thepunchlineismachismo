#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <cbm.h>
#include <c64.h>
#include <joystick.h>
#include <6502.h>
#include <conio.h>
#include "utils.h"
#include "c64.h"
#include "seq.h"
#include "sprite.h"
#include "sid.h"
#include "../resources/sprites/canada.h"
#include "char_state.h"

#define MAX_SCREEN_CHARACTERS 16
#define MAX_SCREENS 16

#define MAX_PATH_X (SCREEN_SPRITE_BORDER_WIDTH - 40)
#define MAX_PATH_Y (SCREEN_SPRITE_BORDER_HEIGHT / 2)

void debug_marker() {}

typedef struct level_screen level_screen;
struct level_screen {
    char_state* characters[MAX_SCREEN_CHARACTERS];
    unsigned char* bg_data;
    unsigned int bg_length;
};

typedef struct level_state level_state;
struct level_state {
    unsigned char* snz; // Pointer to the loaded sounds
    unsigned char screen_index; // Which screen are we looking at.
    bool bg_rendered; // Was the background rendered since update?
    char_state* guy; // State for our guy gets put here so it's easier
                    // To find him, since we deal with him a lot.
    level_screen* screens[MAX_SCREENS];
};

level_state* state;
unsigned int now;

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

unsigned char level_screen_delete_character(level_screen* screen, unsigned char idx) {
    char_state** charas = screen->characters;
    sprite_hide(idx);
    for(; idx < MAX_SCREEN_CHARACTERS - 1; idx++) {
        charas[idx] = charas[idx+1];
        if(!charas[idx]) {
            sprite_hide(idx % VIC_SPR_COUNT);
        }
    }

    return EXIT_SUCCESS;
}

unsigned char level_screen_add_character(level_screen* screen, char_state* chara) {
    signed char i;
    char_state** charas = screen->characters;
    for(i = MAX_SCREEN_CHARACTERS - 1; i >= 0; i--) {
        if(charas[i]) {
            break;
        }
    }

    charas[i+1] = chara;
    depth_sort(charas);

    return EXIT_SUCCESS;
}

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

        if(!me) continue;

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

unsigned char* level_screen_load_bg(unsigned char* filename, unsigned int* fullsize) {
    return seq_load(filename, fullsize);
}

unsigned char level_screen_init_bg(unsigned char* bg, unsigned int fullsize) {
    unsigned int partialsize, i;
    unsigned char bg_char;

    partialsize = fullsize;
    while((bg_char = bg[partialsize-1]) && bg_char < 0x20 || (bg_char >= 0x80 && bg_char <= 0x9f)) {
        partialsize--;
    }
    partialsize--;

    gotoxy(0,0);
    cbm_k_ckout(STDOUT_FILENO);
    for(i = 0; i < partialsize; i++) {
        cbm_k_bsout(bg[i]);
    }
    screen_init(false);

    *(unsigned char*)(SCREEN_START + XSIZE * YSIZE - 1) = *(unsigned char*)(SCREEN_START + XSIZE * YSIZE - 2);
    COLOR_RAM[COLOR_RAM_SIZE - 1] = COLOR_RAM[COLOR_RAM_SIZE - 2];

    return EXIT_SUCCESS;
}


unsigned char move_to_screen(unsigned char screen_idx, unsigned char guy_idx) {
    unsigned char err;
    level_screen* dest_screen = state->screens[screen_idx];

    if(screen_idx > MAX_SCREEN_CHARACTERS || !dest_screen) {
        return EXIT_FAILURE;
    }

    if(err = level_screen_delete_character(state->screens[state->screen_index], guy_idx)) {
        return err;
    }

    state->screen_index = screen_idx;

    if(err = level_screen_add_character(dest_screen, state->guy)) {
        return err;
    }

    state->bg_rendered = false;

    return EXIT_SUCCESS;
}

unsigned char update(void) {
    unsigned char i, j, err;
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
            level_screen_delete_character(screen, i);
            free(me);
            break;
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
                            ? (
                                (me->path_x < other->path_x + 50)
                                && (me->path_x >= other->path_x)
                            )
                            // Attacker facing right
                            : (
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
                        if(state->guy == me && !(err = move_to_screen(state->screen_index - 1, i))) {
                            me->path_x = MAX_PATH_X;
                            break;
                        }
                        else {
                            me->path_x = 0;
                        }
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

            if(me->path_y > MAX_PATH_Y) {
                me->path_y = MAX_PATH_Y;
            }

            if(me->path_x > MAX_PATH_X) {
                if(state->guy == me && !(err = move_to_screen(state->screen_index + 1, i))) {
                    me->path_x = 0;
                    break;
                }
                else {
                    me->path_x = MAX_PATH_X;
                }
            }
        }
    }

    if(my_last_resort && now % 2 == 0) {
        depth_sort(screen->characters);
    }

    return EXIT_SUCCESS;
}

unsigned char render() {
    unsigned char i, sheet_idx, action_flags, err;
    bool animate = true;
    sprite_sequence* selected_sprite;
    char_sprite_group* selected_char;
    char_state *me;
    unsigned int animation_time;

    sprite_sequence* after_finish = NULL;
    level_screen* screen = state->screens[state->screen_index];

    if(!state->bg_rendered && (err = level_screen_init_bg(screen->bg_data, screen->bg_length))) {
        return err;
    }
    state->bg_rendered = true;

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(!me) continue;

        selected_char = SPRITES[me->char_type];

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
    if(state->bg_rendered) {
        update();
    }
}

unsigned char level_irq_handler(void) {
    return consume_raster_irq(&level_raster_irq);
}

unsigned char level_state_init(unsigned char num) {
    unsigned char i, err;
    unsigned char* bg_data;
    char_state* meece;
    level_screen** screens;
    level_screen* screen;

    state = calloc(1, sizeof(level_state));

    screens = state->screens;

    if(!(state->snz = snz_load("canada.snz", &err))) {
        printf("sound load error: %x\n", err);
        return EXIT_FAILURE;
    }

    // FIXME map screen?
    if(num == 0) {
        screen = calloc(1, sizeof(level_screen));

        screen->bg_data = "froad1.sex";

        state->guy = char_state_init(CHAR_TYPE_GUY);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = 0;
        meece->path_y = 0;
        level_screen_add_character(screen, meece);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = MAX_PATH_X;
        meece->path_y = MAX_PATH_Y;
        level_screen_add_character(screen, meece);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = 0;
        meece->path_y = MAX_PATH_Y;
        level_screen_add_character(screen, meece);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = MAX_PATH_X;
        meece->path_y = 0;
        level_screen_add_character(screen, meece);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = MAX_PATH_X / 2;
        meece->path_y = MAX_PATH_Y / 2;
        level_screen_add_character(screen, meece);

        screens[0] = screen;

        screen = calloc(1, sizeof(level_screen));

        screen->bg_data = "froad2.sex";

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = 0;
        meece->path_y = 0;
        level_screen_add_character(screen, meece);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = MAX_PATH_X;
        meece->path_y = MAX_PATH_Y;
        level_screen_add_character(screen, meece);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = 0;
        meece->path_y = MAX_PATH_Y;
        level_screen_add_character(screen, meece);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = MAX_PATH_X;
        meece->path_y = 0;
        level_screen_add_character(screen, meece);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = MAX_PATH_X / 2;
        meece->path_y = MAX_PATH_Y / 2;
        level_screen_add_character(screen, meece);

        screens[1] = screen;

        state->screen_index = 0;

        level_screen_add_character(screens[state->screen_index], state->guy);
    }
    else {
        return EXIT_FAILURE;
    }

    for(i = 0; i < MAX_SCREENS; i++) {
        screen = screens[i];
        if(!screen) {
            break;
        }

        if(!(screen->bg_data = level_screen_load_bg(screen->bg_data, &(screen->bg_length)))) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

unsigned char play_level (void) {
    unsigned char err;
    level_screen* screen;

    screen_init(true);

    puts("growing pine needles...");

    if(err = spritesheet_load("canada.spd")) {
        printf("sprite load error: %x\n", err);
        while(1);
    }

    puts("beebifying hair spikes...");

    if(err = sid_load("empty.sid")) {
        printf("sid load error: %x\n", err);
        return EXIT_FAILURE;
    }

    puts("compressing accordions...");

    if(err = level_state_init(0)) {
        printf("level state init error: %x\n", err);
        return EXIT_FAILURE;
    }

    screen = state->screens[state->screen_index];

    if(err = level_screen_init_bg(screen->bg_data, screen->bg_length)) {
        printf("level background init error: %x\n", err);
        return EXIT_FAILURE;
    }

    setup_irq_handler(&level_irq_handler);

    bgcolor(COLOR_LIGHTBLUE);
    bordercolor(COLOR_GREEN);

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

