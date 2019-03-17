#ifndef BULLETMISSILE_H
#define BULLETMISSILE_H

#include "bullet.h"
#include "constants.h"


class BulletMissile : public Bullet {
public:
  BulletMissile(Vector& position, Robot* owner);
  void draw(bool shadow, std::vector<Particle>& particles) const override;
  int getPersistence() const override { return MISSILE_PERSISTENCE; }

private:
  void computeCMC(const Piece3DObject& bulletTile) override;
  int getBaseDamage() const override { return 3; }
};

#endif // BULLETMISSILE_H
