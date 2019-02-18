#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>
#include "string.h"
#include <stdlib.h>
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

/* Artificial Intelligence variables: */ 
/*

int *discreetmap;	->	- Will keep a discreet copy of the map, telling whether a robot can occupy a position
						or not. 
						- Each cell of the original map, transform to a set of 2x2 cells of this new map, so
						  each 2x2 square of this cells correspond to one of the positions where a robot can 
						  be.

int *bk_discreetmap;	-> just contains the background.

AI_OPERATOR *searchmap;	-> In this map is where the search process will be performed.

*/

const int WE_SEARCH_DEPTH=12;	/* Search depth for robots with ELECTRONICS	*/ 
//const int WE_SEARCH_DEPTH=80;	/* Search depth for robots with ELECTRONICS	*/ 
const int WOE_SEARCH_DEPTH=4; /* Search depth for robots without ELECTRONICS	*/ 

const int xd[4]={ 1, 0,-1, 0};
const int yd[4]={ 0, 1, 0,-1};
const int xd2[4]={ 0, 0,-1, 0};
const int yd2[4]={ 0, 0, 0,-1};
const int xd3[4]={ 1, 0, 1, 0};
const int yd3[4]={ 0, 1, 0, 1};
const int dangle[4]={0,90,180,270};


extern FILE *debug_fp;

void robot_zone(Vector pos,int *x,int *y,int *dx,int *dy)
{
	*x=int((pos.x-0.5)/0.5);
	*y=int((pos.y-0.5)/0.5);

	if ((int(pos.x*256)%128)==0) *dx=2;
					 	    else *dx=3;
	if ((int(pos.y*256)%128)==0) *dy=2;
						    else *dy=3;
} /* robot_zone */ 


void fill_zone(std::vector<int>& map, int w, int val, int x, int y, int dx, int dy)
{
  for (int i = 0; i < dx; i++) {
    for (int j = 0; j < dy; j++) {
      map[(y + j) * w + (x + i)] = val;
    }
  }
}


void NETHER::AI_precomputations(void)
{
  discreetmap.clear();
  discreetmap.resize(map.width() * 2 * map.height() * 2, 0);
  bk_discreetmap.clear();
  bk_discreetmap.resize(map.width() * 2 * map.height() * 2);
  searchmap.resize(map.width() * 2 * map.height() * 2);
  attackmap.resize(map.width() * 2 * map.height() * 2);
  for (int i = 0; i < map.width() * 2 * map.height() * 2; i++) {
    AIOperator op;
    op.used = false;
    searchmap[i] = op;
    attackmap[i] = 0;
  }

  /* Setup the terrains for the new map: */
  for (int y = 0; y < map.height(); y++) {
    for(int x = 0; x < map.width(); x++) {
      fill_zone(discreetmap, map.width() * 2, map.terrain(float(x), float(y)), x * 2, y * 2, 2, 2);
      fill_zone(bk_discreetmap, map.width() * 2, map.terrain(float(x), float(y)), x * 2, y * 2, 2, 2);
    }
  }

  for (const Building& b: map.buildings) {
    fill_zone(discreetmap, map.width() * 2, T_BUILDING, int(b.pos.x / 0.5), int(b.pos.y / 0.5), 2, 2);
  }
}


void NETHER::AI_deleteprecomputations(void)
{
  discreetmap.clear();
  bk_discreetmap.clear();
  searchmap.clear();
}


void NETHER::AI_removebuilding(Vector pos)
{
  int x = int(pos.x / 0.5);
  int y = int(pos.y / 0.5);

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j  <2; j++) {
      discreetmap[(y + j) * (map.width() * 2) + (x + i)] = bk_discreetmap[(y + j) * (map.width() * 2) + (x + i)];
    }
  }
}


void NETHER::AI_release(void)
{
  discreetmap.clear();
}


void NETHER::AI_newrobot(Vector pos,int owner)
{
  int x,y,dx,dy;
  robot_zone(pos,&x,&y,&dx,&dy);
  if (owner == 0)
    fill_zone(discreetmap, map.width() * 2, T_ROBOT, x, y, dx, dy);
  else
    fill_zone(discreetmap, map.width() * 2, T_EROBOT, x, y, dx, dy);
}


int NETHER::AI_killrobot(Vector pos)
{
  int x, y, dx, dy;
  robot_zone(pos, &x, &y, &dx, &dy);
  int owner = discreetmap[y * (map.width() * 2) + x];
  for (int i = 0; i < dx; i++) {
    for(int j = 0; j < dy; j++) {
      discreetmap[(y + j) * (map.width() * 2) + (x + i)] = bk_discreetmap[(y + j) * (map.width() * 2) + (x + i)];
    }
  }
  return owner;
}


void NETHER::AI_moverobot(Vector oldpos, Vector newpos, int owner)
{
  AI_killrobot(oldpos);
  AI_newrobot(newpos, owner);
}


void NETHER::AI_availableoperators(const Robot& robot, std::vector<AIOperator>& l)
{
  int x, y, dx, dy;

  robot_zone(robot.pos, &x, &y, &dx, &dy);

  for (int i = 0; i < 4; i++) {
    int terrain = AI_WorseMapTerrain(x + xd2[i], y + yd2[i], 2 + xd3[i], 2 + yd3[i]);
    if (terrain <= T_HOLE && robot.walkable(terrain)) {
      /* Rotation cost: */
      int dif = dangle[i] - robot.angle;
      if (dif > 360) dif -= 360;
      if (dif < 0) dif += 360;
      int n_turns = 0;
      if (dif == 90 || dif == 270) n_turns=1;
      if (dif == 180) n_turns = 2;
      int cost = n_turns * int(90.f / robot.robotRotationSpeed(terrain));

      /* Displacement cost: */
      terrain = AI_WorseMapTerrain(x + xd[i], y + yd[i], 2, 2);
      cost += int(0.5 / robot.robotSpeed(terrain));

      /* Build a new AI_operator: */
      AIOperator op;
      op.cost = cost;
      op.previous = -1;
      op.deadend = false;
      if (dif == 0) op.first_robotop = ROBOTOP_FORWARD;
      if (dif == 90) op.first_robotop = ROBOTOP_RIGHT;
      if (dif == 180) op.first_robotop = ROBOTOP_RIGHT;
      if (dif == 270) op.first_robotop = ROBOTOP_LEFT;
      op.newpos = Vector(robot.pos.x + xd[i] * 0.5, robot.pos.y + yd[i] * 0.5, robot.pos.z);
      l.push_back(op);
    }
  }
}


bool NETHER::AI_expandoperators(int x, int y, int angle, const Robot& robot, int previous,
                                int oldcost, int depth)
{
	int terrain,terrain2;
	int n_turns;
	int dif;
	int cost;
	bool deadend=true;

	for (int i = 0; i < 4; i++) {
		int newpos = previous+xd[i]+yd[i]*(map.width()*2);
		if (newpos!=searchmap[previous].previous) {
			terrain=AI_WorseMapTerrain(x+xd2[i],y+yd2[i],2+xd3[i],2+yd3[i]);
			if (terrain<=T_HOLE &&
				robot.walkable(terrain)) {
				cost=oldcost;

				/* Rotation cost: */ 
				dif = dangle[i] - angle;
				if (dif>360) dif-=360;
				if (dif<0) dif+=360;
				n_turns=0;
				if (dif==90 || dif==270) n_turns=1;
				if (dif==180) n_turns=2;

				terrain2=AI_WorseMapTerrain(x,y,2,2);
				cost+=n_turns*int(90/float(robot.robotRotationSpeed(terrain2)));

				/* Displacement cost: */ 
				cost+=int(0.5/robot.robotSpeed(terrain));

				/* Replace the an old operator by a new one: */ 
				if (!searchmap[newpos].used ||
					searchmap[newpos].cost>cost) {

					/* Set the new AI_operator: */  
					searchmap[newpos].used=true;
					searchmap[newpos].cost=cost;
					searchmap[newpos].previous=previous;
					searchmap[newpos].deadend=false;
					if (dif==0) searchmap[newpos].first_robotop=ROBOTOP_FORWARD;
					if (dif==90) searchmap[newpos].first_robotop=ROBOTOP_RIGHT;
					if (dif==180) searchmap[newpos].first_robotop=ROBOTOP_RIGHT;
					if (dif==270) searchmap[newpos].first_robotop=ROBOTOP_LEFT;
					searchmap[newpos].newpos=Vector((x+xd[i])*0.5,(y+yd[i])*0.5,0);

					/* Continue the search process: */ 
					if (depth>0) {
						if (!AI_expandoperators(x+xd[i],y+yd[i],dangle[i],robot,newpos,cost,depth-1)) {
							deadend=false;
						} /* if */ 
					} else {
						deadend=false;
					} /* if */ 
				} /* if */ 
			} /* if */ 
		} /* if */ 
	} /* for */ 

	searchmap[previous].deadend=deadend;

	return deadend;
} /* NETHER::AI_expandoperators */ 


int NETHER::AI_searchengine(const Robot& robot, int goaltype, Vector goalpos, int depth)
{
  int x, y, dx, dy;
  /* Expand the search tree: */
  robot_zone(robot.pos, &x, &y, &dx, &dy);

  /* Build a new AI_operator: */
  int offs = y * (map.width() * 2) + x;
  searchmap[offs].used = true;
  searchmap[offs].cost = 0;
  searchmap[offs].previous = -1;
  searchmap[offs].first_robotop = ROBOTOP_NONE;
  searchmap[offs].newpos = robot.pos;
  searchmap[offs].deadend = false;
  AI_expandoperators(x, y, robot.angle, robot, y * (map.width() * 2) + x, 0, depth);

  /* ADVANCE PROGRAM: */
  if (goaltype==Robot::PROGRAM_ADVANCE) {
    double further;
    int mincost;
    bool first = true;
    AIOperator* op;
    AIOperator* bestop = 0;

    for(int i = -depth; i < depth; i++) {
      for(int j = -depth; j < depth; j++) {
        if ((x + i) >= 0 && (x + i) < (map.width() * 2) &&
            (y + j) >= 0 && (y + j) < (map.height() * 2) &&
            (i != 0 || j != 0)) {
          op = &searchmap[(y + j) * (map.width() * 2) + (x + i)];
          if (op->used) {
            if (first ||
                (op->deadend == bestop->deadend &&
                 (op->newpos.y > further ||
                  (op->newpos.y == further && op->cost < mincost))) ||
                (!op->deadend && bestop->deadend)) {
              bestop = op;
              further = op->newpos.y;
              mincost = op->cost;
              first = false;
            }
          }
        }
      }
    }

    if (bestop != 0) {
      int rop;
      AIOperator *prev;

      prev = bestop;
      while (bestop->previous != -1 && searchmap[bestop->previous].used) {
        prev = bestop;
        bestop = &searchmap[bestop->previous];
      }
      rop = prev->first_robotop;
      AI_resetsearch(robot.pos, depth);
      return rop;
    } else {
      AI_resetsearch(robot.pos, depth);
      return ROBOTOP_NONE;
    }

    AI_resetsearch(robot.pos, depth);
    if (bestop != 0)
      return bestop->first_robotop;
    else
      return ROBOTOP_NONE;
  }

  /* RETREAT PROGRAM: */
  if (goaltype==Robot::PROGRAM_RETREAT) {
    double further;
    int mincost;
    bool first = true;
    AIOperator* op;
    AIOperator* bestop = 0;

    for (int i = -depth; i < depth; i++) {
      for(int j = -depth; j < depth; j++) {
        if ((x + i) >= 0 && (x + i) < (map.width() * 2) &&
            (y+j)>=0 && (y+j)<(map.height()*2) &&
					(i!=0 || j!=0)) {
					op=&searchmap[(y+j)*(map.width()*2)+(x+i)];
					if (op->used) {
						if (first ||
							(op->deadend==bestop->deadend &&
							 (op->newpos.y<further ||
							  (op->newpos.y==further && op->cost<mincost))) ||
							(!op->deadend && bestop->deadend)) {
							bestop=op;
							further=op->newpos.y;
							mincost=op->cost;
							first=false;
						} /*if */ 
					} /* if */ 
				} /* if */ 
			} /* for */ 
		} /* for */ 

		if (bestop!=0) {
			int rop;
			AIOperator *prev;

			prev=bestop;
			while(bestop->previous!=-1 && searchmap[bestop->previous].used) {
				prev=bestop;
				bestop=&searchmap[bestop->previous];
			} /* while */ 
			rop=prev->first_robotop;

			AI_resetsearch(robot.pos,depth);
			return rop;
		} else {
			AI_resetsearch(robot.pos,depth);
			return ROBOTOP_NONE;
		} /* if */ 

		AI_resetsearch(robot.pos,depth);
		if (bestop!=0) return bestop->first_robotop;
				  else return ROBOTOP_NONE;
	} /* if */ 

	/* CAPTURE PROGRAM: */ 
	if (goaltype==Robot::PROGRAM_CAPTURE) {
		int i,j;
		double closer;
		int mincost;
		bool first=true;
		AIOperator *op,*bestop=0;

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Search engine for CAPTURE START\n");
	fflush(debug_fp);
#endif

		for(int i=-depth;i<depth;i++) {
			for(int j=-depth;j<depth;j++) {
				if ((x+i)>=0 && (x+i)<(map.width()*2) &&
					(y+j)>=0 && (y+j)<(map.height()*2) &&
					(i!=0 || j!=0)) {
					op=&searchmap[(y+j)*(map.width()*2)+(x+i)];
					if (op->used) {
						if (first ||
							((op->deadend==bestop->deadend || (op->newpos+Vector(0.5,0.5,0)-goalpos).norma()==0) &&
							 ((op->newpos+Vector(0.5,0.5,0)-goalpos).norma()<closer ||
							  ((op->newpos+Vector(0.5,0.5,0)-goalpos).norma()==closer && op->cost<mincost))) ||
							(!op->deadend && bestop->deadend && closer!=0)) {
							bestop=op;
							closer=(op->newpos+Vector(0.5,0.5,0)-goalpos).norma();
							mincost=op->cost;
							first=false;
						} /*if */ 
					} /* if */ 
				} /* if */ 
			} /* for */ 
		} /* for */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Search engine for CAPTURE FINISHED\n");
	fflush(debug_fp);
#endif

		if (bestop!=0) {
			int rop;
			AIOperator *prev;

			prev=bestop;
			while(bestop->previous!=-1 && searchmap[bestop->previous].used) {
				prev=bestop;
				bestop=&searchmap[bestop->previous];
			} /* while */ 
			rop=prev->first_robotop;

			AI_resetsearch(robot.pos,depth);
			return rop;
		} else {
			AI_resetsearch(robot.pos,depth);
			return ROBOTOP_NONE;
		} /* if */ 

		AI_resetsearch(robot.pos,depth);
		if (bestop!=0) return bestop->first_robotop;
				  else return ROBOTOP_NONE;
	} /* if */ 


	/* DESTROY PROGRAM: */ 
	if (goaltype==Robot::PROGRAM_DESTROY) {
		int i,j;
		double closer;
		AIOperator *op;
		int mincost,mincost2;
		bool first=true,first2=true;
		AIOperator *bestop=0;	/* Best operator to get closer to the nearest robot */ 
		AIOperator *bestop2=0;	/* Best operator to reach an attaking position		*/ 

		for(i=-depth;i<depth;i++) {
			for(j=-depth;j<depth;j++) {
				if ((x+i)>=0 && (x+i)<(map.width()*2) &&
					(y+j)>=0 && (y+j)<(map.height()*2) &&
					(i!=0 || j!=0)) {
					op=&searchmap[(y+j)*(map.width()*2)+(x+i)];
					if (op->used) {
						if (first ||
							((op->deadend==bestop->deadend || (op->newpos+Vector(0.5,0.5,0)-goalpos).norma()==0) &&
							 ((op->newpos+Vector(0.5,0.5,0)-goalpos).norma()<closer ||
							  ((op->newpos+Vector(0.5,0.5,0)-goalpos).norma()==closer && op->cost<mincost))) ||
							(!op->deadend && bestop->deadend && closer!=0)) {
							bestop=op;
							closer=(op->newpos+Vector(0.5,0.5,0)-goalpos).norma();
							mincost=op->cost;
							first=false;
						} /*if */ 
						if (attackmap[(y+j)*(map.width()*2)+(x+i)]!=0 ||
							attackmap[(y+j+1)*(map.width()*2)+(x+i)]!=0 ||
							attackmap[(y+j)*(map.width()*2)+(x+i+1)]!=0 ||
							attackmap[(y+j+1)*(map.width()*2)+(x+i+1)]!=0) {
							if (first2 ||
								op->cost<mincost2) {
								bestop2=op;
								mincost2=op->cost;
								first2=false;
							} /*if */ 
						} /* if */ 
					} /* if */ 
				} /* if */ 
			} /* for */ 
		} /* for */ 

		/* It's better to reach an attacking position if this is possible: */ 
		if (bestop2!=0) bestop=bestop2;

		if (bestop!=0) {
			int rop;
			AIOperator *prev;

			prev=bestop;
			while(bestop->previous!=-1 && searchmap[bestop->previous].used) {
				prev=bestop;
				bestop=&searchmap[bestop->previous];
			} /* while */ 
			rop=prev->first_robotop;

			AI_resetsearch(robot.pos,depth);
			return rop;
		} else {
			AI_resetsearch(robot.pos,depth);
			return ROBOTOP_NONE;
		} /* if */ 

		AI_resetsearch(robot.pos, depth);
		if (bestop!=0) return bestop->first_robotop;
				  else return ROBOTOP_NONE;
	} /* if */ 

	return ROBOTOP_NONE;
} /* AI_searchengine */ 


void NETHER::AI_resetsearch(Vector pos, int depth)
{
  int x, y, dx, dy;

  robot_zone(pos, &x, &y, &dx, &dy);

  for (int i = -depth; i < depth; i++) {
    for(int j = -depth; j < depth; j++) {
      if ((x + i) >= 0 && (x + i) < (map.width() * 2) &&
          (y + j) >= 0 && (y + j) < (map.height() * 2)) {
        searchmap[(y + j) * (map.width() * 2) + (x + i)].used = false;
      }
    }
  }
}


int NETHER::AI_WorseMapTerrain(int x,int y,int dx,int dy)
{
	int t=T_GRASS,t2;
	int i,j;

	for(i=0;i<dx;i++) {
		for(j=0;j<dy;j++) {
			if (x+i<0 || x+i>=map.width()*2 ||
				y+j<0 || y+j>=map.height()*2) return T_OUT;
			t2=discreetmap[(y+j)*(map.width()*2)+x+i];
			if (t2>t) t=t2;
		} /* if */ 
	} /* for */ 

	return t;
} /* NETHER::AI_WorseMapTerrain */ 


int NETHER::AI_program_advance(Robot& robot, int player)
{
  std::vector<AIOperator> operators;
  Vector tmp_goal;

  int op = AI_program_stopdefend(robot, &tmp_goal, player);
  if (op != ROBOTOP_NONE) return op;

  int type = AI_killrobot(robot.pos);

  AI_availableoperators(robot, operators);

  if (operators.size()) {
    if (robot.hasElectronics()) {
      op = AI_searchengine(robot, Robot::PROGRAM_ADVANCE, Vector(0, 0, 0), WE_SEARCH_DEPTH);
    } else {
      if ((rand() % 4) != 0) {
        op = AI_searchengine(robot, Robot::PROGRAM_ADVANCE, Vector(0, 0, 0), WOE_SEARCH_DEPTH);
      } else {
        AI_rankoperators_advance(operators);
        op = AI_chooseoperator(operators, 8).first_robotop;
      }
    }
  }

  /* Reconstruct the decreet map: */
  if (type == T_ROBOT)
    AI_newrobot(robot.pos, 0);
  else
    AI_newrobot(robot.pos, 2);

  return op;
}


int NETHER::AI_program_retreat(Robot& robot, int player)
{
  int type;
  std::vector<AIOperator> operators;

  Vector tmp_goal;
  int op = AI_program_stopdefend(robot, &tmp_goal, player);
  if (op != ROBOTOP_NONE) return op;

  type = AI_killrobot(robot.pos);

  AI_availableoperators(robot, operators);

  if (operators.size()) {
    /* Choose one operator: */
    if (robot.hasElectronics()) {
      op = AI_searchengine(robot, Robot::PROGRAM_RETREAT, Vector(0, 0, 0), WE_SEARCH_DEPTH);
    } else {
      if ((rand() % 4) != 0) {
        op = AI_searchengine(robot, Robot::PROGRAM_RETREAT, Vector(0, 0, 0), WOE_SEARCH_DEPTH);
      } else {
        AI_rankoperators_retreat(operators);
        op = AI_chooseoperator(operators, 8).first_robotop;
      }
    }
  }

  /* Reconstruct the decreet map: */
  if (type == T_ROBOT)
    AI_newrobot(robot.pos, 0);
  else
    AI_newrobot(robot.pos, 1);

  return op;
}


int NETHER::AI_program_capture(Robot& robot, Vector *program_goal, int player)
{
  /* First of all, delete the robot from the discreet map: */
  std::vector<AIOperator> l;

  int op = AI_program_stopdefend(robot, program_goal, player);
  if (op != ROBOTOP_NONE) return op;

  int type = AI_killrobot(robot.pos);
  AI_availableoperators(robot, l);

  if (l.size()) {
    /* Choose one operator: */
    {
      /* Seek a goal: */
      bool anygoal = false;
      float distance, minimumdistance;

      *program_goal = Vector(-1, -1, -1);

      for (const Building& b: map.buildings) {
        if (robot.program_parameter.as_int == Robot::P_PARAM_WARBASES &&
            b.type == Building::TYPE::WARBASE &&
            b.owner != player &&
            AI_WorseMapTerrain(int((b.pos.x + 2.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
          distance = float(((b.pos + Vector(2.5, 0.5, 0)) - robot.pos).norma());
          if (!anygoal || distance < minimumdistance) {
            anygoal = true;
            minimumdistance = distance;
            *program_goal = b.pos + Vector(2.5, 0.5, 0);
          }
        }
        if (robot.program_parameter.as_int == Robot::P_PARAM_NFACTORIES &&
            (b.type == Building::TYPE::FACTORY_ELECTRONICS ||
             b.type == Building::TYPE::FACTORY_NUCLEAR ||
             b.type == Building::TYPE::FACTORY_PHASERS ||
             b.type == Building::TYPE::FACTORY_MISSILES ||
             b.type == Building::TYPE::FACTORY_CANNONS ||
             b.type == Building::TYPE::FACTORY_CHASSIS)
            && b.owner == 0 &&
            AI_WorseMapTerrain(int((b.pos.x + 1.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
          distance = float(((b.pos + Vector(1.5, 0.5,0 )) - robot.pos).norma());
          if (!anygoal || distance < minimumdistance) {
            anygoal = true;
            minimumdistance = distance;
            *program_goal = b.pos + Vector(1.5, 0.5, 0);
          }
        }
        if (robot.program_parameter.as_int == Robot::P_PARAM_EFACTORIES &&
            (b.type == Building::TYPE::FACTORY_ELECTRONICS ||
             b.type == Building::TYPE::FACTORY_NUCLEAR ||
             b.type == Building::TYPE::FACTORY_PHASERS ||
             b.type == Building::TYPE::FACTORY_MISSILES ||
             b.type == Building::TYPE::FACTORY_CANNONS ||
             b.type == Building::TYPE::FACTORY_CHASSIS) &&
            b.owner!=0 &&
            b.owner!=player &&
            AI_WorseMapTerrain(int((b.pos.x + 1.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
          distance=float(((b.pos + Vector(1.5, 0.5, 0)) - robot.pos).norma());
          if (!anygoal || distance < minimumdistance) {
            anygoal = true;
            minimumdistance = distance;
            *program_goal = b.pos + Vector(1.5, 0.5, 0);
          }
        }
      }
    }

    if (program_goal->x != -1 &&
        (*program_goal) != robot.pos) {
      if (robot.hasElectronics()) {
        op = AI_searchengine(robot, Robot::PROGRAM_CAPTURE, *program_goal, WE_SEARCH_DEPTH);
      } else {
        if ((rand() % 4) != 0) {
          op = AI_searchengine(robot, Robot::PROGRAM_CAPTURE, *program_goal, WOE_SEARCH_DEPTH);
        } else {
          AI_rankoperators_capture(l, *program_goal);
          op = AI_chooseoperator(l, 8).first_robotop;
        }
      }
    }
  }

  /* Reconstruct the decreet map: */
  if (type==T_ROBOT)
    AI_newrobot(robot.pos,0);
  else AI_newrobot(robot.pos, 1);

  return op;
}


int NETHER::AI_RealShotPaths(int x,int y,int player,int persistence)
{
	int rsp=0;
//	int persistence=CANNON_PERSISTENCE;

	for (int i = 2;i < int((persistence*BULLET_SPEED)/0.5)+2 && (x+i<map.width()*2); i++) {
		if (discreetmap[x+i+y*(map.width()*2)]==T_BUILDING ||
			discreetmap[x+i+(y+1)*(map.width()*2)]==T_BUILDING) break;
		if (player==1) {
			if (discreetmap[x+i+y*(map.width()*2)]==T_ROBOT ||
				discreetmap[x+i+(y+1)*(map.width()*2)]==T_ROBOT) break;
			if (discreetmap[x+i+y*(map.width()*2)]==T_EROBOT ||
				discreetmap[x+i+(y+1)*(map.width()*2)]==T_EROBOT) {
				rsp|=1;
				break;
			} /* if */ 
		} else {
			if (discreetmap[x+i+y*(map.width()*2)]==T_EROBOT ||
				discreetmap[x+i+(y+1)*(map.width()*2)]==T_EROBOT) break;
			if (discreetmap[x+i+y*(map.width()*2)]==T_ROBOT ||
				discreetmap[x+i+(y+1)*(map.width()*2)]==T_ROBOT) {
				rsp|=1;
				break;
			} /* if */ 
		} /* if */ 
	} /* for */ 

	for (int i = 1;i < int((persistence*BULLET_SPEED)/0.5)+1 && (x-i>=0); i++) {
		if (discreetmap[x-i+y*(map.width()*2)]==T_BUILDING ||
			discreetmap[x-i+(y+1)*(map.width()*2)]==T_BUILDING) break;
		if (player==1) {
			if (discreetmap[x-i+y*(map.width()*2)]==T_ROBOT ||
				discreetmap[x-i+(y+1)*(map.width()*2)]==T_ROBOT) break;
			if (discreetmap[x-i+y*(map.width()*2)]==T_EROBOT ||
				discreetmap[x-i+(y+1)*(map.width()*2)]==T_EROBOT) {
				rsp|=4;
				break;
			} /* if */ 
		} else {
			if (discreetmap[x-i+y*(map.width()*2)]==T_EROBOT ||
				discreetmap[x-i+(y+1)*(map.width()*2)]==T_EROBOT) break;
			if (discreetmap[x-i+y*(map.width()*2)]==T_ROBOT ||
				discreetmap[x-i+(y+1)*(map.width()*2)]==T_ROBOT) {
				rsp|=4;
				break;
			} /* if */ 
		} /* if */ 
	} /* for */ 

	for (int i = 2; i < int((persistence*BULLET_SPEED)/0.5)+2 && (y+i<map.height()*2); i++) {
		if (discreetmap[x+(y+i)*(map.width()*2)]==T_BUILDING ||
			discreetmap[(x+1)+(y+i)*(map.width()*2)]==T_BUILDING) break;
		if (player==1) {
			if (discreetmap[x+(y+i)*(map.width()*2)]==T_ROBOT ||
				discreetmap[(x+1)+(y+i)*(map.width()*2)]==T_ROBOT) break;
			if (discreetmap[x+(y+i)*(map.width()*2)]==T_EROBOT ||
				discreetmap[(x+1)+(y+i)*(map.width()*2)]==T_EROBOT) {
				rsp|=2;
				break;
			} /* if */ 
		} else {
			if (discreetmap[x+(y+i)*(map.width()*2)]==T_EROBOT ||
				discreetmap[(x+1)+(y+i)*(map.width()*2)]==T_EROBOT) break;
			if (discreetmap[x+(y+i)*(map.width()*2)]==T_ROBOT ||
				discreetmap[(x+1)+(y+i)*(map.width()*2)]==T_ROBOT) {
				rsp|=2;
				break;
			} /* if */ 
		} /* if */ 
	} /* for */ 

	for (int i = 1; i < int((persistence*BULLET_SPEED)/0.5)+1 && (y-i>=0); i++) {
		if (discreetmap[x+(y-i)*(map.width()*2)]==T_BUILDING ||
			discreetmap[(x+1)+(y-i)*(map.width()*2)]==T_BUILDING) break;
		if (player==1) {
			if (discreetmap[x+(y-i)*(map.width()*2)]==T_ROBOT ||
				discreetmap[(x+1)+(y-i)*(map.width()*2)]==T_ROBOT) break;
			if (discreetmap[x+(y-i)*(map.width()*2)]==T_EROBOT ||
				discreetmap[(x+1)+(y-i)*(map.width()*2)]==T_EROBOT) {
				rsp|=8;
				break;
			} /* if */ 
		} else {
			if (discreetmap[x+(y-i)*(map.width()*2)]==T_EROBOT ||
				discreetmap[(x+1)+(y-i)*(map.width()*2)]==T_EROBOT) break;
			if (discreetmap[x+(y-i)*(map.width()*2)]==T_ROBOT ||
				discreetmap[(x+1)+(y-i)*(map.width()*2)]==T_ROBOT) {
				rsp|=8;
				break;
			} /* if */ 
		} /* if */ 
	} /* for */ 

	return rsp;
} /* NETHER::AI_RealShotPaths */ 


int NETHER::AI_program_destroy(Robot& robot, Vector *program_goal, int player)
{
  /* First of all, delete the robot from the discreet map: */
  int op = ROBOTOP_NONE;
  int type = AI_killrobot(robot.pos);
  std::vector<AIOperator> lops;

  AI_availableoperators(robot, lops);

  if (lops.size()) {
    /* Choose one operator: */
    if (robot.program_parameter.as_int != Robot::P_PARAM_ROBOTS) {
      /* Seek a goal: */
      bool anygoal = false;
      float distance, minimumdistance;

      *program_goal = Vector(-1, -1, -1);

      for (const Building& b: map.buildings) {
        if (robot.program_parameter.as_int == Robot::P_PARAM_WARBASES &&
            b.type == Building::TYPE::WARBASE &&
            b.owner!=player &&
            AI_WorseMapTerrain(int((b.pos.x + 2.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
          distance = float(((b.pos + Vector(2.5, 0.5, 0)) - robot.pos).norma());
          if (!anygoal || distance < minimumdistance) {
            anygoal = true;
            minimumdistance = distance;
            *program_goal = b.pos+Vector(2.5,0.5,0);
          }
        }
        if (robot.program_parameter.as_int == Robot::P_PARAM_EFACTORIES
            && (b.type == Building::TYPE::FACTORY_ELECTRONICS ||
                b.type == Building::TYPE::FACTORY_NUCLEAR ||
                b.type == Building::TYPE::FACTORY_PHASERS ||
                b.type == Building::TYPE::FACTORY_MISSILES ||
                b.type == Building::TYPE::FACTORY_CANNONS ||
                b.type == Building::TYPE::FACTORY_CHASSIS) && b.owner != 0 && b.owner != player &&
            AI_WorseMapTerrain(int((b.pos.x + 1.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
          distance=float(((b.pos + Vector(1.5, 0.5, 0)) - robot.pos).norma());
          if (!anygoal || distance < minimumdistance) {
            anygoal = true;
            minimumdistance = distance;
            *program_goal = b.pos + Vector(1.5,0.5,0);
          }
        }
      }

      if (program_goal->x != -1 &&
          (*program_goal) != robot.pos) {
        if (robot.hasElectronics()) {
          op = AI_searchengine(robot, Robot::PROGRAM_CAPTURE, *program_goal, WE_SEARCH_DEPTH);
        } else {
          if ((rand() % 4) != 0) {
            op = AI_searchengine(robot, Robot::PROGRAM_CAPTURE, *program_goal, WOE_SEARCH_DEPTH);
          } else {
            AI_rankoperators_capture(lops, *program_goal);
            op = AI_chooseoperator(lops, 8).first_robotop;
          }
        }
      } else {
        if (program_goal->x != -1) op = ROBOTOP_NUCLEAR;
      }

    } else {
      /* Find the nearest position to destroy an enemy robot: */
      int x, y, dx, dy;
      bool collided;
      bool first = true;
      float distance;
      int persistence = CANNON_PERSISTENCE;
      if (robot.hasMissiles()) persistence = MISSILE_PERSISTENCE;

      *program_goal = Vector(-1,-1,-1);
      std::fill(attackmap.begin(), attackmap.end(), 0);

      /* Find the nearest FIRE position: */
      for (Robot* r: robots[2 - player]) {
        if (first ||
            (*program_goal - robot.pos).norma() < distance) {
          first = false;
          distance = float((*program_goal - robot.pos).norma());
          *program_goal = r->pos;
        }

        robot_zone(r->pos, &x, &y, &dx, &dy);
        for (int i = 0; i < dx; i++) {
          for (int j = 0; j < dy; j++) {
            collided = false;
            for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
              if (x + i + k < 0 || x + i + k >= map.width() * 2 ||
                  y + j < 0 || y + j >= map.height() * 2 ||
                  discreetmap[(y + j) * (map.width() * 2) + (x + i + k)] > 3) {
                collided = true;
              } else {
                attackmap[(y + j) * (map.width() * 2) + (x + i + k)] |= 4;
              }
            }

            collided = false;
            for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
              if (x + i - k < 0 || x + i - k >= map.width() * 2 ||
                  y + j < 0 || y + j >= map.height() * 2 ||
                  discreetmap[(y + j) * (map.width() * 2) + (x + i - k)] > 3) {
                collided = true;
              } else {
                attackmap[(y + j) * (map.width() * 2) + (x + i - k)] |= 1;
              }
            }

            collided = false;
            for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
              if (x + i < 0 || x + i >= map.width() * 2 ||
                  y + j + k < 0 || y + j + k >= map.height() * 2 ||
                  discreetmap[(y + j + k) * (map.width() * 2) + (x + i)] > 3) {
                collided = true;
              } else {
                attackmap[(y + j + k) * (map.width() * 2) + (x + i)] |= 8;
              }
            }

            collided = false;
            for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
              if (x + i < 0 || x + i >= map.width() * 2 ||
                  y + j - k < 0 || y + j - k >= map.height() * 2 ||
                  discreetmap[(y + j - k) * (map.width() * 2) + (x + i)] >3) {
                collided = true;
              } else {
                attackmap[(y + j - k) * (map.width() * 2) + (x + i)] |= 2;
              }
            }
          }
        }
      }

      if (!first) {
        robot_zone(robot.pos, &x, &y, &dx, &dy);
        if ((attackmap[y * (map.width() * 2) + x] !=0 ||
             attackmap[(y + 1) * (map.width() * 2) + x] != 0 ||
             attackmap[y * (map.width() * 2) + x + 1] !=0 ||
             attackmap[(y + 1) * (map.width() * 2) + x + 1] != 0)) {
          int prsp = 0, mrsp = 0,crsp = 0,rsp = 0;
          if (robot.hasPhasers()) prsp = AI_RealShotPaths(x, y, player, PHASER_PERSISTENCE);
          if (robot.hasMissiles()) mrsp = AI_RealShotPaths(x, y, player, MISSILE_PERSISTENCE);
          if (robot.hasCannons()) crsp = AI_RealShotPaths(x, y, player, CANNON_PERSISTENCE);
          rsp = prsp | mrsp | crsp;

          if (rsp != 0) {
            int dirmask = 0;
            if (robot.angle == 0) dirmask = 1;
            if (robot.angle == 90) dirmask = 2;
            if (robot.angle == 180) dirmask = 4;
            if (robot.angle == 270) dirmask = 8;
            if ((rsp & dirmask) != 0) {
              if ((prsp & dirmask) != 0) {
                op = ROBOTOP_PHASERS;
              } else {
                if ((mrsp & dirmask) != 0) {
                  op = ROBOTOP_MISSILES;
                } else {
                  op = ROBOTOP_CANNONS;
                }
              }
            } else {
              dirmask *= 2;
              if (dirmask >= 16) dirmask = 1;
              if ((rsp & dirmask) != 0) {
                op = ROBOTOP_RIGHT;
              } else {
                op = ROBOTOP_LEFT;
              }
            }
          } else {
            if (robot.hasElectronics()) {
              op = AI_searchengine(robot, Robot::PROGRAM_DESTROY, *program_goal, WE_SEARCH_DEPTH);
            } else {
              if ((rand() % 4) != 0) {
                op = AI_searchengine(robot, Robot::PROGRAM_DESTROY, *program_goal, WOE_SEARCH_DEPTH);
              } else {
                AI_rankoperators_capture(lops, *program_goal);
                op = AI_chooseoperator(lops, 8).first_robotop;
              }
            }
          }
        } else {
          if (robot.hasElectronics()) {
            op = AI_searchengine(robot, Robot::PROGRAM_DESTROY, *program_goal, WE_SEARCH_DEPTH);
          } else {
            if ((rand() % 4) != 0) {
              op = AI_searchengine(robot, Robot::PROGRAM_DESTROY, *program_goal, WOE_SEARCH_DEPTH);
            } else {
              AI_rankoperators_capture(lops, *program_goal);
              op = AI_chooseoperator(lops, 8).first_robotop;
            }
          }
        }
      } else {
        /* There are no enemy robots: */
        op = ROBOTOP_NONE;
      }
    }
  }

  /* Reconstruct the decreet map: */
  if (type == T_ROBOT)
    AI_newrobot(robot.pos, 0);
  else
    AI_newrobot(robot.pos, 1);

  return op;
}


int NETHER::AI_program_stopdefend(Robot& robot, Vector *program_goal, int player)
{
  /* First of all, delete the robot from the discreet map: */
  int op = ROBOTOP_NONE;
  int type = AI_killrobot(robot.pos);
  std::vector<AIOperator> lops;

  AI_availableoperators(robot, lops);

  if (lops.size()) {
    /* Choose one operator: */

    /* Find the nearest position to destroy an enemy robot: */
    int x, y, dx, dy;
    bool collided;
    int persistence = CANNON_PERSISTENCE;
    if (robot.hasMissiles()) persistence = MISSILE_PERSISTENCE;

    *program_goal = Vector(-1, -1, -1);
    std::fill(attackmap.begin(), attackmap.end(), 0);

    /* Find the nearest FIRE position: */
    for (Robot* r: robots[2 - player]) {
      robot_zone(r->pos, &x, &y, &dx, &dy);
      for (int i = 0; i < dx; i++) {
        for (int j = 0; j < dy; j++) {
          collided = false;
          for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
            if (x + i + k < 0 || x + i + k >= map.width() * 2 ||
                y + j < 0 || y + j >= map.height() * 2 ||
                discreetmap[(y + j) * (map.width() * 2) + (x + i + k)] > 3) {
              collided = true;
            } else {
              attackmap[(y + j) * (map.width() * 2) + (x + i + k)] |= 4;
            }
          }

          collided = false;
          for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
            if (x + i - k < 0 || x + i - k >= map.width() * 2 ||
                y + j < 0 || y + j >= map.height() * 2 ||
                discreetmap[(y + j) * (map.width() * 2) + (x + i - k)] >3) {
              collided = true;
            } else {
              attackmap[(y + j) * (map.width() * 2) + (x + i - k)] |= 1;
            }
          }

          collided = false;
          for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
            if (x + i < 0 || x + i >= map.width() * 2 ||
                y + j + k < 0 || y + j + k >= map.height() * 2 ||
                discreetmap[(y + j + k) * (map.width() * 2) + (x + i)] >3) {
              collided = true;
            } else {
              attackmap[(y + j + k) * (map.width() * 2) + (x + i)] |= 8;
            }
          }

          collided = false;
          for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
            if (x + i < 0 || x + i >= map.width() * 2 ||
                y + j - k < 0 || y + j - k >= map.height() * 2 ||
                discreetmap[(y + j - k) * (map.width() * 2) + (x + i)] > 3) {
              collided = true;
            } else {
              attackmap[(y + j - k) * (map.width() * 2) + (x + i)] |= 2;
            }
          }
        }
      }
    }


    if (robots[2 - player].size()) {
      robot_zone(robot.pos, &x, &y, &dx, &dy);
      if ((attackmap[y * (map.width() * 2) + x] != 0 ||
           attackmap[(y + 1) * (map.width() * 2) + x] != 0 ||
           attackmap[y * (map.width() * 2) + x + 1] != 0 ||
           attackmap[(y + 1) * (map.width() * 2) + x + 1] != 0)) {
        int prsp = 0, mrsp = 0, crsp = 0, rsp = 0;
        if (robot.pieces[2]) prsp = AI_RealShotPaths(x, y, player, PHASER_PERSISTENCE);
        if (robot.pieces[1]) mrsp = AI_RealShotPaths(x, y, player, MISSILE_PERSISTENCE);
        if (robot.pieces[0]) crsp = AI_RealShotPaths(x, y, player, CANNON_PERSISTENCE);
        rsp = prsp | mrsp | crsp;

        if (rsp != 0) {
          int dirmask = 0;

          /* This is just to make the program not to think that the robot doesn't have any goal: */
          *program_goal = Vector(0, 0, 0);

          if (robot.angle == 0) dirmask = 1;
          if (robot.angle == 90) dirmask = 2;
          if (robot.angle == 180) dirmask = 4;
          if (robot.angle == 270) dirmask = 8;
          if ((rsp & dirmask) != 0) {
            if ((prsp & dirmask) !=0 ) {
              op = ROBOTOP_PHASERS;
            } else {
              if ((mrsp & dirmask) != 0) {
                op = ROBOTOP_MISSILES;
              } else {
                op = ROBOTOP_CANNONS;
              }
            }
          } else {
            dirmask *= 2;
            if (dirmask >= 16) dirmask = 1;
            if ((rsp & dirmask) != 0) {
              op = ROBOTOP_RIGHT;
            } else {
              op = ROBOTOP_LEFT;
            }
          }
        } else {
          /* There are no enemy robots at sight: */
          op = ROBOTOP_NONE;
        }
      } else {
        /* There are no enemy robots at sight: */
        op = ROBOTOP_NONE;
      }
    } else {
      /* There are no enemy robots: */
      op = ROBOTOP_NONE;
    }
  }

  /* Reconstruct the decreet map: */
  if (type == T_ROBOT)
    AI_newrobot(robot.pos, 0);
  else
    AI_newrobot(robot.pos, 1);

  return op;
}


void NETHER::AI_rankoperators_advance(std::vector<AIOperator>& l)
{
  std::stable_sort(l.begin(), l.end(),
                   [](auto& op1, auto& op2) {
                     if (op1.newpos.y > op2.newpos.y)
                       return true;
                     else if (op1.newpos.y < op2.newpos.y)
                       return false;
                     else
                       return op1.cost < op2.cost;
                   });
}


void NETHER::AI_rankoperators_retreat(std::vector<AIOperator>& l)
{
    std::stable_sort(l.begin(), l.end(),
                     [](auto& op1, auto& op2) {
                       if (op1.newpos.y > op2.newpos.y)
                         return true;
                       else if (op1.newpos.y < op2.newpos.y)
                         return false;
                       else
                         return op1.cost > op2.cost;
                     });
}


void NETHER::AI_rankoperators_capture(std::vector<AIOperator>& l,Vector goal)
{
  std::stable_sort(l.begin(), l.end(),
                   [goal](auto& op1, auto& op2) {
                     float dist1 = (op1.newpos - goal).norma();
                     float dist2 = (op2.newpos - goal).norma();
                     int c1 = (0.1) < (dist1 - dist2);
                     int c2 = 0;
                     if ((0.1) > (dist1 - dist2) &&
                         (-0.1) < (dist1-dist2))
                       c2 = op2.cost < op1.cost;
                     return c1 || c2;
                   });
}


const AIOperator NETHER::AI_chooseoperator(std::vector<AIOperator>& l, int factor)
{
  if (factor == 0) {
    return *l.begin();
  } else {
    auto iter = l.begin();
    while (iter != l.end() && (rand() % factor) == 0)
      iter++;
    if (iter != l.end()) {
      return *iter;
    } else {
      return *l.end();
    }
  }
}


int NETHER::AI_robothere(Vector pos)
{
  int x = int(pos.x/0.5);
  int y = int(pos.y/0.5);
  int robot = discreetmap[y * (map.width() * 2) + x];

  if (robot != T_ROBOT && robot != T_EROBOT) robot = 0;
  return robot;
}
