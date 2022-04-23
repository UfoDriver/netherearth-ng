#ifndef BULLET_H
#define BULLET_H

#include <memory>

#include <sexp/value.hpp>

#include "cmc.h"
#include "vector.h"
#include "particle.h"
#include "robots.h"

class BuildingBlock;
class Robot;
class Piece3DObject;


class Bullet {
public:
  enum class TYPE {
    CANNONS,
    MISSILES,
    PHASERS
  };

  Bullet(TYPE type, Vector position, Robot *robot);
  virtual ~Bullet() {};

  virtual void draw(bool shadow, std::vector<Particle>& particles) const = 0;

  bool checkCollision(const std::vector<std::shared_ptr<BuildingBlock>>& buildings,
                      const Robots& robots, std::shared_ptr<Robot> &r);

  virtual int getPersistence() const = 0;
  int getDamageForRobot(const Robot* robot) const;

  TYPE type;
  int step;

  Vector pos;
  int angle;
  Robot *owner;	/* The robot who fired this bullet */

  CMC cmc;

  virtual sexp::Value toSexp() const;
  static Bullet* fromSexp(const sexp::Value&);

protected:
  void drawParticles(std::vector<Particle>& particles) const;

private:
  virtual int getBaseDamage() const = 0;
  virtual void computeCMC(const Piece3DObject& bulletTile) = 0;
};

#endif // BULLET_H
