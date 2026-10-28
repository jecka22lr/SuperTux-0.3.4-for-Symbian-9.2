//  SuperTux - Symbian port
//  A SoundSource that plays a Mix_Chunk on an SDL_mixer channel.
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.

#include "audio/mixer_sound_source.hpp"

#include <SDL_mixer.h>

#include "audio/sound_manager.hpp"

MixerSoundSource::MixerSoundSource(SoundManager* manager_, Mix_Chunk* chunk_) :
  manager(manager_),
  chunk(chunk_),
  channel(-1),
  looping(false),
  relative(false),
  positioned(false),
  gain(1.0f),
  reference_distance(128),
  position()
{
}

MixerSoundSource::~MixerSoundSource()
{
  stop();
}

void
MixerSoundSource::play()
{
  stop();
  channel = Mix_PlayChannel(-1, chunk, looping ? -1 : 0);
  update_volume();
}

void
MixerSoundSource::stop()
{
  if(playing())
    Mix_HaltChannel(channel);
  channel = -1;
}

bool
MixerSoundSource::playing()
{
  /* the channel may have finished and been reused by another sound */
  return channel >= 0 && Mix_Playing(channel) && Mix_GetChunk(channel) == chunk;
}

void
MixerSoundSource::set_looping(bool looping_)
{
  looping = looping_;
}

void
MixerSoundSource::set_relative(bool relative_)
{
  relative = relative_;
  update_volume();
}

void
MixerSoundSource::set_gain(float gain_)
{
  gain = gain_;
  update_volume();
}

void
MixerSoundSource::set_pitch(float )
{
  /* SDL_mixer can't change pitch; the few sounds that use it play at normal pitch */
}

void
MixerSoundSource::set_position(const Vector& position_)
{
  position = position_;
  positioned = true;
  update_volume();
}

void
MixerSoundSource::set_velocity(const Vector& )
{
  /* no doppler effect */
}

void
MixerSoundSource::set_reference_distance(float distance)
{
  reference_distance = distance;
  update_volume();
}

void
MixerSoundSource::update_volume()
{
  if(!playing())
    return;
  float g = gain;
  if(positioned && !relative)
    g *= manager->distance_gain(position, reference_distance);
  int volume = (int) (g * MIX_MAX_VOLUME);
  if(volume < 0) volume = 0;
  if(volume > MIX_MAX_VOLUME) volume = MIX_MAX_VOLUME;
  Mix_Volume(channel, volume);
}

/* EOF */
