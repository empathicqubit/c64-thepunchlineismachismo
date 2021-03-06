/* Move a sprite that is already loaded
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param x - X position
 * @param y - Y position
 * @return If we were successful or not
 */
unsigned char sprite_move(unsigned char sprite_slot, unsigned int x, unsigned char y);

/* Get the next sprite in the sequence, based on the time since the action started
 * @param action_time - jiffies since the action started
 * @param frame_duration - jiffies per frame
 * @param sheet_idx_begin - first sprite in the animation
 * @param animation_length - How many frames
 * @param animation_loop - Keep looping the animation after it finishes. Otherwise use sheet_idx_after_finish
 * @param sheet_idx_after_finish - The sprite index to use after the animation completes
 */
unsigned char spritesheet_animation_next(unsigned int action_time, unsigned char frame_duration, unsigned char sheet_idx_begin, unsigned char animation_length, bool animation_loop, unsigned char sheet_idx_after_finish);

/* Load a sprite sheet in SpritePad format
 * @param filename - The filename on disk
 * Must be aligned to 64 - 9 bytes, to leave room for the header and allow VIC-II
 * to access.
 * @return - Whether the sheet successfully loaded into memory.
 */
unsigned char spritesheet_load(unsigned char* filename);

/* Load a sprite with SpritePad metadata byte
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param sheet_idx - The sprite index in the sheet.
 * @param x - X position
 * @param y - Y position
 * @param double_width - Double sprite width
 * @param double_height - Double sprite height
 * @return If we were successful or not
 */
unsigned char spritesheet_show(unsigned char sprite_slot, unsigned char sheet_idx, unsigned int x, unsigned char y, bool dbl_width, bool dbl_height);

/* Advance the sprite to the next one in the sequence.
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param sheet_idx_begin - The first sprite index in the sheet.
 * @param sheet_idx_end - The last sprite index in the sheet.
 */
unsigned char spritesheet_next_image(unsigned char sprite_slot, unsigned char sheet_idx_begin, unsigned char sheet_idx_end);

/* Load a sprite with SpritePad metadata byte
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param sheet_idx - The sprite index in the sheet.
 * @return If we were successful or not
 */
unsigned char spritesheet_set_image(unsigned char sprite_slot, unsigned char sheet_idx);

/** Hide the sprite in the slot
 * @param sprite_slot - The sprite to hide
 * @return If we were successful or not
 */
unsigned char sprite_hide(unsigned char sprite_slot);
