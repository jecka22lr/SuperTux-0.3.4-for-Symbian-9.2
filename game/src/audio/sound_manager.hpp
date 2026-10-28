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

// Symbian port: the OpenAL sound manager replaced by one built on SDL_mixer.
// Same public interface as the original, so the rest of the game is unchanged.
//   - Sounds (.wav/.ogg) are decoded once into Mix_Chunks and cached.
//   - Music (.ogg, or .music descriptors with loop points) is read through
//     PhysicsFS into memory and played from there: streaming from a file
//     fails silently on Symbian, because SDL's audio thread can't read it.
//   - 3D positioning is approximated with volume falloff by distance.

#ifndef HEADER_SUPERTUX_AUDIO_SOUND_MANAGER_HPP
#define HEADER_SUPERTUX_AUDIO_SOUND_MANAGER_HPP

#include <map>
#include <string>
#include <vector>

#include "math/vector.hpp"

struct Mix_Chunk;
struct _Mix_Music;
class SoundSource;
class StreamSoundSource;

class SoundManager
{
public:
  SoundManager();
  virtual ~SoundManager();

  void enable_sound(bool sound_enabled);
  /**
   * Creates a new sound source object which plays the specified soundfile.
   * You are responsible for deleting the sound source later (this will stop the
   * sound).
   * This function never throws exceptions, but might return a DummySoundSource
   */
  SoundSource* create_sound_source(const std::string& filename);
  /**
   * Convenience function to simply play a sound at a given position.
   */
  void play(const std::string& name, const Vector& pos = Vector(-1, -1));
  /**
   * Adds the source to the list of managed sources (= the source gets deleted
   * when it finished playing)
   */
  void manage_source(SoundSource* source);
  /// preloads a sound, so that you don't get a lag later when playing it
  void preload(const std::string& name);

  void set_listener_position(const Vector& position);
  void set_listener_velocity(const Vector& velocity);

  void enable_music(bool music_enabled);
  void play_music(const std::string& filename, bool fade = false);
  void stop_music(float fadetime = 0);

  bool is_music_enabled() { return music_enabled; }
  bool is_sound_enabled() { return sound_enabled; }

  bool is_audio_enabled() { return audio_open; }

  void update();

  /* kept for compatibility; music is not streamed on Symbian */
  void register_for_update( StreamSoundSource* sss );
  void remove_from_update( StreamSoundSource* sss );

  /** volume (0..1) for a sound at pos, given the listener position */
  float distance_gain(const Vector& pos, float reference_distance) const;

  /** Mix_Chunk for a sound file, loaded on first use; NULL if it can't be loaded */
  Mix_Chunk* get_chunk(const std::string& filename);

private:
  bool load_music(const std::string& filename);
  void free_music();

  bool audio_open;
  bool sound_enabled;
  bool music_enabled;

  typedef std::map<std::string, Mix_Chunk*> Chunks;
  Chunks chunks;

  typedef std::vector<SoundSource*> ManagedSources;
  ManagedSources managed_sources;

  Vector listener_position;

  std::string current_music;
  _Mix_Music* music;
  char* music_data;
  float music_loop_begin;   /* seconds */
  float music_loop_at;      /* seconds, -1 = play to the end */
  unsigned int music_started;  /* SDL_GetTicks() when the current pass started */
  float music_start_pos;    /* seconds into the file where that pass started */

private:
  SoundManager(const SoundManager&);
  SoundManager& operator=(const SoundManager&);
};

#endif

/* EOF */
