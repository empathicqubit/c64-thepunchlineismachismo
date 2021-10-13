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

unsigned char level_screen_delete_character(level_screen* screen, register char_state* chara) {
    static unsigned char idx;
    register char_state** charas = screen->characters;
    for(idx = 0; idx < MAX_SCREEN_CHARACTERS - 1; idx++) {
        if(charas[idx] == chara) {
            break;
        }
    }

    for(; idx < MAX_SCREEN_CHARACTERS - 1; idx++) {
        charas[idx] = charas[idx+1];
    }

    discard_sprite(chara->sprite);

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

    return EXIT_SUCCESS;
}

unsigned char process_input(void) {
    static unsigned char joyval;
    register char_state* guy = state->guy;
    joyval = joy_read(0x01);
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
    static unsigned char i;
    static char_state* me;
    register level_screen* screen = state->screens[state->screen_index];

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
    static unsigned char bg_char;

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

unsigned char level_screen_init_sprites(register level_screen* screen) {
    static unsigned char i;
    register char_state* chara;
    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        chara = screen->characters[i];
        if(!chara) {
            continue;
        }
        chara->sprite = new_sprite(true);
    }

    return EXIT_SUCCESS;
}

unsigned char move_to_screen(unsigned char screen_idx) {
    static unsigned char err;
    static level_screen* dest_screen;
    dest_screen = state->screens[screen_idx];

    if(screen_idx > MAX_SCREEN_CHARACTERS || !dest_screen) {
        return EXIT_FAILURE;
    }

    if(err = level_screen_delete_character(state->screens[state->screen_index], state->guy)) {
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
    static unsigned char i, j, err;
    static unsigned char action_frames_left, status_time;
    register char_state *other, *me;
    static char_action_flag action_flags;

    level_screen* screen = state->screens[state->screen_index];
    bool my_last_resort = false;

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(!me) continue;

        action_flags = me->action_flags;

        // Reconcile flags and last_flags
        if(action_flags != me->last_action_flags) {
            if(action_flags & CHAR_ACTION_ATTACKING && !(me->last_action_flags & CHAR_ACTION_ATTACKING)) {
                me->action_frames_left = 25;

                // FIXME sid_play_sound(state->snz, 0, 2);
            }
        }

        if(me->action_frames_left > 0) {
            me->action_frames_left--;
        }
        else if(action_flags & CHAR_ACTION_MOVING_MASK) {
            me->action_frames_left = 2;
        }

        me->last_action_flags = action_flags;

        if(me->status_flags != me->last_status_flags) {
            // FIXME
            me->status_frames_left = 0;
        }

        me->last_status_flags = me->status_flags;

        action_frames_left = me->action_frames_left;
        status_time = me->status_frames_left;

        if((me->status_flags & CHAR_STATUS_INVINCIBLE) && status_time > (FRAMES_PER_SEC*3/2)) {
            me->status_flags &= ~CHAR_STATUS_INVINCIBLE;
        }

        if((action_flags & CHAR_ACTION_DYING) && action_frames_left > FRAMES_PER_SEC) {
            level_screen_delete_character(screen, me);
            free(me);
            break;
        }

        // Dying precludes attacking and moving
        if(action_flags & CHAR_ACTION_DYING) {
            continue;
        }

        if(action_flags & CHAR_ACTION_ATTACKING) {
            if(action_frames_left < 20 && action_frames_left > 5) {
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
                                (me->path_x < other->path_x + 60)
                                && (me->path_x >= other->path_x)
                            )
                            // Attacker facing right
                            : (
                                (me->path_x + 60 > other->path_x)
                                && (me->path_x + 60 < other->path_x + 60)
                            )
                        )
                        && (other->path_y >= 25 ? me->path_y > other->path_y - 25 : true)
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
            else if(action_frames_left == 0) {
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
                        if(state->guy == me && !(err = move_to_screen(state->screen_index - 1))) {
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
                if(state->guy == me && !(err = move_to_screen(state->screen_index + 1))) {
                    me->path_x = 0;
                    break;
                }
                else {
                    me->path_x = MAX_PATH_X;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}

unsigned char render() {
    static unsigned char i, sheet_idx, action_flags, err;
    static bool animate;
    static sprite_sequence* selected_sprite;
    static char_sprite_group* selected_char;
    register char_state *me;
    static unsigned int animation_time;

    static sprite_sequence* after_finish = NULL;
    static level_screen* screen;

    screen = state->screens[state->screen_index];

    if(!state->bg_rendered && (
        (err = level_screen_init_bg(screen->bg_data, screen->bg_length))
        || (err = level_screen_init_sprites(screen))
        )
    ) {
        return err;
    }
    state->bg_rendered = true;

    for(i = 0; i < MAX_SCREEN_CHARACTERS; i++) {
        me = screen->characters[i];

        if(!me) continue;

        animate = true;

        selected_char = SPRITES[me->char_type];

        action_flags = me->action_flags;

        animation_time = me->action_frames_left;

        if(action_flags & CHAR_ACTION_DIRECTION_RIGHTLEFT) {
            if(me->status_flags & CHAR_STATUS_INVINCIBLE) {
                selected_sprite = selected_char->oof_left;
                animation_time = me->status_frames_left;
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
                animation_time = me->status_frames_left;
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

        set_sprite_graphic(me->sprite, sheet_idx);
        set_sprite_x(me->sprite, SCREEN_SPRITE_BORDER_X_START + me->path_x + ((SCREEN_SPRITE_BORDER_HEIGHT / 2 - me->path_y) / 4));
        set_sprite_y(me->sprite, (me->path_y / 2) + (SCREEN_SPRITE_BORDER_HEIGHT * 3 / 4));
    }

    return EXIT_SUCCESS;
}

unsigned char level_state_init(unsigned char num) {
    static unsigned char i, err;
    static unsigned char* bg_data;
    static char_state* meece;
    static level_screen** screens;
    static level_screen* screen;

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
        meece->path_y = state->guy->path_y;
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
        meece->path_x = MAX_PATH_X / 2;
        meece->path_y = MAX_PATH_Y;
        level_screen_add_character(screen, meece);

        meece = char_state_init(CHAR_TYPE_MOOSE);
        meece->path_x = MAX_PATH_X / 4;
        meece->path_y = MAX_PATH_Y;
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
    static unsigned char err;
    register level_screen* screen;

    is_pal = get_tv();

    init_sprite_pool();

    puts("growing pine needles...");

    if(err = spritesheet_load("canada.spd")) {
        printf("sprite load error: %s\n", strerror(err));
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

    setup_irq_handler();
    screen_init(true);

    bgcolor(COLOR_LIGHTBLUE);
    bordercolor(COLOR_GREEN);

    game_clock = 0;
    while (true)
    {
        process_input();
        process_cpu_input();

        if(game_clock != 0) {
            update();
            game_clock--;
            continue;
        }

        if(err = render()) {
            printf("Error rendering the game state: %x\n", err);
            return EXIT_FAILURE;
        }
    }
}

