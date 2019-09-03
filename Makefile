D81=machismo.d81
SID_HIGHBYTE=B0
BITMAP_START=E000
SCREEN_START=C000

all: build

build: $(D81)

run: $(D81)
	SOMMELIER=$$(which sommelier && echo -n " --scale=0.5 --x-display=:0" || echo) && echo $$SOMMELIER && $$SOMMELIER x64 +VICIIdsize -VICIIfilter 0 -model ntsc -iecdevice8 -sidenginemodel 256 -residsamp 0 $<

clean:
	rm -rf resources/generated/
	rm -rf machismo.prg
	rm -rf $(D81)
	rm -rf empty.d81
	find . -iname '*.o' -delete
	rm -rf machismo.lbl
	rm -rf $(audio)

copy-emu: /mnt/chromeos/removable/Chromebook/user/roms/machismo.d81

copy-c64: /mnt/chromeos/removable/C64/machismo.d81

/mnt/chromeos/removable/Chromebook/user/roms/machismo.d81: $(D81)
	cp $< $@

/mnt/chromeos/removable/C64/machismo.d81: $(D81)
	cp $< $@

sprites := $(wildcard resources/sprites/*.spd)

audio := $(patsubst %.sng,%.sid,$(wildcard resources/audio/*.sng))

charset := $(wildcard resources/charset/*.s)

# Generate these with a rule if possible. Poke at polizei JAR?
bitmaps := $(patsubst %.png,%.koa,$(wildcard resources/bitmap/*.png))

code := $(wildcard code/*.c) $(wildcard code/*.s)

machismo.d81: empty.d81 machismo.prg $(audio) $(bitmaps) $(sprites)
	# Writes all files that have changed.
	c1541 -attach $@ $(foreach content,$(filter-out $<,$?), -delete $(notdir $(content)) -write $(content) $(notdir $(content)))

empty.d81:
	c1541 -format "canada,01" d81 $@
	test ! -f "$(D81)" && cp empty.d81 $(D81) || exit 0

machismo.prg: linker.cfg $(code) resources/text.s $(charset) $(audio)
	sidsize=$$(stat -c'%s' $(audio) | sort -nr | head -1) && echo "SID SIZE $$sidsize" && cl65 -g -t c64 -C linker.cfg -Wa "-DSID_START=\$$$(SID_HIGHBYTE)00" -Wc "-DBITMAP_START=0x$(BITMAP_START)" -Wc "-DSCREEN_START=0x$(SCREEN_START)" "-DSID_START=0x$(SID_HIGHBYTE)00" -Wc "-DSID_SIZE=$$sidsize" -Wl "-Lnmachismo.lbl" -o $@ -O $(filter %.c %.s,$^)

%.sid: %.sng
	# You need to set the correct extension otherwise the output format will be SIDPlay!!!
	gt2reloc $< $@.bin -N -W$(SID_HIGHBYTE) -B1 -D1 -ZFB -C0 -E0 -H0
	mv $@.bin $@
