#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include "c64.h"
#include "sprite.h"

#define SPD_PADDING 55

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
    unsigned char padding[SPD_PADDING];
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

#define SPRITE_MAX 80

/* Load a sprite sheet in SpritePad format
 * @param filename - The filename on disk
 * @return - Whether the sheet successfully loaded into memory.
 */
unsigned char spritesheet_load(unsigned char* filename) {
    static FILE* fp;
    register spd* spd_data;
    static unsigned char* header;

    fp = fopen(filename, "rb");
    if(!fp) {
        if(errno != 0) {
            return errno;
        }
        else {
            return EXIT_FAILURE;
        }
    }

    // Used to write this directly to the memory area, but we can't read it if we do.
    if(!(header = calloc(1, VIC_SPR_SIZE))
        || !fread(header + SPD_PADDING, VIC_SPR_SIZE - SPD_PADDING, 1, fp)) {
        fclose(fp);
        return EXIT_FAILURE;
    }

    spd_data = (spd*)header;

    if(spd_data->sprite_count + 1 > SPRITE_MAX) {
        free(header);
        fclose(fp);
        return EXIT_FAILURE;
    }

    memcpy(SPRITE_START, header, VIC_SPR_SIZE);

    if(!fread(SPRITE_START + VIC_SPR_SIZE, VIC_SPR_SIZE, spd_data->sprite_count + 1, fp)) {
        fclose(fp);
        return EXIT_FAILURE;
    }

    // FIXME tf is background? The background is transparent!

    *(unsigned char *)VIC_SPR_MCOLOR0 = spd_data->multicolor_0;
    *(unsigned char *)VIC_SPR_MCOLOR1 = spd_data->multicolor_1;

    free(header);
    fclose(fp);

    return EXIT_SUCCESS;
}

#define SPD_SPRITE_MULTICOLOR_ENABLE_MASK 0x80
#define SPD_SPRITE_COLOR_VALUE_MASK 0x0F

#define SPRITE_POOL_SIZE 32
struct sprite_data _sprite_pool[SPRITE_POOL_SIZE];
sprite_handle _sprite_list[SPRITE_POOL_SIZE];
unsigned char sprite_count = 0;

void init_sprite_pool(void) {
    memset(&_sprite_pool, 0x00, sizeof(struct sprite_data) * SPRITE_POOL_SIZE);
    memset(&_sprite_list, NULL, sizeof(sprite_handle) * SPRITE_POOL_SIZE);
}

void set_sprite_pointer(sprite_handle handle, unsigned char sprite_pointer) {
    static spd_sprite* sprite;

    sprite = (spd_sprite*)(SCREEN_START + sprite_pointer * VIC_SPR_SIZE);

    handle->pointer = sprite_pointer;
    handle->color = sprite->metadata & SPD_SPRITE_COLOR_VALUE_MASK;
    if(sprite->metadata & SPD_SPRITE_MULTICOLOR_ENABLE_MASK) {
        handle->multi = handle->ena;
    }
    else {
        handle->multi = 0;
    }
}

void set_sprite_graphic(sprite_handle handle, unsigned char sheet_index) {
    static spd* s = (spd*)SPRITE_START;
    set_sprite_pointer(handle, ((unsigned int)(&s->sprites[sheet_index]) % VIC_BANK_SIZE) / VIC_SPR_SIZE);
}

void set_sprite_x(sprite_handle a, unsigned int x) {
    static sprite_handle arg;

    arg = a;

    if(x>>8) {
        arg->hi_x = arg->ena;
    }
    else {
        arg->hi_x = 0;
    }

    arg->lo_x = (unsigned char)x;
}

void set_sprite_y(sprite_handle a, unsigned char y) {
    register sprite_handle *comp_handle, *current_handle;
    register sprite_handle comp;
    static sprite_handle* start_handle;
    static sprite_handle arg;
    static unsigned char index, last_index, hi_mask, comp_y, yarg;
    static bool direction, is_last;

    yarg = y;
    arg = a;

    comp_y = arg->lo_y;
    if(yarg == comp_y) {
        return;
    }
    arg->lo_y = yarg;

    index = 0;
    for(
        start_handle = _sprite_list;
        *start_handle != arg;
        start_handle++
    ) {
        index++;
    }

    if(yarg > comp_y) {
        last_index = sprite_count - 1;
        if(last_index == index) {
            return;
        }
        direction = true;
    }
    else {
        if(index == 0) {
            return;
        }
        direction = false;
    }

    current_handle = start_handle;
    do {
        if(direction) {
            comp_handle = current_handle + 1;
            comp = *comp_handle;
            is_last = (yarg <= comp->lo_y
                || index == last_index);
        }
        else {
            comp_handle = current_handle - 1;
            comp = *comp_handle;
            is_last = (comp->lo_y <= yarg
                || index == 0);
        }

        if(is_last) {
            if(current_handle == start_handle) {
                break;
            }

            comp = arg;
        }

        hi_mask = 1<<(index%VIC_SPR_COUNT);

        __asm__("ldy #%b", offsetof(struct sprite_data, ena));

        __asm__("ldx %v", hi_mask);
        __asm__("loop: lda (%v),Y", comp);
        __asm__("beq done");
        __asm__("txa");
        __asm__("sta (%v),Y", comp);
        __asm__("done: iny");
        __asm__("tya");
        __asm__("sbc #%b", offsetof(struct sprite_data, multi));
        __asm__("bne loop");

        if(is_last) {
            *current_handle = comp;
            break;
        }

        *current_handle = comp;
        *comp_handle = arg;

        if(direction) {
            index++;
            current_handle++;
        }
        else {
            index--;
            current_handle--;
        }

    } while (true);
}

void discard_sprite(sprite_handle handle) {
    sprite_count--;
    _sprite_list[sprite_count] = NULL;
    set_sprite_x(handle, 0xff);
    set_sprite_y(handle, 0xff);
}

sprite_handle new_sprite(bool dbl) {
    register sprite_handle handle;
    static unsigned char hi_mask;

    handle = &_sprite_pool[sprite_count];
    _sprite_list[sprite_count] = handle;
    hi_mask = 1<<(sprite_count%VIC_SPR_COUNT);
    handle->ena = hi_mask;
    if(dbl) {
        handle->dbl = hi_mask;
    }
    else {
        handle->dbl = 0;
    }
    handle->lo_x = 0xfe;
    handle->lo_y = 0xfe;
    sprite_count++;

    return handle;
}
