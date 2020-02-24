#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "c64.h"

/* Move a sprite that is already loaded
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param x - X position
 * @param y - Y position
 * @return If we were successful or not
 */
unsigned char sprite_move(unsigned char sprite_slot, unsigned int x, unsigned char y) {
    unsigned char hi_mask = (1<<sprite_slot);

    unsigned char* hi = VIC_SPR_HI_X;

    unsigned char* sprite_x = VIC_SPR0_X + sprite_slot * 2;
    unsigned char* sprite_y = VIC_SPR0_Y + sprite_slot * 2;

    if(sprite_slot > VIC_SPR_COUNT-1) return EXIT_FAILURE;

    if(x>>8) {
        *hi |= hi_mask;
    }
    else {
        *hi &= ~hi_mask;
    }

    *sprite_x = (unsigned char)x;
    *sprite_y = y;

    return EXIT_SUCCESS;
}

unsigned char spritesheet_animation_next(unsigned int action_time, unsigned char frame_duration, unsigned char sheet_idx_begin, unsigned char animation_length, bool animation_loop, unsigned char sheet_idx_after_finish) {
    if(!animation_loop && action_time > animation_length * frame_duration) {
        return sheet_idx_after_finish;
    }

    return sheet_idx_begin + ((action_time % (animation_length * frame_duration)) / frame_duration);
}

struct spd_sprite {
    unsigned char sprite_data[63];
    unsigned char metadata;
};
typedef struct spd_sprite spd_sprite;

struct spd {
    unsigned char magic[3];
    unsigned char version;
    unsigned char sprite_count;
    unsigned char animation_count;
    unsigned char background_color;
    unsigned char multicolor_0;
    unsigned char multicolor_1;
    spd_sprite sprites[];
};
typedef struct spd spd;

const unsigned char* SPRITE_AREA = SCREEN_START + VIC_VIDEO_ADR_SCREEN_DIVISOR + VIC_SPR_SIZE - 9;
const unsigned char SPRITE_MAX = 47;

/* Load a sprite sheet in SpritePad format
 * @param filename - The filename on disk
 * @return - Whether the sheet successfully loaded into memory.
 */
unsigned char spritesheet_load(char filename[]) {
    FILE* fp;
    spd* spd_data;

    fp = fopen(filename, "rb");

    if(!fread(SPRITE_AREA, 9, 1, fp)) {
        fclose(fp);
        return EXIT_FAILURE;
    }

    spd_data = (spd*)SPRITE_AREA;

    if(spd_data->sprite_count + 1 > SPRITE_MAX) {
        fclose(fp);
        return EXIT_FAILURE;
    }

    if(!fread(SPRITE_AREA + 9, VIC_SPR_SIZE, spd_data->sprite_count + 1, fp)) {
        fclose(fp);
        return EXIT_FAILURE;
    }

    // FIXME tf is background? The background is transparent!

    *(unsigned char *)VIC_SPR_MCOLOR0 = spd_data->multicolor_0;
    *(unsigned char *)VIC_SPR_MCOLOR1 = spd_data->multicolor_1;

    return EXIT_SUCCESS;
}

const unsigned char SPD_SPRITE_MULTICOLOR_ENABLE_MASK = 0x80;
const unsigned char SPD_SPRITE_COLOR_VALUE_MASK = 0x0F;

/* Get the current sprite index in the spritesheet
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 */
unsigned char spritesheet_get_index(unsigned char sprite_slot) {
    unsigned char* spr_pointers;
    unsigned char spr_pointer;
    spd* spd_data = (spd*)SPRITE_AREA;

    if(sprite_slot > VIC_SPR_COUNT-1) return -1;

    spr_pointers = SCREEN_START + 0x03F8;

    spr_pointer = spr_pointers[sprite_slot];

    return spr_pointer - (unsigned char)((unsigned int)&(spd_data->sprites) / VIC_SPR_SIZE);
}

/* Load a sprite with SpritePad metadata byte
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param sheet_idx - The sprite index in the sheet.
 * @return If we were successful or not
 */
unsigned char spritesheet_set_image(unsigned char sprite_slot, unsigned char sheet_idx) {
    unsigned char* spr_pointers;
    spd* spd_data = (spd*)SPRITE_AREA;
    spd_sprite* spd_sprite;
    char hi_mask;

    if(sprite_slot > VIC_SPR_COUNT-1) return EXIT_FAILURE;

    hi_mask = (1<<sprite_slot);

    spd_sprite = &(spd_data->sprites[sheet_idx]);

    spr_pointers = SCREEN_START + 0x03F8;

    spr_pointers[sprite_slot] = (unsigned char)((unsigned int)spd_sprite / VIC_SPR_SIZE);

    if(spd_sprite->metadata & SPD_SPRITE_MULTICOLOR_ENABLE_MASK) {
        *(char *)VIC_SPR_MCOLOR |= hi_mask;
    }
    else {
        *(char *)VIC_SPR_MCOLOR &= ~hi_mask;
    }

    ((char *)VIC_SPR0_COLOR)[sprite_slot] = spd_sprite->metadata;

    return EXIT_SUCCESS;
}

/** Hide the sprite in the slot
 * @param sprite_slot - The sprite to hide
 * @return If we were successful or not
 */
unsigned char sprite_hide(unsigned char sprite_slot) {
    char hi_mask;
    if(sprite_slot > VIC_SPR_COUNT-1) return EXIT_FAILURE;

    hi_mask = (1<<sprite_slot);

    *(unsigned char *)VIC_SPR_ENA &= ~hi_mask;

    return EXIT_SUCCESS;
}

/* Load a sprite with SpritePad metadata byte
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param sheet_idx - The sprite index in the sheet.
 * @param x - X position
 * @param y - Y position
 * @param double_width - Double sprite width
 * @param double_height - Double sprite height
 * @return If we were successful or not
 */
unsigned char spritesheet_show(unsigned char sprite_slot, unsigned char sheet_idx, unsigned int x, unsigned char y, bool double_width, bool double_height) {
    char hi_mask;
    unsigned char err;

    if(err = spritesheet_set_image(sprite_slot, sheet_idx)) {
        return err;
    }

    if(err = sprite_move(sprite_slot, x, y)) {
        return err;
    }

    hi_mask = (1<<sprite_slot);

    if(double_width) {
        *(unsigned char *)VIC_SPR_EXP_X |= hi_mask;
    }
    else {
        *(unsigned char *)VIC_SPR_EXP_X &= ~hi_mask;
    }

    if(double_height) {
        *(unsigned char *)VIC_SPR_EXP_Y |= hi_mask;
    }
    else {
        *(unsigned char *)VIC_SPR_EXP_Y &= ~hi_mask;
    }

    *(unsigned char *)VIC_SPR_ENA |= hi_mask;

    return EXIT_SUCCESS;
}

/* Advance the sprite to the next one in the sequence.
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param sheet_idx_begin - The first sprite index in the sheet.
 * @param sheet_idx_end - The last sprite index in the sheet.
 */
unsigned char spritesheet_next_image(unsigned char sprite_slot, unsigned char sheet_idx_begin, unsigned char sheet_idx_end) {
    unsigned char current_idx = spritesheet_get_index(sprite_slot);
    unsigned char err;

    if(current_idx == -1) {
        return EXIT_FAILURE;
    }

    current_idx++;

    if(current_idx > sheet_idx_end || current_idx < sheet_idx_begin) {
        current_idx = sheet_idx_begin;
    }

    if(err = spritesheet_set_image(sprite_slot, current_idx)) {
        return err;
    }

    return EXIT_SUCCESS;
}
