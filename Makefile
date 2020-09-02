SHELL=busybox
.SHELLFLAGS=sh -e -c

DISKIMAGE=machismo.d64
SID_HIGHBYTE=B0
BITMAP_START=E000
SPRITE_START=C400
SCREEN_START=C000
CHARACTER_START=D800
MODEL?=ntsc

CC=cl65

SFX=1
ifdef SFX
LINKER_CFG=linker_sfx.cfg
else
LINKER_CFG=linker.cfg
endif

CFLAGS=-O -Osr -t c64 -C "$(LINKER_CFG)"
DFLAGS?=-Wa "--cpu" -Wa "6502X" -Wa "-DSID_START=\$$$(SID_HIGHBYTE)00" -Wc "-DSID_START=0x$(SID_HIGHBYTE)00"  -Wc "-DSID_SIZE=$$sidsize" -Wc "-DBITMAP_START=0x$(BITMAP_START)" -Wc "-DSCREEN_START=0x$(SCREEN_START)" -Wc "-DCHARACTER_START=0x$(CHARACTER_START)" -Wc "-DSPRITE_START=0x$(SPRITE_START)"
DBGFLAGS?=-g -Wl "-Lnbuild/machismo.lbl" -vm -Wl "--mapfile,build/machismo.map" -Wl "--dbgfile,build/machismo.dbg"

CCFLAGS=$(CFLAGS) $(DFLAGS) $(DBGFLAGS)

GT2RELOC_OPTS=-P -W$(SID_HIGHBYTE) -B1 -D1 -ZFB -C0 -E0 -H0 -R1

CC65_VERSION=V2.18

.ONESHELL:

all: build

build: build/$(DISKIMAGE)

run: build/$(DISKIMAGE)
		SOMMELIER=$$(which sommelier && echo -n " --scale=1" || echo)
		echo $$SOMMELIER
		$$SOMMELIER $$(which x64sc x64 | head -1) -directory '$$$$' -moncommands ./moncommands.vice +VICIIdsize -rsuser -rsuserdev 2 -rsdev3baud 2400 -rsuserbaud 2400 -rsdev3ip232 -VICIIfilter 0 -model $(MODEL) -iecdevice8 -autostart-warp -nativemonitor -remotemonitor -remotemonitoraddress 127.0.0.1:2332 -raminitrandomchance 0xff -sidenginemodel 256 -sound -autostart-handle-tde -residsamp 0 -autostart "$<"

dm: ./docker
		docker-compose run build

mp:
		cd tools/multipaint/application.linux64
		./multipaint

gt:
		goattracker -P

clean:
		rm -rf docker
		rm -rf build
		find code -iname '*.o' -exec rm -rf {} \;
		rm -rf $(music)
		rm -rf $(bitmaps)
		rm -rf *.ocb *.ocs
		rm -rf resources/audio/*.snz
		rm -rf $(sounds)
		rm -rf $(seq)
		cd tools/exo/src
		make clean

cp-emu: /mnt/chromeos/removable/Chromebook/user/roms/$(DISKIMAGE)

cp-c64: /mnt/chromeos/removable/C64/$(DISKIMAGE)

/mnt/chromeos/removable/Chromebook/user/roms/machismo.d64: build/$(DISKIMAGE)
		cp "$<" "$@"

/mnt/chromeos/removable/C64/machismo.d64: build/$(DISKIMAGE)
		cp "$<" "$@"

sprites := $(wildcard resources/sprites/*.spd)

seq := $(patsubst %.seq,%.sex,$(wildcard resources/seq/*.seq))

ntsc_music := $(patsubst %.sng,%.sid,$(wildcard resources/audio/*.sng))

pal_music := $(patsubst %.sid,%.sidp,$(ntsc_music))

music := $(ntsc_music) $(pal_music)

sounds := $(sort $(patsubst %.ins,%.snd,$(wildcard resources/audio/*.ins)))

sound_sizes=$(foreach sound,$(sounds),$(shell stat -c '%s' "$(sound)"))

charset := $(wildcard resources/charset/*.s)

# Decided to use OCP since Multipaint supports it natively.
# I wanted to use PNGs since they are well supported, but
# I prefer having a simple toolset instead.
bitmaps := $(patsubst %.ocp,%.ocx,$(wildcard resources/bitmap/*.ocp))

code := $(wildcard code/*.c) $(wildcard code/*_asm.s) resources/sprites/canada.c

build/machismo.d64: build/empty.d64 build/machismo.prg $(music) $(patsubst %.ocx,%.ocb,$(bitmaps)) $(patsubst %.ocx,%.ocs,$(bitmaps)) $(sprites) $(seq) resources/audio/canada.snz build/.sentinel
		# Writes all files that have changed.
		c1541 -attach $@ $(foreach content,$(filter-out $<,$?), -delete $(notdir $(content)) -write $(content) $(notdir $(content)))

build/empty.d64: build/.sentinel
		c1541 -format "canada,01" d64 "$@"
		test ! -f "build/$(DISKIMAGE)" && cp "$@" "build/$(DISKIMAGE)" || exit 0

build/machismo.prg: build/exomizer build/precrunch.prg
ifdef SFX
	"$<" sfx 0x0401 -x3 "$(filter-out $<,$^)" -o "$@"
else
	cp "$(filter-out $<,$^)" "$@"
endif

build/precrunch.prg: build/.sentinel $(LINKER_CFG) $(code) resources/text.s $(charset) $(music)
		sidsize=$$(stat -c'%s' $(music) | sort -nr | head -1)
		echo "SID SIZE $$sidsize"

		$(CC) $(CCFLAGS) -o "$@" $(filter %.c %.s,$^)
		$(CC) $(CCFLAGS) -S $(filter %.c,$^)

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

%.sidp: %.sng
		# PAL
		gt2reloc "$<" "$@.bin" $(GT2RELOC_OPTS)
		mv "$@.bin" "$@"

%.sid: %.sng
		# You need to set the correct extension otherwise the output format will be SIDPlay!!!
		# NTSC
		gt2reloc "$<" "$@.bin" $(GT2RELOC_OPTS) -G424
		mv "$@.bin" "$@"

build/exomizer: tools/exo/src/exomizer build/.sentinel
		cp "$<" "$@"

tools/exo/src/exomizer: tools/exo/src/Makefile
		cd "$(dir $<)"
		make -j$$(nproc)

%.ocb: build/exomizer %.ocp
		dd "if=$(filter-out $<,$^)" skip=2 bs=1 count=8000 > "$@.tmp"
		"$<" level -c "$@.tmp@0" -o "$@"
		rm "$@.tmp"

%.ocs: build/exomizer %.ocp
		dd "if=$(filter-out $<,$^)" bs=1 skip=8002 count=2016 > "$@.tmp"
		"$<" level -c "$@.tmp@0" -o "$@"
		rm "$@.tmp"

%.ocx: %.ocs %.ocb

%.sex: build/exomizer %.seq
		"$<" level -c "$(filter-out $<,$^)@0" -o "$@"

./docker: ./docker/Dockerfile ./docker/cert.pem ./docker/cc65.tar.gz ./docker/goattracker.zip

./docker/Dockerfile: ./docker/.sentinel
		cp ./Dockerfile "$@"

./docker/cert.pem: ./docker/.sentinel
		{ security find-certificate -a -c " $$COMPANYNAME " -p || echo "" ; } > ./docker/cert.pem

./docker/cc65.tar.gz: ./docker/.sentinel
		curl -L 'https://github.com/cc65/cc65/archive/$(CC65_VERSION).tar.gz' > '$@' || rm "$@"

./docker/goattracker.zip: ./docker/.sentinel
		curl -L 'http://csdb.dk/getinternalfile.php/180091/GoatTracker_2.75.zip' > "$@" || rm "$@"

# A single pattern rule will create all appropriate folders as required
.PRECIOUS: %/.sentinel
%/.sentinel:
		mkdir -p ${@D}
		touch $@
