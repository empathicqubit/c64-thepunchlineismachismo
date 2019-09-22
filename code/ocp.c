#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include "c64.h"

unsigned char ocp_load (char filename[]) {
  unsigned char addr[2];
  FILE* fp;

  /* open the file */
  fp = fopen(filename, "rb");

  if(!fread(addr, 2, 1, fp)) {
    fclose(fp);
    return EXIT_FAILURE;
  }

  /* make sure load address is $2000 */
  if (addr[0] != 0 || addr[1] != 0x20) {
    fclose(fp);
    return EXIT_FAILURE;
  }

  *(unsigned char *)VIC_CTRL1 |= VIC_CTRL1_BITMAP_ON;
  *(unsigned char *)VIC_CTRL2 |= VIC_CTRL2_MULTICOLOR_ON;

  *(unsigned char *)VIC_VIDEO_ADR &= ~(VIC_VIDEO_ADR_CHAR_PTR_MASK);
  *(unsigned char *)VIC_VIDEO_ADR |= ((BITMAP_START % VIC_BANK_SIZE) / VIC_VIDEO_ADR_CHAR_DIVISOR) << 1;

  if(!(
        /* from RECOIL
	bool DecodeOcp!(byte[] content, int contentLength)
		=> contentLength == 10018 && DecodeC64Multicolor(320, content, 2, 0x1f42, 0x233a, content[0x232b]);

	bool DecodeC64Multicolor!(int width, byte[] content, int bitmapOffset, int videoMatrixOffset, int colorOffset, int background)
	{
        */

      fread(BITMAP_START, 200, 8000 / 200, fp)
      && fread(SCREEN_START, 200, 1000 / 200, fp)
      && fread(VIC_BORDERCOLOR, 1, 1, fp)
      && fread(VIC_BG_COLOR0, 1, 1, fp)
      && fread(COLOR_RAM, 14, 1, fp) // This is trash
      && fread(COLOR_RAM, 200, 1000 / 200, fp)
  )) {
      fclose(fp);
      return EXIT_FAILURE;
  }

  fclose(fp);
  return EXIT_SUCCESS;
}
