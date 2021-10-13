# vim: syntax=python
import os

sid_highbyte = 'B0'
screen_start = 'C000'
sprite_start = 'C400'
character_start = 'D800'
bitmap_start = 'E000'

if 'CC65_HOME' in os.environ:
    cc65_home = os.environ['CC65_HOME']
else:
    cc65_home = str(Glob(os.environ['HOME'] + '/.vscode/extensions/entan-gl.cc65-vice*/dist/cc65')[0])

sfx = -1
if sfx == 1:
    linker_cfg=File('linker_sfx.cfg')
elif sfx == 0:
    linker_cfg=File('linker.cfg')
else:
    linker_cfg=File('linker_nosled.cfg')

flags = [
    '--cpu', '6502X', '-g', '-t', 'c64',
]

asflags = [
    '-DSID_START=\\$$'+sid_highbyte+'00', '-g'
]
asflags.extend(flags)

cflags = [
    '-O', '-Osr', '-C', linker_cfg,

    '-Wc', '-DSID_SIZE=1000', '-Wc', '-DBITMAP_START=0x'+bitmap_start,
    '-Wc', '-DSCREEN_START=0x'+screen_start, '-Wc', '-DCHARACTER_START=0x'+character_start,
    '-Wc', '-DSPRITE_START=0x'+sprite_start,

    '-Wc', '--debug-tables', '-Wc', '${SOURCE}.tab',
    '-Wc', '-DDEBUG=1',
]
cflags.extend(flags)

lflags = [
    '-O', '-Osr', '-C', linker_cfg,

    '-Wl', '--mapfile,build/machismo.map', '-Wl', '--dbgfile,build/machismo.dbg',
    '-Wl', '-Lnbuild/machismo.lbl', '-vm',
]
lflags.extend(flags)

gt2reloc_opts = '-P -W%s -B1 -D1 -ZFB -C0 -E0 -H0 -R1' % (sid_highbyte)

exo_path = File('build/exomizer')

sid_builder = Builder(
    action = """
gt2reloc "$SOURCE" "${SOURCE}.bin" %s -G424
echo '00 %s' | xxd -r -p - > "${TARGET}"
cat "${SOURCE}.bin" >> "$TARGET"
rm "${SOURCE}.bin"
""" % (gt2reloc_opts, sid_highbyte),
    suffix = '.sid',
    src_suffix = '.sng',
)

sidp_builder = Builder(
    action = """
gt2reloc "$SOURCE" "${SOURCE}.bin" %s
echo '00 %s' | xxd -r -p - > "${TARGET}"
cat "${SOURCE}.bin" >> "$TARGET"
rm "${SOURCE}.bin"
""" % (gt2reloc_opts, sid_highbyte),
    suffix = '.sidp',
    src_suffix = '.sng',
)

ins_builder = Builder(
    action = """ins2snd2 "$SOURCE" "$TARGET" -b""",
    suffix = '.snd',
    src_suffix = '.ins',
)

def snz_builder_func(target, source, env):
    current_size = 0
    preamble = '%02x' % len(source)
    for item in source:
        preamble += '%02x' % current_size
        current_size += item.get_size()
    env.Execute('''echo "%s" | xxd -r -p - | cat - %s > "%s"''' % (preamble, ' '.join([str(x) for x in source]), str(target[0])))
    return None

snz_builder = Builder(
    action = snz_builder_func,
    suffix = '.snz',
    src_suffix = '.ins',
)

def cut_file(dest, src, start, length):
    src_file = open(src, 'rb')
    src_file.seek(start)
    slice = src_file.read(length)
    src_file.close()
    dest_file = open(dest, 'wb')
    dest_file.write(slice)
    dest_file.close()

def ocb_builder_func(target, source, env):
    tmp_name = File(str(target[0]) + '.tmp')
    cut_file(str(tmp_name), str(source[0]), 2, 8000)
    env.Execute('''
"%s" level -c "%s@0" -o "%s"
''' % (exo_path, tmp_name, target[0]))
    env.Execute(Delete(tmp_name))

ocb_builder = Builder(
    action = ocb_builder_func,
    suffix = '.ocb',
    src_suffix = '.ocp'
)

def ocs_builder_func(target, source, env):
    tmp_name = File(str(target[0]) + '.tmp')
    cut_file(str(tmp_name), str(source[0]), 8002, 2016)
    env.Execute('''
"%s" level -c "%s@0" -o "%s"
''' % (exo_path, tmp_name, target[0]))
    env.Execute(Delete(tmp_name))

ocs_builder = Builder(
    action = ocs_builder_func,
    suffix = '.ocs',
    src_suffix = '.ocp'
)

sequence_builder = Builder(
    action = """
"%s" level -c "${SOURCE}@0" -o "$TARGET"
""" % (exo_path),
    suffix = '.sex',
    src_suffix = '.seq'
)

env = Environment(
    BUILDERS = {
        'SID': sid_builder,
        'SIDP': sidp_builder,
        'Sound': ins_builder,
        'SoundArchive': snz_builder,
        'OCBitmap': ocb_builder,
        'OCScreen': ocs_builder,
        'Sequence': sequence_builder,
    },
    ENV = {
        'PATH': os.environ["PATH"],
        'CC65_HOME': cc65_home,
        'DISPLAY': os.environ['DISPLAY']
    },
    AS = 'ca65',
    ASFLAGS = asflags,
    CC = 'cl65',
    CFLAGS = cflags,
    LINKFLAGS = lflags
)
env.PrependENVPath("PATH", cc65_home + "/bin_linux_x64")
sources = [
    Glob('code/*.c'),
    Glob('code/*_asm.s'),
    'resources/sprites/canada.c'
]
precrunch = env.Program(target=['build/precrunch.prg', 'build/machismo.dbg', 'build/machismo.map', 'build/machismo.lbl'], source=sources)

exomizer_tool = env.Command(target="tools/exo/src/exomizer", source="tools/exo/src/Makefile", action="""
cd "$SOURCE.dir" && make CC=gcc
""")

exomizer = env.Command(target=exo_path, source=exomizer_tool, action=Copy("$TARGET", "$SOURCE"))

if sfx == 1:
    crunch_action = """
"%s" sfx 0x03d3 -x3 "$SOURCE" -o "$TARGET"
""" % (exo_path)
else:
    crunch_action = Copy("$TARGET", "$SOURCE")

crunched = env.Command(target='build/machismo.prg', source=[
    precrunch,
    exomizer
], action=crunch_action)

songs = Glob('resources/audio/*.sng')
sids = []
for song in songs:
    sid = env.SID(source=song)
    sids.append(sid)

    sidp = env.SIDP(source=song)
    sids.append(sidp)

instruments = Glob('resources/audio/*.ins')
snds = []
for ins in instruments:
    snd = env.Sound(target=str(ins)+'.snd', source=ins)
    snds.append(snd)

sound_archive = env.SoundArchive(target='resources/audio/canada.snz', source=snds)

images = Glob('resources/bitmap/*.ocp')
bitmap_parts = []
for image in images:
    ocb = env.OCBitmap(source=[image, exomizer])
    bitmap_parts.append(ocb)

    ocs = env.OCScreen(source=[image, exomizer])
    bitmap_parts.append(ocs)

sequences = Glob('resources/seq/*.seq')
sexes = []
for sequence in sequences:
    sex = env.Sequence(source=[sequence, exomizer])
    sexes.append(sex)

sprites = Glob('resources/sprites/*.spd')

disk_files = []
disk_files.append(crunched)
disk_files.extend(sprites)
disk_files.append(sound_archive)
disk_files.extend(bitmap_parts)
disk_files.extend(sids)
disk_files.extend(sexes)

def disk_func(target, source, env):
    if not target[0].exists():
        env.Execute('c1541 -format "canada,01" d64 "%s"' % target[0])
    changes = []
    for src in source:
        basename = os.path.basename(str(src))
        #if str(src).endswith("spd"):
        #    changes.append(""" -delete 'farts.spd' -write '%s' 'farts.spd'""" % (str(src)))
        #else:
        changes.append(""" -delete '%s' -write '%s' '%s'""" % (basename, str(src), basename))
    env.Execute("""c1541 -attach '%s' %s """ % (str(target[0]), ''.join(changes)))

disk_image = env.Command(target="build/machismo.d64", source=disk_files, action=disk_func)

env.Alias('build', disk_image)
env.Command('mp', [], """cd tools/multipaint/application.linux64 && ./multipaint""")
env.Command('gt', [], """goattracker -P || goattrk2 -P""")

Default(disk_image)
