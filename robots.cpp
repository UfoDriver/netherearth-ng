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

#include "list.h"
#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "myglutaux.h"
#include "nether.h"

#include "glprintf.h"


const float ms[4][3]={{0.0078125,0.015625,0.03125},
					{0.00390625,0.0078125,0.03125},
					{0,0.0078125,0.015625},
					{0,0,0.03125}};

const int rs[4][3]={{2,3,5},
					{1,2,5},
					{0,2,3},
					{0,0,5}};
extern int detaillevel;


CMC NETHER::RobotCMC(Robot *r,int owner)
{
	CMC cmc;
	float m[16]={1,0,0,0,
				 0,1,0,0,
				 0,0,1,0,
				 0,0,0,1};

	switch(r->traction) {
	case 0:
		cmc=piece_tile[owner][0]->cmc;
		m[14]=1.0;
		break;
	case 1:
		cmc=piece_tile[owner][1]->cmc;
		m[14]=0.35;
		break;
	case 2:
		cmc=piece_tile[owner][2]->cmc;
		m[14]=0.45;
		break;
	} /* switch */ 

	if (r->pieces[0]) {
		cmc.expand(&(piece_tile[owner][3]->cmc),m);
		m[14]+=0.5;
	} /* if */ 
	if (r->pieces[1]) {
		cmc.expand(&(piece_tile[owner][4]->cmc),m);
		m[14]+=0.35;
	} /* if */ 
	if (r->pieces[2]) {
		cmc.expand(&(piece_tile[owner][5]->cmc),m);
		m[14]+=0.5;
	} /* if */ 
	if (r->pieces[3]) {
		cmc.expand(&(piece_tile[owner][6]->cmc),m);
		m[14]+=0.8;
	} /* if */ 
	if (r->pieces[4]) {
		cmc.expand(&(piece_tile[owner][7]->cmc),m);
	} /* if */ 

	return cmc;
} /* ROBOT::buildCMC */ 


float NETHER::RobotSpeed(int traction,int terrain)
{
	if (terrain<4 && traction<3) return ms[terrain][traction];

/*	switch(terrain) {
	case T_GRASS:
		if (traction==0) return 0.0078125;
		if (traction==1) return 0.015625;
		return 0.03125;
		break;
	case T_SAND:
		if (traction==0) return 0.00390625;
		if (traction==1) return 0.0078125;
		return 0.03125;
		break;
	case T_MOUNTAINS:
		if (traction==0) return 0;
		if (traction==1) return 0.0078125;
		return 0.015625;
		break;
	case T_HOLE:
		if (traction==0) return 0;
		if (traction==1) return 0;
		return 0.03125;
		break;
	} /* switch */ 
	return 0;
} /* NETHER::RobotSpeed */ 


int NETHER::RobotRotationSpeed(int traction,int terrain)
{
	if (terrain<4 && traction<3) return rs[terrain][traction];

/*	switch(terrain) {
	case T_GRASS:
		if (traction==0) return 2;
		if (traction==1) return 3;
		return 5;
		break;
	case T_SAND:
		if (traction==0) return 1;
		if (traction==1) return 2;
		return 5;
		break;
	case T_MOUNTAINS:
		if (traction==0) return 0;
		if (traction==1) return 2;
		return 3;
		break;
	case T_HOLE:
		if (traction==0) return 0;
		if (traction==1) return 0;
		return 5;
		break;
	} /* switch */ 
	return 0;
} /* NETHER::RobotRotationSpeed */ 


bool NETHER::Walkable(int traction,int terrain)
{
	if (RobotSpeed(traction,terrain)!=0) return true;

	return false;
} /* NETHER::Walkable */ 


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
                  shipp.x, shipp.y, shipp.z, 1};
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
      case Building::B_FENCE:
        if (r->cmc.collision_simple(m1,&(building_tiles[5].cmc),m2)) return true;
        break;
      case Building::B_WALL1:
        if (r->cmc.collision_simple(m1,&(building_tiles[0].cmc),m2)) return true;
        break;
      case Building::B_WALL2:
        if (r->cmc.collision_simple(m1,&(building_tiles[1].cmc),m2)) return true;
        break;
      case Building::B_WALL3:
        if (r->cmc.collision_simple(m1,&(building_tiles[2].cmc),m2)) return true;
        break;
      case Building::B_WALL4:
        if (r->cmc.collision_simple(m1,&(building_tiles[3].cmc),m2)) return true;
        break;
      case Building::B_WALL5:
        if (r->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
        break;
      case Building::B_WALL6:
        if (r->cmc.collision_simple(m1,&(building_tiles[7].cmc),m2)) return true;
        break;
      case Building::B_WARBASE:
        if (r->cmc.collision_simple(m1,&(building_tiles[8].cmc),m2)) return true;
        break;
      case Building::B_FACTORY_ELECTRONICS:
        if (r->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(piece_tile[0][7]->cmc),m2)) return true;
        break;
      case Building::B_FACTORY_NUCLEAR:
        if (r->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(piece_tile[0][6]->cmc),m2)) return true;
        break;
      case Building::B_FACTORY_PHASERS:
        if (r->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(piece_tile[0][5]->cmc),m2)) return true;
        break;
      case Building::B_FACTORY_MISSILES:
        if (r->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(piece_tile[0][4]->cmc),m2)) return true;
        break;
      case Building::B_FACTORY_CANNONS:
        if (r->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(piece_tile[0][3]->cmc),m2)) return true;
        break;
      case Building::B_FACTORY_CHASSIS:
        if (r->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
        m2[12]=b.pos.x+0.5;
        m2[13]=b.pos.y+0.5;
        m2[14]=b.pos.z+1;
        if (r->cmc.collision_simple(m1,&(piece_tile[0][1]->cmc),m2)) return true;
        break;
      }
    }
  }

  for (int i = 0; i < 2; i++) {
    for(Robot& rt: robots[i]) {
      if (((rt.pos.x-r->pos.x)*(rt.pos.x-r->pos.x)+
           (rt.pos.y-r->pos.y)*(rt.pos.y-r->pos.y)+
           (rt.pos.z-r->pos.z)*(rt.pos.z-r->pos.z))<COLISION_TEST_THRESHOLD) {
        if (&rt != r) {
          m2[12]=rt.pos.x;
          m2[13]=rt.pos.y;
          m2[14]=rt.pos.z;
          if (r->cmc.collision_simple(m1,&(rt.cmc),m2)) return true;
        }
      }
    }
  }

  return false;
}
