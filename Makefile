D81=machismo.d81
SID_HIGHBYTE=80

all: build

build: $(D81)

run: $(D81)
	x64sc -model ntsc -iecdevice8 $<

clean:
	rm -rf resources/generated/
	rm -rf machismo.prg
	rm -rf $(D81)
	rm -rf empty.d81
	find . -iname '*.o' -delete
	rm -rf $(sprites)
	rm -rf $(audio)

copy-emu: /mnt/chromeos/removable/Chromebook/user/roms/machismo.d81

copy-c64: /mnt/chromeos/removable/C64/machismo.d81

/mnt/chromeos/removable/Chromebook/user/roms/machismo.d81: $(D81)
	cp $< $@

/mnt/chromeos/removable/C64/machismo.d81: $(D81)
	cp $< $@

sprites := $(patsubst %.pcx,%.sprite.s,$(wildcard resources/sprites/*.pcx))

audio := $(patsubst %.sng,%.sid,$(wildcard resources/audio/*.sng))

machismo.d81: empty.d81 machismo.prg $(audio)
	# Writes all files that have changed.
	c1541 -attach $@ $(foreach content,$(filter-out $<,$?), -delete $(notdir $(content)) -write $(content) $(notdir $(content)))

empty.d81:
	c1541 -format "canada,01" d81 $@
	cp -n empty.d81 $(D81)

machismo.prg: linker.cfg code/main.c code/sid.s resources/text.s $(sprites) $(audio)
	sidsize=$$(du -b -s $(audio) | sort -nr | head -1 | awk '{print $$1}') && cl65 -t c64 -C linker.cfg -Wc "-DSID_SIZE=$$sidsize" -Wl "-D__SIDADDR__=\$$$(SID_HIGHBYTE)00 -D__SIDMEM__=$$sidsize" -o $@ -O $(filter %.c %.s,$^)

%.sid: %.sng
	# You need to set the correct extension otherwise the output format will be SIDPlay!!!
	gt2reloc $< $@.bin -N -W$(SID_HIGHBYTE) -B1 -D1 -ZFB -C0 -E0 -H0
	mv $@.bin $@

%.sprite.s: %.pcx
	sp65 -r $<:pcx -c vic2-sprite -w $(basename $@).bin:bin
	echo '.export _r_sprites_$(basename $(notdir $<))' > $@
	echo '_r_sprites_$(basename $(notdir $<)): .incbin "$@"' >> $@

