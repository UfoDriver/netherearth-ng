#ifndef BULLET_H
#define BULLET_H

#include "cmc.h"
#include "vector.h"
#include "particle.h"

class Building;
class Robot;
class Piece3DObject;


class Bullet {
public:
  enum class TYPE {CANNONS, MISSILES, PHASERS};
  Bullet();
  Bullet(TYPE type, Vector position, Robot *robot);
  explicit Bullet(std::istream& in, std::vector<Robot*> robot[2]);
  void draw(bool shadow, std::vector<Piece3DObject>& bullet_tiles, std::vector<Particle>& particles) const;
  bool checkCollision(const std::vector<Building>& buildings,
                      const std::vector<Robot*> robots[2], Robot** r);

  TYPE type;
  int step;

  Vector pos;
  int angle;
  Robot *owner;	/* The robot who fired this bullet */

  CMC cmc;

private:
  void computeCMC(std::vector<Piece3DObject>& bulletTiles);
  void drawParticles(std::vector<Particle>& particles) const;
};

std::ostream& operator<<(std::ostream& out, std::pair<const Bullet&, std::vector<Robot*>*>pair);

#endif // BULLET_H
