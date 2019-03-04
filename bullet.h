#ifndef BULLET_H
#define BULLET_H

#include <memory>

#include "cmc.h"
#include "vector.h"
#include "particle.h"

class Building;
class Robot;
class Piece3DObject;


class Bullet {
public:
  enum class TYPE {CANNONS, MISSILES, PHASERS};
  Bullet(TYPE type, Vector position, Robot *robot);
  virtual void draw(bool shadow, std::vector<Particle>& particles) const = 0;
  bool checkCollision(const std::vector<Building>& buildings,
                      const std::vector<Robot*> robots[2], Robot** r);
  virtual int getPersistence() const = 0;
  int getDamageForRobot(const Robot* robot) const;

  static Bullet* read(std::istream& in, std::vector<Robot*> robot[2]);

  TYPE type;
  int step;

  Vector pos;
  int angle;
  Robot *owner;	/* The robot who fired this bullet */

  CMC cmc;

protected:
  void drawParticles(std::vector<Particle>& particles) const;

private:
  virtual int getBaseDamage() const = 0;
  virtual void computeCMC(std::vector<Piece3DObject>& bulletTiles) = 0;
};

#endif // BULLET_H
