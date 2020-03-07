#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "rle.h"
#include "c64.h"

/** Load a SEQ file, a text file in PETSCII format.
 * Bear in mind this is different from screen codes!
 * @param filename The filename of the RLE packed SEQ file
 * @param size The size of the returned data
 * @return The file contents
 */
unsigned char* seq_load (char filename[], unsigned int* size) {
  unsigned char check, err;
  unsigned char* data;
  unsigned int unpacked_size;
  rle_cursor* rle;

  if(!(rle = rle_open(filename, &unpacked_size))) {
      return NULL;
  }

  if(err = rle_unpack(rle, &check, 1)) {
    rle_close(rle);
    return NULL;
  }

  // make sure first byte is clear screen
  if (check != 0x93) {
    rle_close(rle);
    return NULL;
  }

  if(!(data = malloc(unpacked_size))) {
    return NULL;
  }

  data[0] = check;

  if(
      (err = rle_unpack(rle, &data[1], unpacked_size - 1))
  ) {
      free(data);
      rle_close(rle);
      return NULL;
  }

  rle_close(rle);

  *size = unpacked_size;

  return data;
}