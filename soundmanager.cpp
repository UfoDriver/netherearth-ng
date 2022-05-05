#include <SDL/SDL_mixer.h>
#include <algorithm>

#include "soundmanager.h"


SoundManager::SoundManager(bool soundEnabled)
  : soundEnabled{soundEnabled}
{
  S_shot = Mix_LoadWAV("sound/shot.wav");
  S_explosion = Mix_LoadWAV("sound/explosion.wav");
  S_select = Mix_LoadWAV("sound/select.wav");
  S_wrong = Mix_LoadWAV("sound/wrong.wav");
  S_construction = Mix_LoadWAV("sound/construction.wav");
}


SoundManager::~SoundManager()
{
  Mix_FreeChunk(S_shot);
  Mix_FreeChunk(S_explosion);
  Mix_FreeChunk(S_select);
  Mix_FreeChunk(S_wrong);
  Mix_FreeChunk(S_construction);
  S_shot=0;
  S_explosion=0;
  S_select=0;
  S_wrong=0;
  S_construction=0;
}


void SoundManager::playShot(const Vector& shipPos, const Vector& where) const
{
  if (S_shot != 0 && soundEnabled)
    Mix_Volume(Mix_PlayChannel(-1, S_shot, 0), withVolume(shipPos, where));
}


void SoundManager::playExplosion(const Vector& shipPos, const Vector& where) const
{
  if (S_explosion != 0 && soundEnabled)
    Mix_Volume(Mix_PlayChannel(-1, S_explosion, 0), withVolume(shipPos, where));
}


void SoundManager::playSelect() const
{
  playSound(S_select);
}


void SoundManager::playWrong() const
{
  playSound(S_wrong);
}


void SoundManager::playConstruction() const
{
  playSound(S_construction);
}


void SoundManager::playSound(const Mix_Chunk* sample) const
{
  if (sample && soundEnabled) {
    Mix_Volume(Mix_PlayChannel(-1, const_cast<Mix_Chunk*>(sample), 0), 128);
  }
}


int SoundManager::withVolume(const Vector& shipPos, const Vector& pos) const
{
  float distance = (pos - shipPos).norma();
  distance = (distance - 8) / 8;
  distance = std::max(1.0f, distance);
  return 128 / distance;
}
