#include <GL/gl.h>

#include "bulletcannon.h"
#include "resources.h"
#include "robot.h"


BulletCannon::BulletCannon(Vector& position, Robot* owner):
  Bullet(TYPE::CANNONS, position, owner)
{
  computeCMC(Resources::bulletTiles);
}


void BulletCannon::draw(bool shadow, std::vector<Particle>&) const
{
  if (!shadow) {
    glPushMatrix();
    glRotatef(angle, 0, 0, 1);
    glTranslatef(0, 0.2, 0);
    Resources::bulletTiles[0].draw(Color(0.2f, 0.2f, 0.2f));
    glTranslatef(0, -0.4, 0);
    Resources::bulletTiles[0].draw(Color(0.2f, 0.2f, 0.2f));
    glPopMatrix();
  }
}


void BulletCannon::computeCMC(std::vector<Piece3DObject>& bulletTiles)
{
  float m[16] = {1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1};

  m[13] = 0.2;
  cmc.expand(&(bulletTiles[0].cmc), m);
  m[13] = -0.2;
  cmc.expand(&(bulletTiles[0].cmc), m);
}
