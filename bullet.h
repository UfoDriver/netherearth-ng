#ifndef BULLET_H
#define BULLET_H

#include "cmc.h"
#include "vector.h"
#include "particle.h"

class Robot;
class Piece3DObject;


class Bullet {
public:
  enum BULLET_TYPE {BULLET_CANNONS, BULLET_MISSILES, BULLET_PHASERS};
  Bullet();
  Bullet(BULLET_TYPE type, Vector position, int angle, Robot *robot);
  explicit Bullet(std::istream& in, std::vector<Robot*> robot[2]);
  void draw(bool shadow, std::vector<Piece3DObject>& bullet_tiles, std::vector<Particle>& particles) const;

  BULLET_TYPE type;
  int step;

  Vector pos;
  int angle;
  Robot *owner;	/* The robot who fired this bullet */

  CMC cmc;
  void computeCMC(std::vector<Piece3DObject>& bulletTiles);

private:
  void drawParticles(std::vector<Particle>& particles) const;
};

#endif // BULLET_H
