#ifndef BULLETCANNON_H
#define BULLETCANNON_H

#include "bullet.h"
#include "constants.h"


class Robot;
class Vector;

class BulletCannon : public Bullet {
public:
  BulletCannon(Vector& position, Robot *owner);
  void draw(bool shadow, std::vector<Particle>& particles) const override;
  int getPersistence() const override { return CANNON_PERSISTENCE; }

private:
  void computeCMC(std::vector<Piece3DObject>& bulletTiles) override;
  int getBaseDamage() const override { return 2; }
};

#endif // BULLETCANNON_H
