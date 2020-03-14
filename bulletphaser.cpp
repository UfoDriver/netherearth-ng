#include <GL/gl.h>

#include "bulletphaser.h"
#include "quaternion.h"
#include "resources.h"


BulletPhaser::BulletPhaser(Vector& position, Robot* owner):
  Bullet(TYPE::PHASERS, position, owner)
{
  computeCMC(Resources::bulletTiles[2]);
}


void BulletPhaser::draw(bool shadow, std::vector<Particle>&) const
{
  if (!shadow) {
    glPushMatrix();
    glRotatef(angle, 0, 0, 1);
    glRotatef(90, 0, 0, 1);

    if ((rand() % 4) != 0)
      Resources::bulletTiles[2].draw(Color(1.0f, 0.5f, 1.0f, 0.9f));
    else
      Resources::bulletTiles[2].draw(Color(1.0f, 1.0f, 1.0f, 0.5f));


    glPopMatrix();
  }
}


void BulletPhaser::computeCMC(const Piece3DObject& bulletTile)
{
  Quaternion q;
  float m[16] = {1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1};

  q.from_axis_angle(Vector(0, 0, 1), 3.141592f / 2);
  q.to_matrix(m);
  cmc.expand(bulletTile.cmc, m);
}
