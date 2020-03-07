#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "rle.h"
#include "c64.h"

unsigned char ocp_load (char filename[]) {
  unsigned char addr[2];
  unsigned char err;
  unsigned int unpacked_size;
  rle_cursor* rle;

  if(!(rle = rle_open(filename, &unpacked_size))) {
      return EXIT_FAILURE;
  }

  if(err = rle_unpack(rle, addr, 2)) {
    rle_close(rle);
    return err;
  }

  // make sure load address is $2000
  if (addr[0] != 0x00 || addr[1] != 0x20) {
    rle_close(rle);
    return EXIT_FAILURE;
  }

  *(unsigned char *)VIC_CTRL1 |= VIC_CTRL1_BITMAP_ON;
  *(unsigned char *)VIC_CTRL2 |= VIC_CTRL2_MULTICOLOR_ON;

  *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_CHAR_PTR_MASK);
  *(unsigned char *)VIC_VIDEO_ADR |= ((BITMAP_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_CHAR_DIVISOR) << 1;

  if(
      (err = rle_unpack(rle, BITMAP_START, SCREEN_BITMAP_SIZE))
      || (err = rle_unpack(rle, SCREEN_START, SCREEN_BYTES))
      || (err = rle_unpack(rle, VIC_BORDERCOLOR, 1))
      || (err = rle_unpack(rle, VIC_BG_COLOR0, 1))
      || (err = rle_unpack(rle, COLOR_RAM, 14)) // This is trash
      || (err = rle_unpack(rle, COLOR_RAM, COLOR_RAM_SIZE))
  ) {
      rle_close(rle);
      return err;
  }

  rle_close(rle);

  return EXIT_SUCCESS;
}
