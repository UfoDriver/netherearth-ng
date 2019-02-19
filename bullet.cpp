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
#include "utils.h"
#include "resources.h"

#include "glprintf.h"


extern int detaillevel;

Bullet::Bullet(): type(TYPE::CANNONS), step(0), angle(0), owner(0)
{
}


Bullet::Bullet(TYPE type, Vector position, int angle, Robot *robot):
  type(type), step(0), pos(position), angle(angle), owner(robot)
{
}


Bullet::Bullet(std::istream& in, std::vector<Robot*> robots[2])
{
  int i, j;
  int type_;
  in >> type_ >> step >> angle;
  type = TYPE(type_);
  in >> pos;
  in >> j >> i;
  if (i >= 0)
    owner = robots[j][i];
  else
    owner = 0;
  in >> cmc;
}


void Bullet::computeCMC(std::vector<Piece3DObject>& bulletTiles)
{
  float m[16] = {1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1};
  Quaternion q;

  switch(type) {
  case TYPE::CANNONS:
    m[13] = 0.2;
    cmc.expand(&(bulletTiles[0].cmc), m);
    m[13] = -0.2;
    cmc.expand(&(bulletTiles[0].cmc), m);
    break;
  case TYPE::MISSILES:
    q.from_axis_angle(Vector(0,0,1), 3.141592f);
    q.to_matrix(m);
    m[13] += 0.33;
    cmc.expand(&(bulletTiles[1].cmc), m);
    m[13] -= 0.66;
    cmc.expand(&(bulletTiles[1].cmc), m);
    break;
  case TYPE::PHASERS:
    q.from_axis_angle(Vector(0, 0, 1), 3.141592f/2);
    q.to_matrix(m);
    cmc.expand(&(bulletTiles[2].cmc), m);
    break;
  }
}


void Bullet::draw(bool shadows, std::vector<Piece3DObject>& bullet_tiles,
                  std::vector<Particle>& particles) const
{
  switch(type) {
  case TYPE::CANNONS:
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle, 0, 0, 1);
      glTranslatef(0, 0.2, 0);
      bullet_tiles[0].draw(Color(0.2f, 0.2f, 0.2f));
      glTranslatef(0, -0.4, 0);
      bullet_tiles[0].draw(Color(0.2f, 0.2f, 0.2f));
      glPopMatrix();
    }
    break;
  case TYPE::MISSILES:
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle, 0, 0, 1);
      glRotatef(180, 0, 0, 1);
      glTranslatef(0, 0.33, 0);
      bullet_tiles[1].draw(Color(0.8f, 0.8f, 0.8f));
      glTranslatef(0, -0.66, 0);
      bullet_tiles[1].draw(Color(0.8f, 0.8f, 0.8f));
      glPopMatrix();
      if (detaillevel >= 4) {
        drawParticles(particles);
      }
    }
    break;
  case TYPE::PHASERS:
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle,0,0,1);
      glRotatef(90,0,0,1);

      if ((rand() % 4) !=0)
        bullet_tiles[2].draw_notexture(Color(1.0f, 0.5f, 1.0f, 0.9f));
      else
        bullet_tiles[2].draw_notexture(Color(1.0f, 1.0f, 1.0f, 0.5f));

      glPopMatrix();
    }
    break;
  }
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
    case 90 : sp1 = Vector(float(rand() % 9 - 4) / 200.0, -0.05, 0);
      pos.y -= 0.25;
      pos.x+=((rand() % 2) == 0 ? -0.33 : 0.33);
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
  float m1[16]={1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                pos.x, pos.y, 0.3, 1};
  float m2[16]={1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1};

  for (const Building& b: buildings) {
    if (b.pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
      m2[12] = b.pos.x;
      m2[13] = b.pos.y;
      m2[14] = b.pos.z;

      switch(b.type) {
      case Building::TYPE::FENCE:
        if (cmc.collision_simple(m1, Resources::buildingTiles[5].cmc ,m2)) return true;
        break;
      case Building::TYPE::WALL1:
        if (cmc.collision_simple(m1, Resources::buildingTiles[0].cmc ,m2)) return true;
        break;
      case Building::TYPE::WALL2:
        if (cmc.collision_simple(m1, Resources::buildingTiles[1].cmc ,m2)) return true;
        break;
      case Building::TYPE::WALL3:
        if (cmc.collision_simple(m1, Resources::buildingTiles[2].cmc ,m2)) return true;
        break;
      case Building::TYPE::WALL4:
        if (cmc.collision_simple(m1, Resources::buildingTiles[3].cmc ,m2)) return true;
        break;
      case Building::TYPE::WALL5:
        if (cmc.collision_simple(m1, Resources::buildingTiles[4].cmc ,m2)) return true;
        break;
      case Building::TYPE::WALL6:
        if (cmc.collision_simple(m1, Resources::buildingTiles[7].cmc ,m2)) return true;
        break;
      case Building::TYPE::WARBASE:
        if (cmc.collision_simple(m1, Resources::buildingTiles[8].cmc ,m2)) return true;
        break;
      case Building::TYPE::FACTORY_ELECTRONICS:
        if (cmc.collision_simple(m1, Resources::buildingTiles[4].cmc ,m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (cmc.collision_simple(m1, Resources::pieceTiles[0][7].cmc ,m2)) return true;
        break;
      case Building::TYPE::FACTORY_NUCLEAR:
        if (cmc.collision_simple(m1, Resources::buildingTiles[4].cmc ,m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (cmc.collision_simple(m1, Resources::pieceTiles[0][6].cmc, m2)) return true;
        break;
      case Building::TYPE::FACTORY_PHASERS:
        if (cmc.collision_simple(m1, Resources::buildingTiles[4].cmc, m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (cmc.collision_simple(m1, Resources::pieceTiles[0][5].cmc ,m2)) return true;
        break;
      case Building::TYPE::FACTORY_MISSILES:
        if (cmc.collision_simple(m1, Resources::buildingTiles[4].cmc, m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (cmc.collision_simple(m1, Resources::pieceTiles[0][4].cmc ,m2)) return true;
        break;
      case Building::TYPE::FACTORY_CANNONS:
        if (cmc.collision_simple(m1, Resources::buildingTiles[4].cmc, m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (cmc.collision_simple(m1, Resources::pieceTiles[0][3].cmc, m2)) return true;
        break;
      case Building::TYPE::FACTORY_CHASSIS:
        if (cmc.collision_simple(m1, Resources::buildingTiles[4].cmc, m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (cmc.collision_simple(m1, Resources::pieceTiles[0][1].cmc ,m2)) return true;
        break;
      }
    }
  }

  /* Collision with the robots: */
  for (int i = 0; i < 2; i++) {
    for (Robot* rt: robots[i]) {
      if (rt->pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
        if (rt != owner) {
          m2[12] = rt->pos.x;
          m2[13] = rt->pos.y;
          m2[14] = rt->pos.z;
          if (cmc.collision_simple(m1, rt->cmc, m2)) {
            *r=rt;
            return true;
          } /* if */
        } /* if */
      } /* if */
    } /* while */
  } /* while */

  return false;
}


std::ostream& operator<<(std::ostream& out, std::pair<const Bullet&, std::vector<Robot*>*>pair)
{
  out << (int)pair.first.type << ' ' << pair.first.step << ' ' << pair.first.angle << '\n';
  out << pair.first.pos;

  int i = find_index(pair.second[0], pair.first.owner);
  if (i == -1) {
    i = find_index(pair.second[1], pair.first.owner);
    out << 1 << ' ' << i;
  } else {
    out << 0 << ' ' << i;
  }

  return out << pair.first.cmc;
}
