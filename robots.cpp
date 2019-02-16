
#ifdef _WIN32
#include "windows.h"
#endif

#include "string.h"
#include "stdio.h"
#include "math.h"

#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "myglutaux.h"
#include "nether.h"

#include "glprintf.h"


extern int detaillevel;


bool NETHER::robotCollision(Robot *r, bool complete)
{
  float m1[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  r->pos.x, r->pos.y, r->pos.z, 1};
  float m2[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1};
  {
    float m2[16]={1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  ship->pos.x, ship->pos.y, ship->pos.z, 1};
    if (r->cmc.collision_simple(m1, &(ship->cmc), m2)) return true;
  }

  if (!complete) return false;

  for (const Building& b: buildings) {
    if (((b.pos.x-r->pos.x)*(b.pos.x-r->pos.x)+
         (b.pos.y-r->pos.y)*(b.pos.y-r->pos.y)+
         (b.pos.z-r->pos.z)*(b.pos.z-r->pos.z))<COLISION_TEST_THRESHOLD) {
      m2[12]=b.pos.x;
      m2[13]=b.pos.y;
      m2[14]=b.pos.z;

      switch(b.type) {
      case Building::TYPE::FENCE:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[5].cmc),m2)) return true;
        break;
      case Building::TYPE::WALL1:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[0].cmc),m2)) return true;
        break;
      case Building::TYPE::WALL2:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[1].cmc),m2)) return true;
        break;
      case Building::TYPE::WALL3:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[2].cmc),m2)) return true;
        break;
      case Building::TYPE::WALL4:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[3].cmc),m2)) return true;
        break;
      case Building::TYPE::WALL5:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[4].cmc),m2)) return true;
        break;
      case Building::TYPE::WALL6:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[7].cmc),m2)) return true;
        break;
      case Building::TYPE::WARBASE:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[8].cmc),m2)) return true;
        break;
      case Building::TYPE::FACTORY_ELECTRONICS:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(Resources::pieceTiles[0][7].cmc),m2)) return true;
        break;
      case Building::TYPE::FACTORY_NUCLEAR:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(Resources::pieceTiles[0][6].cmc),m2)) return true;
        break;
      case Building::TYPE::FACTORY_PHASERS:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(Resources::pieceTiles[0][5].cmc),m2)) return true;
        break;
      case Building::TYPE::FACTORY_MISSILES:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(Resources::pieceTiles[0][4].cmc),m2)) return true;
        break;
      case Building::TYPE::FACTORY_CANNONS:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(Resources::pieceTiles[0][3].cmc),m2)) return true;
        break;
      case Building::TYPE::FACTORY_CHASSIS:
        if (r->cmc.collision_simple(m1,&(Resources::buildingTiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(Resources::pieceTiles[0][1].cmc),m2)) return true;
        break;
      }
    }
  }

  for (int i = 0; i < 2; i++) {
    for(Robot* rt: robots[i]) {
      if (((rt->pos.x-r->pos.x)*(rt->pos.x-r->pos.x)+
           (rt->pos.y-r->pos.y)*(rt->pos.y-r->pos.y)+
           (rt->pos.z-r->pos.z)*(rt->pos.z-r->pos.z))<COLISION_TEST_THRESHOLD) {
        if (rt != r) {
          m2[12]=rt->pos.x;
          m2[13]=rt->pos.y;
          m2[14]=rt->pos.z;
          if (r->cmc.collision_simple(m1,&(rt->cmc),m2)) return true;
        }
      }
    }
  }

  return false;
}
