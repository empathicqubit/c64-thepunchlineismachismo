#include <stdio.h>
#include <stdlib.h>
#include "c64.h"
#include "utils.h"

extern void sid_init (void);

extern void sid_play (void);

extern void __fastcall__ sid_play_sound_int(unsigned char idx, unsigned char* startaddress);

typedef struct {
    unsigned char count;
    unsigned char offsets[];
} snz_file;

unsigned char sid_play_sound(unsigned char* snz_pointer, unsigned char sound_idx, unsigned char channel_idx) {
    snz_file* snz = (snz_file*)snz_pointer;
    unsigned char* snd_pointer;

    if(channel_idx > 2) {
        return EXIT_FAILURE;
    }

    if(sound_idx > snz->count - 1) {
        return EXIT_FAILURE;
    }

    snd_pointer = snz_pointer + snz->offsets[sound_idx] + snz->count + 1;

    sid_play_sound_int(channel_idx * 7, snd_pointer);

    return EXIT_SUCCESS;
}

unsigned char sid_load (char filename[]) {
    FILE* fp;

    fp = fopen(filename, "rb");

    if(!fread(SID_START, 256, SID_SIZE / 256 + 1, fp)) {
        fclose(fp);
        return EXIT_FAILURE;
    }

    fclose(fp);

    sid_init();

    return EXIT_SUCCESS;
}

unsigned char* snz_load(char filename[], unsigned char* error) {
    FILE* fp;
    int size = 0;
    unsigned char* snz;

    size = get_filesize(filename);
    if(size == -1) {
        *error = 1;
        return NULL;
    }

    fp = fopen(filename, "rb");

    snz = malloc(size);

    if(!fread(snz, 1, size, fp)) {
        fclose(fp);
        *error = 2;
        return NULL;
    }

    return snz;
}

unsigned char sid_stop(void) {
    *(unsigned char *)SID_Ctl1 = 0x00;
    *(unsigned char *)SID_Ctl2 = 0x00;
    *(unsigned char *)SID_Ctl3 = 0x00;

    return EXIT_SUCCESS;
}

void sid_play_frame (void) {
    sid_play();
}

