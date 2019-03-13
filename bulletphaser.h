#ifndef BULLETPHASER_H
#define BULLETPHASER_H

#include "bullet.h"
#include "constants.h"


class BulletPhaser : public Bullet {
public:
  BulletPhaser(Vector& position, Robot* owner);
  void draw(bool shadow, std::vector<Particle>& particles) const override;
  int getPersistence() const override { return PHASER_PERSISTENCE; }

private:
  void computeCMC(std::vector<Piece3DObject>& bulletTiles) override;
  int getBaseDamage() const override { return 4; }
};

#endif // BULLETPHASER_H
