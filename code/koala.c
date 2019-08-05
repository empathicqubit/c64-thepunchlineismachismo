#include <stdio.h>
#include <conio.h>
#include "c64.h"

#define poke(A,X) (*(unsigned char *)A) = (X)
#define peek(A) (*(unsigned char *)A)

int koala_load (char filename[]) {
  unsigned char addr[2];
  unsigned char* bitmap = 0x2000;
  unsigned char* screen = 0x0400;
  unsigned char* color = 0xd800;
  unsigned char* bg = 0xd021;

  /* open the file */
  FILE* fp;
  fp = fopen(filename, "rb");

  fread(addr, 2, 1, fp);

  /* make sure load address is $4400 or $6000 */
  if (addr[0] != 0 || (addr[1] != 0x44 && addr[1] != 0x60)) {
    fclose(fp);
    return 1;
  }

  if(!(
      /* load bitmap data */
      fread(bitmap, 200, 8000 / 200, fp)
      /* load screen data */
      && fread(screen, 200, 1000 / 200, fp)
      /* load colour ram */
      && fread(color, 200, 1000 / 200, fp)
      /* load background colour into $d021 */
      && fread(bg, 1, 1, fp)
  )) {
      fclose(fp);
      return 1;
  }

  poke(VIC_CTRL1, 0x3b); /* enable bitmap mode */
  poke(VIC_CTRL2, 0x18); /* enable multicolour */
  poke(VIC_VIDEO_ADR, 0x1f); /* screen at $0400 bitmap at $2000 */
  poke(VIC_BORDERCOLOR, 0x00); /* black border */

  return 0;
}

