Tremor for Symbian S60 3rd Edition

Integer-only Ogg Vorbis decoder (Xiph.Org libvorbisidec 1.2.1) with the
libogg 1.3.2 bitstream code, both BSD-licensed (see ..\COPYING and ..\ogg\COPYING).
Taken from SDL_mixer 2.0.4's external sources.

Symbian changes:
- include\ogg\os_types.h: integer types for Symbian.
- os.h, misc.h: little endian, GCC builtin alloca, no <endian.h>.
- sharedbook.c: codebook setup buffers on the heap instead of the stack
  (Symbian thread stacks are small).
