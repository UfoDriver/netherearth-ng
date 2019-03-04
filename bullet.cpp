#ifdef _WIN32
#include "windows.h"
#endif

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

#include "vector.h"
#include "quaternion.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "myglutaux.h"
#include "nether.h"
#include "bullet.h"
#include "bulletcannon.h"
#include "bulletmissile.h"
#include "bulletphaser.h"
#include "utils.h"
#include "resources.h"

#include "glprintf.h"


extern int detaillevel;


Bullet::Bullet(TYPE type, Vector position, Robot *robot):
  type(type), step(0), pos(position), angle(robot->angle), owner(robot)
{
}


Bullet* Bullet::read(std::istream& in, std::vector<Robot*> robots[2])
{
  int i, j;
  int type_;
  Vector pos;
  int step;
  int angle;
  Robot* owner;

  in >> type_ >> step >> angle;
  in >> pos;
  in >> j >> i;
  if (i >= 0)
    owner = robots[j][i];
  else
    owner = 0;

  Bullet* bullet;
  switch (TYPE(type_)) {
  case TYPE::CANNONS:
    bullet = new BulletCannon(pos, owner);
    break;
  case TYPE::MISSILES:
    bullet = new BulletMissile(pos, owner);
    break;
  case TYPE::PHASERS:
    bullet = new BulletPhaser(pos, owner);
    break;
  }
  bullet->angle = angle;
  bullet->step = step;
  in >> bullet->cmc;
  return bullet;
}


void Bullet::drawParticles(std::vector<Particle>& particles) const
{
  Vector pos, sp1;

  for(int i = 0; i < 10; i++) {
    pos.x = pos.x + float(rand() % 10) / 100.0;
    pos.y = pos.y + float(rand( )% 10) / 100.0;
    pos.z = pos.z;
    Color color(0.9F + float(rand() % 21 - 10) / 100.0,
                0.7F + float(rand() % 21 - 10) / 100.0,
                0.5F + float(rand() % 21 - 10) / 100.0);
    switch(angle) {
    case 0:
      sp1 = Vector(-0.05, float(rand() % 9 - 4) / 200.0,0);
      pos.x -= 0.25;
      pos.y += ((rand() % 2) == 0 ? -0.33 : 0.33);
      break;
    case 90:
      sp1 = Vector(float(rand() % 9 - 4) / 200.0, -0.05, 0);
      pos.y -= 0.25;
      pos.x += ((rand() % 2) == 0 ? -0.33 : 0.33);
      break;
    case 180:
      sp1 = Vector(0.05, float(rand() % 9 - 4) / 200.0, 0);
      pos.x += 0.25;
      pos.y += ((rand() % 2) == 0 ? -0.33 : 0.33);
      break;
    case 270:
      sp1 = Vector(float(rand() % 9 - 4) / 200.0, 0.05, 0);
      pos.y += 0.25;
      pos.x += ((rand() % 2) == 0 ? -0.33 : 0.33);
      break;
    }
    particles.emplace_back(pos, sp1, sp1, 0, 0.3, color, 1.0, 0.0, 10 + (rand() % 8));
  }
}


bool Bullet::checkCollision(const std::vector<Building>& buildings,
                            const std::vector<Robot*> robots[2], Robot** r)
{
  float m1[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  pos.x, pos.y, 0.3, 1};
  float m2[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1};

  for (const Building& b: buildings) {
    if (b.pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
      m2[12] = b.pos.x;
      m2[13] = b.pos.y;
      m2[14] = b.pos.z;

      if (cmc.collision_simple(m1, b.getCMC(), m2))
        return true;
    }
  }

  for (int i = 0; i < 2; i++) {
    for (Robot* rt: robots[i]) {
      if (rt->pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
        if (rt != owner) {
          m2[12] = rt->pos.x;
          m2[13] = rt->pos.y;
          m2[14] = rt->pos.z;
          if (cmc.collision_simple(m1, rt->cmc, m2)) {
            *r = rt;
            return true;
          }
        }
      }
    }
  }

  return false;
}


int Bullet::getDamageForRobot(const Robot* robot) const
{
  int baseDamage = getBaseDamage();
  int maxDamage = baseDamage * 12;
  int multiplier[] = {1, 2, 4, 6, 8};
  if (robot->traction == 0)
    maxDamage -= baseDamage;
  return maxDamage - multiplier[robot->npieces() - 1] * baseDamage;
}
