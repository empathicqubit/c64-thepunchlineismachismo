#include <stdbool.h>

#define FRAMES_PER_SEC 50

/** Check if system is actually PAL and store in the PALFLAG
 * More reliable than the system's flag apparently.
 */
void pal_system(void);

/** Wait a number of milliseconds
 * @param duration - Milliseconds to wait
 */
void wait (unsigned int duration);

/** Get an unused logical file number for setlfs
 * @return The next unused LFN
 */
unsigned char utils_get_unused_lfn(void);

/** Reset the screen to VIC bank #2
 * @param clear - Clear the screen before switching to it.
 */
void screen_init (bool clear);

/** Disable the IO page
 */
void __fastcall__ hide_io(void);
/** Enable the IO page
 */
void __fastcall__ show_io(void);

/** Write a value to stdout
 * @param value - The value to write
 */
void cputs_hex_value(int value);

/** Copies character ROM to RAM.
 * @param use_graphics_charset - Use fancy graphics chars with no lowercase
 */
void character_init(bool use_graphics_charset);

/** Get the file size in a stupid way.
 * @param filename - The filename to check.
 */
int get_filesize(char filename[]);

unsigned char setup_irq_handler(unsigned char (*handler)(void));

unsigned char consume_raster_irq(void (*raster_handler)(void));
