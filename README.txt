SuperTux Milestone 2 (0.3.x) for Symbian S60 3rd Edition - work in progress
Kept separate from the Milestone 1 port in S:\supertux_symbian_src.

libs\tremor          Integer-only Ogg Vorbis decoder (Tremor) + libogg, as tremor.lib
libs\sdl_mixer_ogg   SDL_mixer with Ogg, built from S:\official\SDL_mixer-release-1.2.12 as sdl_mixer_ogg.lib
libs\physfs          PhysicsFS 2.0.3 (file access, zip archives), as physfs.lib
tests\oggtest        Plays an Ogg file: checks the audio layer
tests\physfstest     Reads files and a zip through PhysicsFS, then plays an Ogg from the zip

sdl\group            SuperTux 2's own SDL: st2sdl.dll + st2sdlexe.dll, compiled from
                     S:\official\SDL-1.2.14 under new names and UIDs (0xA000A031, 0xA000A032)
                     so it never clashes with the sdl.dll of SuperTux 0.1.3.
                     fix_st2sdl.pl repairs st2sdl.dll's static data after every fresh link.
package              Game data zips + add-ons\ for the full package
game\symbian         supertux2.pkg (test, no data) and supertux2_full.pkg (everything)

build_libs.bat       Builds the libraries
build_supertux2.bat  Builds SDL, icon, game and the test package supertux2.sis
make_release2.bat    Signed all-in-one package in S:\supertux_release
make_split2.bat      Signed split packages (program + 4 data) zipped, for phones that refuse the big one
make_release_r1.bat  Builds both release variants (no log / debug log), full and split

Rules learned so far:
- Music must be loaded into memory before playing. Streaming it from a file
  fails silently on Symbian (the audio thread cannot read the file).
