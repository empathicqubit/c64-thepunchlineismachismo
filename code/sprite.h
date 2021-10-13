#include <stdbool.h>

#ifndef _SPRITE_H
#define _SPRITE_H

struct sprite_data {
    unsigned char color;
    unsigned char pointer;
    unsigned char lo_x;
    unsigned char lo_y;

    unsigned char ena;
    unsigned char hi_x;
    unsigned char dbl;
    unsigned char multi;
};
typedef struct sprite_data* sprite_handle;

/* Get the next sprite in the sequence, based on the time since the action started
 * @param action_time - updates until the action finishes
 * @param frame_duration - updates per frame
 * @param sheet_idx_begin - first sprite in the animation
 * @param animation_length - How many frames
 * @param animation_loop - Keep looping the animation after it finishes. Otherwise use sheet_idx_after_finish
 * @param sheet_idx_after_finish - The sprite index to use after the animation completes
 */
unsigned char spritesheet_animation_next(unsigned char action_time, unsigned char frame_duration, unsigned char sheet_idx_begin, unsigned char animation_length, bool animation_loop, unsigned char sheet_idx_after_finish);

/* Load a sprite sheet in SpritePad format
 * @param filename - The filename on disk
 * Must be aligned to 64 - 9 bytes, to leave room for the header and allow VIC-II
 * to access.
 * @return - Whether the sheet successfully loaded into memory.
 */
unsigned char spritesheet_load(unsigned char* filename);

void init_sprite_pool(void);

void set_sprite_pointer(sprite_handle handle, unsigned char sprite_pointer);

void set_sprite_graphic(sprite_handle handle, unsigned char sheet_index);

void set_sprite_x(sprite_handle a, unsigned int x);

void set_sprite_y(sprite_handle a, unsigned char y);

void discard_sprite(sprite_handle handle);

sprite_handle new_sprite(bool dbl);

#endif