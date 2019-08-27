#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "c64.h"

unsigned char koala_load (char filename[]) {
  unsigned char addr[2];
  FILE* fp;

  /* open the file */
  fp = fopen(filename, "rb");

  if(!fread(addr, 2, 1, fp)) {
    fclose(fp);
    return EXIT_FAILURE;
  }

  /* make sure load address is $4400 or $6000 */
  if (addr[0] != 0 || (addr[1] != 0x44 && addr[1] != 0x60)) {
    fclose(fp);
    return EXIT_FAILURE;
  }

  *(unsigned char *)VIC_CTRL1 |= VIC_CTRL1_BITMAP_ON;
  *(unsigned char *)VIC_CTRL2 |= VIC_CTRL2_MULTICOLOR_ON;

  *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_CHAR_PTR_MASK);
  *(unsigned char *)VIC_VIDEO_ADR |= ((BITMAP_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_CHAR_DIVISOR) << 1;

  if(!(
      fread(BITMAP_START, 200, 8000 / 200, fp)
      && fread(SCREEN_START, 200, 1000 / 200, fp)
      && fread(COLOR_RAM, 200, 1000 / 200, fp)
      && fread(VIC_BG_COLOR0, 1, 1, fp)
  )) {
      fclose(fp);
      return EXIT_FAILURE;
  }

  fclose(fp);
  return EXIT_SUCCESS;
}

