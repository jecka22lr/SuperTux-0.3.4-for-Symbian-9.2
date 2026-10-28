//  SuperTux - Symbian port
//  A SoundSource that plays a Mix_Chunk on an SDL_mixer channel.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#ifndef HEADER_SUPERTUX_AUDIO_MIXER_SOUND_SOURCE_HPP
#define HEADER_SUPERTUX_AUDIO_MIXER_SOUND_SOURCE_HPP

#include "audio/sound_source.hpp"
#include "math/vector.hpp"

struct Mix_Chunk;
class SoundManager;

class MixerSoundSource : public SoundSource
{
public:
  MixerSoundSource(SoundManager* manager, Mix_Chunk* chunk);
  virtual ~MixerSoundSource();

  virtual void play();
  virtual void stop();
  virtual bool playing();

  virtual void set_looping(bool looping);
  virtual void set_relative(bool relative);
  virtual void set_gain(float gain);
  virtual void set_pitch(float pitch);
  virtual void set_position(const Vector& position);
  virtual void set_velocity(const Vector& velocity);
  virtual void set_reference_distance(float distance);

  /** recompute the channel volume (listener moved or settings changed) */
  void update_volume();

private:
  SoundManager* manager;
  Mix_Chunk* chunk;
  int channel;
  bool looping;
  bool relative;
  bool positioned;
  float gain;
  float reference_distance;
  Vector position;
};

#endif

/* EOF */
