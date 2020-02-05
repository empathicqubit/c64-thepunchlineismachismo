#include <stdbool.h>

/** Check if system is actually PAL and store in the PALFLAG
 * More reliable than the system's flag apparently.
 */
void pal_system(void);

/** Wait a number of milliseconds
 * @param duration - Milliseconds to wait
 */
void wait (unsigned int duration);

/** Reset the screen to VIC bank #2
 * @param use_graphics_charset - Use fancy graphics chars with no lowercase
 */
void screen_init (bool use_graphics_charset);

/** Write a value to stdout
 * @param value - The value to write
 */
void cputs_hex_value(int value);

/** Copies character ROM to RAM.
 */
void character_init(void);

/** Get the file size in a stupid way.
 * @param filename - The filename to check.
 */
int get_filesize(char filename[]);

unsigned char setup_irq_handler(unsigned char (*handler)(void));

unsigned char consume_raster_irq(void (*raster_handler)(void));
