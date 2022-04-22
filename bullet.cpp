#ifdef _WIN32
#include "windows.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <sexp/util.hpp>

#include "3dobject.h"
#include "bullet.h"
#include "bulletcannon.h"
#include "bulletmissile.h"
#include "bulletphaser.h"
#include "cmc.h"
#include "glprintf.h"
#include "myglutaux.h"
#include "nether.h"
#include "piece3dobject.h"
#include "quaternion.h"
#include "resources.h"
#include "shadow3dobject.h"
#include "vector.h"


Bullet::Bullet(TYPE type, Vector position, Robot *robot):
  type(type), step(0), pos(position), angle(robot->getAngle()), owner(robot)
{
}


Bullet* Bullet::read(std::istream& in, const std::vector<Robot*>& robots)
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
    owner = robots[i];
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
  Vector particlePos, sp1;

  for(int i = 0; i < 10; i++) {
    particlePos.x = pos.x + float(rand() % 10) / 100.0;
    particlePos.y = pos.y + float(rand( )% 10) / 100.0;
    particlePos.z = pos.z;
    Color color(0.9F + float(rand() % 21 - 10) / 100.0,
                0.7F + float(rand() % 21 - 10) / 100.0,
                0.5F + float(rand() % 21 - 10) / 100.0);
    switch(angle) {
    case 0:
      sp1 = Vector(-0.05, float(rand() % 9 - 4) / 200.0,0);
      particlePos.x -= 0.25;
      particlePos.y += ((rand() % 2) == 0 ? -0.33 : 0.33);
      break;
    case 90:
      sp1 = Vector(float(rand() % 9 - 4) / 200.0, -0.05, 0);
      particlePos.y -= 0.25;
      particlePos.x += ((rand() % 2) == 0 ? -0.33 : 0.33);
      break;
    case 180:
      sp1 = Vector(0.05, float(rand() % 9 - 4) / 200.0, 0);
      particlePos.x += 0.25;
      particlePos.y += ((rand() % 2) == 0 ? -0.33 : 0.33);
      break;
    case 270:
      sp1 = Vector(float(rand() % 9 - 4) / 200.0, 0.05, 0);
      particlePos.y += 0.25;
      particlePos.x += ((rand() % 2) == 0 ? -0.33 : 0.33);
      break;
    }
    particles.emplace_back(particlePos, sp1, sp1, 0, 0.3, color, 1.0, 0.0, 10 + (rand() % 8));
  }
}


bool Bullet::checkCollision(const std::vector<std::unique_ptr<Building>>& buildings,
                            const Robots& robots, std::shared_ptr<Robot> &r)
{
  float m1[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  pos.x, pos.y, 0.3, 1};
  float m2[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1};

  for (const auto& b: buildings) {
    if (b->pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
      m2[12] = b->pos.x;
      m2[13] = b->pos.y;
      m2[14] = b->pos.z;

      // if (cmc.collision_simple(m1, b->getCMC(), m2))
      //   return true;

      if (b->collisionCheck(cmc, m1))
        return true;

    }
  }

  for (std::shared_ptr<Robot> rt: robots) {
    if ((rt.get() != owner) && rt->pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
      m2[12] = rt->pos.x;
      m2[13] = rt->pos.y;
      m2[14] = rt->pos.z;
      if (cmc.collision_simple(m1, rt->cmc, m2)) {
        r = rt;
        return true;
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
  if (robot->getTraction() == 0)
    maxDamage -= baseDamage;
  return maxDamage - multiplier[robot->npieces() - 1] * baseDamage;
}


sexp::Value Bullet::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("bullet"),
    sexp::Value::integer((int)type),
    sexp::Value::integer(step),
    sexp::Value::integer(angle),
    pos.toSexp(),
    // @TODO: bullet owner should be in tags
    //   map.robots[index]->getOwner()
    //  index
    cmc.toSexp()
  );
}


Bullet* Bullet::fromSexp(const sexp::Value& value)
{
  sexp::Value posAndCMC = sexp::cdddar(value.get_cdr());
  Vector pos;
  pos.fromSexp(sexp::car(posAndCMC));

  Bullet* bullet;
  switch (TYPE(sexp::cdar(value).as_int())) {
  case TYPE::CANNONS:
    bullet = new BulletCannon(pos, nullptr);
    break;
  case TYPE::MISSILES:
    bullet = new BulletMissile(pos, nullptr);
    break;
  case TYPE::PHASERS:
    bullet = new BulletPhaser(pos, nullptr);
    break;
  }
  bullet->step = sexp::cddar(value).as_int();
  bullet->angle = sexp::cdddar(value).as_int();
  bullet->cmc.fromSexp(sexp::cdar(posAndCMC));
  return bullet;
}
