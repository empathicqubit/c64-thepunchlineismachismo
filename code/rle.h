typedef struct rle_cursor rle_cursor;

/** Load the packed RLE data into memory from a file
 * @param fp - File pointer
 * @param dest - Destination pointer. Memory will be automatically allocated if this isn't set.
 * @param unpacked_size - Sets the unpacked size of the file, the only attribute we want to expose to the caller.
 * @return A read cursor which points to the data in memory and can be used to retrieve the file a piece at a time
 */
rle_cursor* rle_open(unsigned char* filename, unsigned int* unpacked_size);

/** Close an RLE cursor
 * @param cursor - RLE data pointer
 * @return Status code
 */
unsigned char rle_close(rle_cursor* cursor);

/** RLE unpack data in memory
 * @param src - RLE data cursor obtained from rle_load_file
 * @param dest - Destination pointer
 * @param count - How many unpacked bytes to write before stopping
 * @return Status code
 */
unsigned char rle_unpack(rle_cursor* cursor, unsigned char* dest, unsigned int count);
