# c64-thepunchlineismachismo

A Commodore game about the Canadian military's random bootleg macho man.

# Prerequisites (build/run)

- VICE emulator (Required to run **and** build) (c1541, x64sc)
- cc65 (cl65, sp65)
- Goattracker (gt2reloc, ins2snd2. Had to build from source because distribution didn't have this binary!)

# Additional dev tools
- Multipaint -- To draw with the C64 restrictions in mind
- ppolizei -- Not necessary but helpful to convert images that aren't already C64 friendly.
- Spritemate (preferably the offline electron version) -- To draw sprites and save them in the handy SpritePad sprite sheet format. Stores sprite info like multicolor mode and sprite color in the 64th byte, and has a header to track the multicolor1&2 for the sheet.
