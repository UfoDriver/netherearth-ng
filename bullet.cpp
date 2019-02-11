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

#include "list.h"
#include "vector.h"
#include "quaternion.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "myglutaux.h"
#include "nether.h"
#include "bullet.h"

#include "glprintf.h"


extern int detaillevel;

Bullet::Bullet(): type(BULLET_CANNONS), step(0), angle(0), owner(0)
{
}


Bullet::Bullet(BULLET_TYPE type, Vector position, int angle, Robot *robot):
  type(type), step(0), pos(position), angle(angle), owner(robot)
{
}


CMC NETHER::BulletCMC(Bullet *b)
{
	CMC cmc;
	float m[16]={1,0,0,0,
				 0,1,0,0,
				 0,0,1,0,
				 0,0,0,1};

	/* compute CMC: */ 
	switch(b->type) {
	case 0:/* CANNON: */ 
		m[13]=0.2;
		cmc.expand(&(bullet_tile[0]->cmc),m);
		m[13]=-0.2;
		cmc.expand(&(bullet_tile[0]->cmc),m);
		break;
	case 1:/* MISSILES: */ 
		{
			Quaternion q;

			q.from_axis_angle(Vector(0,0,1),3.141592f);
			q.to_matrix(m);
			m[13]+=0.33;
			cmc.expand(&(bullet_tile[1]->cmc),m);
			m[13]-=0.66;
			cmc.expand(&(bullet_tile[1]->cmc),m);
		}
		break;
	case 2:/* PHASERS: */ 
		{
			Quaternion q;

			q.from_axis_angle(Vector(0,0,1),3.141592f/2);
			q.to_matrix(m);
			cmc.expand(&(bullet_tile[2]->cmc),m);
		}
		break;
	} /* switch */ 

	return cmc;
} /* BULLET::BULLET */ 


void Bullet::draw(bool shadows, Piece3DObject **bullet_tile, std::vector<Particle>& particles)
{
  switch(type) {
  case BULLET_CANNONS:
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle, 0, 0, 1);
      glTranslatef(0, 0.2, 0);
      bullet_tile[0]->draw(Color(0.2f, 0.2f, 0.2f));
      glTranslatef(0, -0.4, 0);
      bullet_tile[0]->draw(Color(0.2f, 0.2f, 0.2f));
      glPopMatrix();
    }
    break;
  case BULLET_MISSILES:
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle, 0, 0, 1);
      glRotatef(180, 0, 0, 1);
      glTranslatef(0, 0.33, 0);
      bullet_tile[1]->draw(Color(0.8f, 0.8f, 0.8f));
      glTranslatef(0, -0.66, 0);
      bullet_tile[1]->draw(Color(0.8f, 0.8f, 0.8f));
      glPopMatrix();
      if (detaillevel >= 4) {
        drawParticles(particles);
      }
    }
    break;
  case BULLET_PHASERS:
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle,0,0,1);
      glRotatef(90,0,0,1);

      if ((rand() % 4) !=0)
        bullet_tile[2]->draw_notexture(Color(1.0f, 0.5f, 1.0f, 0.9f));
      else
        bullet_tile[2]->draw_notexture(Color(1.0f, 1.0f, 1.0f, 0.5f));

      glPopMatrix();
    }
    break;
  }
}


void Bullet::drawParticles(std::vector<Particle>& particles)
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


bool NETHER::BulletCollision(Bullet *bullet,Robot **r)
{
	int i;
	List<Building> l;
	Building *b;
	List<Robot> l2;
	Robot *rt;
	float m1[16]={1,0,0,0,
				  0,1,0,0,
				  0,0,1,0,
				  bullet->pos.x,bullet->pos.y,0.3,1};
	float m2[16]={1,0,0,0,
				  0,1,0,0,
				  0,0,1,0,
				  0,0,0,1}; 

	*r=0;

	/* Collision with the ship: */ 
	/* Bullets do not collide with SHIP!: */ 
/*
	{
		float m2[16]={1,0,0,0,
				 	  0,1,0,0,
					  0,0,1,0,
					  shipp.x,shipp.y,shipp.z,1}; 
		if (bullet->cmc.collision_simple(m1,&(ship->cmc),m2)) return true;
	}
*/

	/* Collision with buildings: */  
	l.Instance(buildings);
	l.Rewind();
	while(l.Iterate(b)) {
		if (((b->pos.x-bullet->pos.x)*(b->pos.x-bullet->pos.x)+
			 (b->pos.y-bullet->pos.y)*(b->pos.y-bullet->pos.y)+
			 (b->pos.z-bullet->pos.z)*(b->pos.z-bullet->pos.z))<COLISION_TEST_THRESHOLD) {
			m2[12]=b->pos.x;
			m2[13]=b->pos.y;
			m2[14]=b->pos.z;

			switch(b->type) {
            case Building::B_FENCE:
              if (bullet->cmc.collision_simple(m1,&(building_tile[5]->cmc),m2)) return true;
              break;
            case Building::B_WALL1:
              if (bullet->cmc.collision_simple(m1,&(building_tile[0]->cmc),m2)) return true;
              break;
            case Building::B_WALL2:
              if (bullet->cmc.collision_simple(m1,&(building_tile[1]->cmc),m2)) return true;
              break;
            case Building::B_WALL3:
              if (bullet->cmc.collision_simple(m1,&(building_tile[2]->cmc),m2)) return true;
              break;
            case Building::B_WALL4:
              if (bullet->cmc.collision_simple(m1,&(building_tile[3]->cmc),m2)) return true;
              break;
            case Building::B_WALL5:
              if (bullet->cmc.collision_simple(m1,&(building_tile[4]->cmc),m2)) return true;
              break;
            case Building::B_WALL6:
              if (bullet->cmc.collision_simple(m1,&(building_tile[7]->cmc),m2)) return true;
              break;
            case Building::B_WARBASE:
              if (bullet->cmc.collision_simple(m1,&(building_tile[8]->cmc),m2)) return true;
              //m2[13]=b->pos.y-2;
              //m2[14]=b->pos.z+1;
              //if (b->owner!=0) if (bullet->cmc.collision_simple(m1,&(building_tile[6]->cmc),m2)) return true;
              break;
            case Building::B_FACTORY_ELECTRONICS:
              if (bullet->cmc.collision_simple(m1,&(building_tile[4]->cmc),m2)) return true;
              m2[12]=b->pos.x+0.5;
              m2[13]=b->pos.y+0.5;
              m2[14]=b->pos.z+1;
              if (bullet->cmc.collision_simple(m1,&(piece_tile[0][7]->cmc),m2)) return true;
              //m2[12]=b->pos.x;
              //m2[13]=b->pos.y-1;
              //if (b->owner!=0) if (bullet->cmc.collision_simple(m1,&(building_tile[6]->cmc),m2)) return true;
              break;
            case Building::B_FACTORY_NUCLEAR:
              if (bullet->cmc.collision_simple(m1,&(building_tile[4]->cmc),m2)) return true;
              m2[12]=b->pos.x+0.5;
              m2[13]=b->pos.y+0.5;
              m2[14]=b->pos.z+1;
              if (bullet->cmc.collision_simple(m1,&(piece_tile[0][6]->cmc),m2)) return true;
              //m2[12]=b->pos.x;
              //m2[13]=b->pos.y-1;
              //if (b->owner!=0) if (bullet->cmc.collision_simple(m1,&(building_tile[6]->cmc),m2)) return true;
              break;
            case Building::B_FACTORY_PHASERS:
              if (bullet->cmc.collision_simple(m1,&(building_tile[4]->cmc),m2)) return true;
              m2[12]=b->pos.x+0.5;
              m2[13]=b->pos.y+0.5;
              m2[14]=b->pos.z+1;
              if (bullet->cmc.collision_simple(m1,&(piece_tile[0][5]->cmc),m2)) return true;
              //m2[12]=b->pos.x;
              //m2[13]=b->pos.y-1;
              //if (b->owner!=0) if (bullet->cmc.collision_simple(m1,&(building_tile[6]->cmc),m2)) return true;
              break;
            case Building::B_FACTORY_MISSILES:
              if (bullet->cmc.collision_simple(m1,&(building_tile[4]->cmc),m2)) return true;
              m2[12]=b->pos.x+0.5;
              m2[13]=b->pos.y+0.5;
              m2[14]=b->pos.z+1;
              if (bullet->cmc.collision_simple(m1,&(piece_tile[0][4]->cmc),m2)) return true;
              //m2[12]=b->pos.x;
              //m2[13]=b->pos.y-1;
              //if (b->owner!=0) if (bullet->cmc.collision_simple(m1,&(building_tile[6]->cmc),m2)) return true;
              break;
            case Building::B_FACTORY_CANNONS:
              if (bullet->cmc.collision_simple(m1,&(building_tile[4]->cmc),m2)) return true;
              m2[12]=b->pos.x+0.5;
              m2[13]=b->pos.y+0.5;
              m2[14]=b->pos.z+1;
              if (bullet->cmc.collision_simple(m1,&(piece_tile[0][3]->cmc),m2)) return true;
              //m2[12]=b->pos.x;
              //m2[13]=b->pos.y-1;
              //if (b->owner!=0) if (bullet->cmc.collision_simple(m1,&(building_tile[6]->cmc),m2)) return true;
              break;
            case Building::B_FACTORY_CHASSIS:
              if (bullet->cmc.collision_simple(m1,&(building_tile[4]->cmc),m2)) return true;
              m2[12]=b->pos.x+0.5;
              m2[13]=b->pos.y+0.5;
              m2[14]=b->pos.z+1;
              if (bullet->cmc.collision_simple(m1,&(piece_tile[0][1]->cmc),m2)) return true;
              //m2[12]=b->pos.x;
              //m2[13]=b->pos.y-1;
              //if (b->owner!=0) if (bullet->cmc.collision_simple(m1,&(building_tile[6]->cmc),m2)) return true;
              break;
			} /* switch */ 
		} /* if */ 
	} /* while */ 

	/* Collision with the robots: */ 
	for(i=0;i<2;i++) {
		l2.Instance(robots[i]);
		l2.Rewind();
		while(l2.Iterate(rt)) {
			if (((rt->pos.x-bullet->pos.x)*(rt->pos.x-bullet->pos.x)+
				 (rt->pos.y-bullet->pos.y)*(rt->pos.y-bullet->pos.y)+
				 (rt->pos.z-bullet->pos.z)*(rt->pos.z-bullet->pos.z))<COLISION_TEST_THRESHOLD) {
				if (rt!=bullet->owner) {
					m2[12]=rt->pos.x;
					m2[13]=rt->pos.y;
					m2[14]=rt->pos.z; 
					if (bullet->cmc.collision_simple(m1,&(rt->cmc),m2)) {
						*r=rt;
						return true;
					} /* if */ 
				} /* if */ 
			} /* if */ 
		} /* while */ 
	} /* while */ 

	return false;
} /* NETHER::BulletCollision */ 
