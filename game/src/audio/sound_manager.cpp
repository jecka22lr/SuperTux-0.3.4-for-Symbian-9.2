//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

// Symbian port: SDL_mixer implementation of the sound manager (see sound_manager.hpp).

#include "audio/sound_manager.hpp"

#include <SDL.h>
#include <SDL_mixer.h>
#include <physfs.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <memory>

#include "audio/dummy_sound_source.hpp"
#include "audio/mixer_sound_source.hpp"
#include "lisp/lisp.hpp"
#include "lisp/parser.hpp"
#include "util/file_system.hpp"
#include "util/log.hpp"

/* Phones mix in mono at 22 kHz: half the work of stereo, and the speakers barely care */
#ifndef SUPERTUX_AUDIO_RATE
#define SUPERTUX_AUDIO_RATE 22050
#endif
#ifndef SUPERTUX_AUDIO_CHANNELS
#define SUPERTUX_AUDIO_CHANNELS 1
#endif
#define SUPERTUX_MIX_CHANNELS 16

namespace {

/** Read a whole file through PhysicsFS; caller frees with free() */
char* read_whole_file(const std::string& filename, long* size)
{
  PHYSFS_file* file = PHYSFS_openRead(filename.c_str());
  if(!file) {
    log_warning << "Couldn't open '" << filename << "': " << PHYSFS_getLastError() << std::endl;
    return NULL;
  }
  long length = (long) PHYSFS_fileLength(file);
  char* data = (char*) malloc(length > 0 ? length : 1);
  if(data != NULL && PHYSFS_read(file, data, 1, length) != length) {
    log_warning << "Couldn't read '" << filename << "': " << PHYSFS_getLastError() << std::endl;
    free(data);
    data = NULL;
  }
  PHYSFS_close(file);
  *size = length;
  return data;
}

bool ends_with(const std::string& s, const char* suffix)
{
  size_t n = strlen(suffix);
  return s.length() > n && s.compare(s.length() - n, n, suffix) == 0;
}

/* SDL_mixer 1.2.12 hands Tremor seconds where Tremor expects milliseconds */
void set_music_position(float seconds)
{
#ifdef OGG_USE_TREMOR_SEEK_BUG
  Mix_SetMusicPosition(seconds * 1000.0);
#else
  Mix_SetMusicPosition(seconds);
#endif
}

} // namespace

SoundManager::SoundManager() :
  audio_open(false),
  sound_enabled(false),
  music_enabled(false),
  chunks(),
  managed_sources(),
  listener_position(),
  current_music(),
  music(0),
  music_data(0),
  music_loop_begin(0),
  music_loop_at(-1),
  music_started(0),
  music_start_pos(0)
{
  if(Mix_OpenAudio(SUPERTUX_AUDIO_RATE, AUDIO_S16SYS, SUPERTUX_AUDIO_CHANNELS, 2048) < 0) {
    log_warning << "Couldn't open audio: " << Mix_GetError() << std::endl;
    log_warning << "Disabling sound and music." << std::endl;
    return;
  }
  Mix_AllocateChannels(SUPERTUX_MIX_CHANNELS);
  audio_open = true;
  sound_enabled = true;
  music_enabled = true;
}

SoundManager::~SoundManager()
{
  for(ManagedSources::iterator i = managed_sources.begin(); i != managed_sources.end(); ++i)
    delete *i;
  managed_sources.clear();

  free_music();

  if(audio_open)
    Mix_HaltChannel(-1);
  for(Chunks::iterator i = chunks.begin(); i != chunks.end(); ++i) {
    if(i->second)
      Mix_FreeChunk(i->second);
  }
  chunks.clear();

  if(audio_open)
    Mix_CloseAudio();
}

Mix_Chunk*
SoundManager::get_chunk(const std::string& filename)
{
  Chunks::iterator i = chunks.find(filename);
  if(i != chunks.end())
    return i->second;

  Mix_Chunk* chunk = NULL;
  long size = 0;
  char* data = read_whole_file(filename, &size);
  if(data != NULL) {
    chunk = Mix_LoadWAV_RW(SDL_RWFromMem(data, size), 1);  /* decodes into its own buffer */
    if(chunk == NULL)
      log_warning << "Couldn't decode sound '" << filename << "': " << Mix_GetError() << std::endl;
    free(data);
  }
  chunks[filename] = chunk;  /* remember failures too, so we don't retry every time */
  return chunk;
}

void
SoundManager::preload(const std::string& filename)
{
  if(!audio_open)
    return;
  get_chunk(filename);
}

SoundSource*
SoundManager::create_sound_source(const std::string& filename)
{
  if(!audio_open)
    return create_dummy_sound_source();

  Mix_Chunk* chunk = get_chunk(filename);
  if(chunk == NULL)
    return create_dummy_sound_source();
  return new MixerSoundSource(this, chunk);
}

void
SoundManager::play(const std::string& filename, const Vector& pos)
{
  if(!sound_enabled)
    return;

  Mix_Chunk* chunk = get_chunk(filename);
  if(chunk == NULL)
    return;

  int channel = Mix_PlayChannel(-1, chunk, 0);
  if(channel < 0)
    return;  /* all channels busy */

  float gain = 1.0f;
  if(pos.x >= 0 && pos.y >= 0)
    gain = distance_gain(pos, 128);
  Mix_Volume(channel, (int) (gain * MIX_MAX_VOLUME));
}

void
SoundManager::manage_source(SoundSource* source)
{
  assert(source != NULL);
  managed_sources.push_back(source);
}

void
SoundManager::register_for_update(StreamSoundSource* )
{
}

void
SoundManager::remove_from_update(StreamSoundSource* )
{
}

void
SoundManager::enable_sound(bool enable)
{
  if(!audio_open)
    return;

  sound_enabled = enable;
  if(!enable)
    Mix_HaltChannel(-1);
}

void
SoundManager::enable_music(bool enable)
{
  if(!audio_open)
    return;

  music_enabled = enable;
  if(music_enabled) {
    std::string name = current_music;
    current_music = "";
    play_music(name);
  } else {
    std::string name = current_music;
    free_music();
    current_music = name;  /* remember it for when music is switched back on */
  }
}

void
SoundManager::free_music()
{
  if(music) {
    Mix_HaltMusic();
    Mix_FreeMusic(music);
    music = 0;
  }
  if(music_data) {
    free(music_data);
    music_data = 0;
  }
}

bool
SoundManager::load_music(const std::string& filename)
{
  std::string ogg_file = filename;
  music_loop_begin = 0;
  music_loop_at = -1;

  if(ends_with(filename, ".music")) {
    /* (supertux-music (file "x.ogg") (loop-begin s) (loop-at s)) */
    try {
      lisp::Parser parser(false);
      const lisp::Lisp* root = parser.parse(filename);
      const lisp::Lisp* desc = root->get_lisp("supertux-music");
      if(desc == NULL) {
        log_warning << "'" << filename << "' is not a supertux-music file" << std::endl;
        return false;
      }
      std::string raw;
      desc->get("file", raw);
      desc->get("loop-begin", music_loop_begin);
      desc->get("loop-at", music_loop_at);
      if(music_loop_begin < 0)
        music_loop_begin = 0;
      ogg_file = FileSystem::normalize(FileSystem::dirname(filename) + raw);
    } catch(std::exception& e) {
      log_warning << "Couldn't read '" << filename << "': " << e.what() << std::endl;
      return false;
    }
  }

  long size = 0;
  music_data = read_whole_file(ogg_file, &size);
  if(music_data == NULL)
    return false;

  music = Mix_LoadMUS_RW(SDL_RWFromConstMem(music_data, size));
  if(music == NULL) {
    log_warning << "Couldn't load music '" << ogg_file << "': " << Mix_GetError() << std::endl;
    free(music_data);
    music_data = 0;
    return false;
  }
  return true;
}

void
SoundManager::play_music(const std::string& filename, bool fade)
{
  if(filename == current_music && music != 0)
    return;
  current_music = filename;
  if(!music_enabled)
    return;

  free_music();
  if(filename == "")
    return;

  if(!load_music(filename))
    return;

  /* played once; update() takes care of looping, from loop-begin like the original */
  if(fade)
    Mix_FadeInMusic(music, 0, 500);
  else
    Mix_PlayMusic(music, 0);
  music_started = SDL_GetTicks();
  music_start_pos = 0;
}

void
SoundManager::stop_music(float fadetime)
{
  current_music = "";
  if(!music)
    return;

  if(fadetime > 0)
    Mix_FadeOutMusic((int) (fadetime * 1000));  /* freed in update() once it has faded */
  else
    free_music();
}

void
SoundManager::set_listener_position(const Vector& pos)
{
  listener_position = pos;
}

void
SoundManager::set_listener_velocity(const Vector& )
{
  /* no doppler effect */
}

float
SoundManager::distance_gain(const Vector& pos, float reference_distance) const
{
  /* OpenAL's default model: full volume within the reference distance,
     then falling off with 1/distance */
  float dx = pos.x - listener_position.x;
  float dy = pos.y - listener_position.y;
  float distance = sqrtf(dx * dx + dy * dy);
  if(distance <= reference_distance || reference_distance <= 0)
    return 1.0f;
  return reference_distance / distance;
}

void
SoundManager::update()
{
  if(!audio_open)
    return;

  /* delete managed sources that have finished */
  for(ManagedSources::iterator i = managed_sources.begin(); i != managed_sources.end(); ) {
    if(!(*i)->playing()) {
      delete *i;
      i = managed_sources.erase(i);
    } else {
      ++i;
    }
  }

  if(!music)
    return;

  if(current_music == "") {
    /* stop_music() with a fade: free it once the fade is over */
    if(!Mix_PlayingMusic())
      free_music();
    return;
  }

  Uint32 now = SDL_GetTicks();
  float position = music_start_pos + (now - music_started) / 1000.0f;

  if(!Mix_PlayingMusic() || (music_loop_at > 0 && position >= music_loop_at)) {
    /* loop: back to loop-begin */
    if(!Mix_PlayingMusic())
      Mix_PlayMusic(music, 0);
    if(music_loop_begin > 0)
      set_music_position(music_loop_begin);
    else if(Mix_PlayingMusic())
      Mix_RewindMusic();
    music_started = now;
    music_start_pos = music_loop_begin;
  }
}

/* EOF */
