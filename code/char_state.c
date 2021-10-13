#include <stdlib.h>
#include "../resources/sprites/canada.h"
#include "char_state.h"

char_state* char_state_init(char_type c) {
    register char_state* state = calloc(1, sizeof(char_state));
    state->char_type = c;
    state->action_frames_left = 0;
    state->status_frames_left = 0;
    if(c == CHAR_TYPE_GUY) {
        state->path_x = 25;
        state->path_y = 25;
        state->movement_speed = 2;
        state->default_sprite = SPRITES[c]->neutral->start_index;
    }
    else if(c == CHAR_TYPE_MOOSE) {
        state->default_sprite = SPRITES[c]->neutral->start_index;
        state->movement_speed = 1;
        state->hitpoints = 5;
        state->path_x = 160;
        state->path_y = 50;
    }

    return state;
}