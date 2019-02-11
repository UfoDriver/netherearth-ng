#ifndef BULLET_H
#define BULLET_H

#include "cmc.h"
#include "vector.h"
#include "particle.h"
#include "list.h"

class Robot;
class Piece3DObject;


class Bullet {
public:
  enum BULLET_TYPE {BULLET_CANNONS, BULLET_MISSILES, BULLET_PHASERS};
  Bullet();
  Bullet(BULLET_TYPE type, Vector position, int angle, Robot *robot);
  void draw(bool shadow, Piece3DObject **bullet_tile, std::vector<Particle>& particles) const;


  BULLET_TYPE type;
  int step;

  Vector pos;
  int angle;
  Robot *owner;	/* The robot who fired this bullet */

  CMC cmc;

private:
  void drawParticles(std::vector<Particle>& particles) const;
};

#endif // BULLET_H
