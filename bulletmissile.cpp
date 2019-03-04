#include <GL/gl.h>

#include "bulletmissile.h"
#include "quaternion.h"
#include "resources.h"


extern int detaillevel;

BulletMissile::BulletMissile(Vector& position, Robot* owner):
  Bullet(TYPE::MISSILES, position, owner)
{
  computeCMC(Resources::bulletTiles);
}


void BulletMissile::draw(bool shadow, std::vector<Particle>& particles) const
{
  if (!shadow) {
    glPushMatrix();
    glRotatef(angle, 0, 0, 1);
    glRotatef(180, 0, 0, 1);
    glTranslatef(0, 0.33, 0);
    Resources::bulletTiles[1].draw(Color(0.8f, 0.8f, 0.8f));
    glTranslatef(0, -0.66, 0);
    Resources::bulletTiles[1].draw(Color(0.8f, 0.8f, 0.8f));
    glPopMatrix();
    if (detaillevel >= 4) {
      drawParticles(particles);
    }
  }
}


void BulletMissile::computeCMC(std::vector<Piece3DObject>& bulletTiles)
{
  Quaternion q;
  float m[16] = {1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1};

  q.from_axis_angle(Vector(0,0,1), 3.141592f);
  q.to_matrix(m);
  m[13] += 0.33;
  cmc.expand(&(bulletTiles[1].cmc), m);
  m[13] -= 0.66;
  cmc.expand(&(bulletTiles[1].cmc), m);
}
