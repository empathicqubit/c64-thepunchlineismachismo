/* Move a sprite that is already loaded
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param x - X position
 * @param y - Y position
 */
void sprite_move(unsigned char sprite_slot, unsigned int x, unsigned char y);

/* Load a sprite sheet in SpritePad format
 * @param filename - The filename on disk
 * Must be aligned to 64 - 9 bytes, to leave room for the header and allow VIC-II
 * to access.
 * @return - Whether the sheet successfully loaded into memory.
 */
unsigned char spritesheet_load(char filename[]);

/* Load a sprite with SpritePad metadata byte
 * @param sheet_idx - The sprite index in the sheet.
 * @param sprite_slot - Which of the 8 sprite slots on the C64.
 * @param x - X position
 * @param y - Y position
 */
unsigned char spritesheet_show(unsigned char sheet_idx, unsigned char sprite_slot, unsigned int x, unsigned char y, bool dbl_width, bool dbl_height);
