#include <stdio.h>
#include <stdlib.h>
#include <c64.h>
#include <string.h>
#include "rle.h"
#include "c64.h"

struct rle_pair {
    unsigned char count;
    unsigned char byte;
} ;
typedef struct rle_pair rle_pair;

struct rle {
    unsigned int count;
    unsigned int unpacked_size;
    rle_pair pairs[];
};
typedef struct rle rle;

struct rle_cursor {
    void* src;
    unsigned int index;
    unsigned char skip;
};

/** Load the packed RLE data into memory from a file
 * @param fp - File pointer
 * @param dest - Destination pointer. Memory will be automatically allocated if this isn't set.
 * @param unpacked_size - Sets the unpacked size of the file, the only attribute we want to expose to the caller.
 * @return A read cursor which points to the data in memory and can be used to retrieve the file a piece at a time
 */
rle_cursor* rle_load_file(FILE* fp, unsigned char* dest, unsigned int* unpacked_size) {
    unsigned int count;
    rle* data;
    rle_cursor* cursor;
    if(!fread(&count, sizeof(unsigned int), 1, fp)) {
        fclose(fp);
        return NULL;
    }

    data = (rle*)dest;

    if(!data) {
        data = malloc(4 + count * sizeof(rle_pair));
    }

    data->count = count;

    if(
            !fread(&(data->unpacked_size), sizeof(unsigned int), 1, fp)
            || !fread(&(data->pairs), sizeof(rle_pair), count, fp)
      ) {
        fclose(fp);
        return NULL;
    }

    *unpacked_size = data->unpacked_size;

    cursor = calloc(1, sizeof(rle_cursor));

    cursor->src = data;

    return cursor;
}

/** RLE unpack data in memory
 * @param src - RLE data pointer
 * @param dest - Destination pointer
 * @param seek - Index of first RLE pair to read
 * @param count - How many unpacked bytes to write before stopping
 * @return Whether we were successful
 */
unsigned char rle_unpack(rle_cursor* cursor, unsigned char* dest, unsigned int count) {
    unsigned char data_count;
    rle_pair pair;

    unsigned int i = cursor->index;
    unsigned char write_count = cursor->skip;
    rle* data = cursor->src;

    for(; count && i < data->count; i++) {
        pair = data->pairs[i];
        data_count = pair.count;
        write_count = data_count - write_count;
        if(count < write_count) {
            write_count = count;
            cursor->skip += write_count;
        }
        else {
            cursor->skip = 0;
        }

        memset(dest, pair.byte, write_count);

        dest += write_count;
        count -= write_count;

        write_count = 0;
    }

    cursor->index = i;
    if(cursor->skip) {
        cursor->index--;
    }

    if(i < data->count) return EXIT_SUCCESS;

    free(data);
    free(cursor);

    return EXIT_SUCCESS;
}
