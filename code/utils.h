/* Check if system is PAL
 * @return true if PAL
 */
bool pal_system(void);

/* Wait a number of milliseconds
 * @param duration - Milliseconds to wait
 */
void wait (unsigned int duration);

/* Reset the screen to VIC bank #2
 * @param use_graphics_charset - Use fancy graphics chars with no lowercase
 */
void screen_init (bool use_graphics_charset);

/* Copies character ROM to RAM.
 */
void character_init(void);
