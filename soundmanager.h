#ifndef SOUNDMANAGER_H
#define SOUNDMANAGER_H

#include "SDL/SDL_mixer.h"

#include "vector.h"


class SoundManager {
public:
  SoundManager();
  ~SoundManager();

  void playShot(const Vector& ship, const Vector& where) const;
  void playExplosion(const Vector& ship, const Vector& where) const;
  void playSelect() const;
  void playWrong() const;
  void playConstruction() const;

private:
  void playSound(const Mix_Chunk* sample) const;
  int withVolume(const Vector& shipPos, const Vector& pos) const;

  Mix_Chunk *S_shot, *S_explosion, *S_select, *S_wrong, *S_construction;
};

#endif // SOUNDMANAGER_H
