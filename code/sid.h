unsigned char sid_load (char filename[]);

unsigned char sid_stop (void);

void sid_play_frame (void);

unsigned char sid_play_sound(unsigned char* snz_pointer, unsigned char sound_idx, unsigned char channel_idx);

unsigned char* snz_load(char filename[], unsigned char* error);
