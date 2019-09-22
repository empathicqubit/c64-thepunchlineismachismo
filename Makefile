SHELL=busybox
.SHELLFLAGS=sh -e -c

D81=machismo.d81
SID_HIGHBYTE=B0
BITMAP_START=E000
SCREEN_START=C000
PAL_ROM ?=

CC65_VERSION=V2.18

.ONESHELL:

all: build

build: $(D81)

run: $(D81)
	SOMMELIER=$$(which sommelier && echo -n " --scale=0.5 --x-display=:0" || echo) && echo $$SOMMELIER && $$SOMMELIER x64 -moncommands ./moncommands.vice -userportdac +VICIIdsize -VICIIfilter 0 -model $(if $(PAL_ROM),pal,ntsc) -iecdevice8 -sidenginemodel 256 -residsamp 0 $<

dm: ./docker
	docker-compose run build

multipaint:
	cd tools/multipaint/application.linux64
	./multipaint

clean:
	rm -rf resources/generated/
	rm -rf machismo.prg
	rm -rf docker
	rm -rf $(D81)
	rm -rf empty.d81
	find . -iname '*.o' -delete
	rm -rf *.pem
	rm -rf machismo.lbl
	rm -rf $(music)
	rm -rf resources/audio/*.snz
	rm -rf $(sounds)

cp-emu: /mnt/chromeos/removable/Chromebook/user/roms/machismo.d81

cp-c64: /mnt/chromeos/removable/C64/machismo.d81

/mnt/chromeos/removable/Chromebook/user/roms/machismo.d81: $(D81)
	cp $< $@

/mnt/chromeos/removable/C64/machismo.d81: $(D81)
	cp $< $@

sprites := $(wildcard resources/sprites/*.spd)

music := $(patsubst %.sng,%.sid,$(wildcard resources/audio/*.sng))

sounds := $(sort $(patsubst %.ins,%.snd,$(wildcard resources/audio/*.ins)))

sound_sizes=$(foreach sound,$(sounds),$(shell stat -c '%s' "$(sound)"))

charset := $(wildcard resources/charset/*.s)

# Decided to use OCP since Multipaint supports it natively.
# I wanted to use PNGs since they are well supported, but
# I prefer having a simple toolset instead.
bitmaps := $(wildcard resources/bitmap/*.ocp)

code := $(wildcard code/*.c) $(wildcard code/*.s)

machismo.d81: empty.d81 machismo.prg $(music) $(bitmaps) $(sprites) resources/audio/canada.snz
	# Writes all files that have changed.
	c1541 -attach $@ $(foreach content,$(filter-out $<,$?), -delete $(notdir $(content)) -write $(content) $(notdir $(content)))

empty.d81:
	c1541 -format "canada,01" d81 $@
	test ! -f "$(D81)" && cp empty.d81 $(D81) || exit 0

machismo.prg: linker.cfg $(code) resources/text.s $(charset) $(music)
	sidsize=$$(stat -c'%s' $(music) | sort -nr | head -1) 
	echo "SID SIZE $$sidsize"
	cl65 -Osr -g -t c64 -C linker.cfg -Wa "-DSID_START=\$$$(SID_HIGHBYTE)00" -Wc "-DBITMAP_START=0x$(BITMAP_START)" -Wc "-DSCREEN_START=0x$(SCREEN_START)" "-DSID_START=0x$(SID_HIGHBYTE)00" -Wc "-DSID_SIZE=$$sidsize" -Wl "-Lnmachismo.lbl" -o $@ -O $(filter %.c %.s,$^)

resources/audio/canada.snz: $(sounds)
	sound_header="\x$$(printf '%x' $(words $(sounds)))"
	size_total=0
	# This is not okay, sh.
	for each in $(sound_sizes) ; do
	    sound_header="$${sound_header}\x$$(printf '%x' "$$size_total")"
	    size_total=$$((size_total + each))
	done
	printf "$$sound_header" | cat - $(sounds) > "$@"

%.snd: %.ins
	ins2snd2 "$<" "$@" -b

%.sid: %.sng
	# You need to set the correct extension otherwise the output format will be SIDPlay!!!
	gt2reloc $< $@.bin -N -W$(SID_HIGHBYTE) -B1 -D1 -ZFB -C0 -E0 -H0 -R1
	mv $@.bin $@

./docker: ./docker/cert.pem ./docker/cc65.tar.gz ./docker/goattracker.zip

./docker/cert.pem: ./docker/.sentinel
	{ security find-certificate -a -c " $$COMPANYNAME " -p || echo "" } > ./docker/cert.pem

./docker/cc65.tar.gz: ./docker/.sentinel
	wget $$(test -n "$$IGNORE_SSL" && echo "--no-check-certificate" || echo "") -O "$@" https://github.com/cc65/cc65/archive/${CC65_VERSION}.tar.gz || rm "$@"

./docker/goattracker.zip: ./docker/.sentinel
	wget $$(test -n "$$IGNORE_SSL" && echo "--no-check-certificate" || echo "") -O "$@" http://csdb.dk/getinternalfile.php/180091/GoatTracker_2.75.zip || rm "$@"

# A single pattern rule will create all appropriate folders as required
.PRECIOUS: %/.sentinel # otherwise make (annoyingly) deletes it
%/.sentinel:
	 mkdir -p ${@D}
	 touch $@
