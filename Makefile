OUTPUT=machismo.prg
OUTPUTPATH=build/$(OUTPUT)
all: build crunch copy

build:
	mkdir -p build/
	cl65 -t c64 -o $(OUTPUTPATH) -O code/main.c resources/text.s

crunch:
	echo nope

copyclean:
	rm -i /mnt/chromeos/PlayFiles/roms/$(OUTPUT) || exit 0

copy: copyclean
	cp build/machismo.prg /mnt/chromeos/removable/Chromebook/user/roms/$(OUTPUT)

run: all
	x64 $(OUTPUTPATH)
