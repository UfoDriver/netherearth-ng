#include <algorithm>
#include "ai.h"
#include "constants.h"
#include "nether.h"
#include "robot.h"

extern int level;

const int xd[4] = { 1, 0,-1, 0};
const int yd[4]={ 0, 1, 0,-1};
const int xd2[4] = { 0, 0,-1, 0};
const int yd2[4] = { 0, 0, 0,-1};
const int xd3[4]={ 1, 0, 1, 0};
const int yd3[4]={ 0, 1, 0, 1};
const int dangle[4]={0,90,180,270};


void AI::fillZone(std::vector<int>& map, int w, int val, int x, int y, int dx, int dy)
{
  for (int i = 0; i < dx; i++) {
    for (int j = 0; j < dy; j++) {
      map[(y + j) * w + (x + i)] = val;
    }
  }
}


void AI::makePrecomputations()
{
  discreetmap.clear();
  discreetmap.resize(map->width() * 2 * map->height() * 2, 0);
  bk_discreetmap.clear();
  bk_discreetmap.resize(map->width() * 2 * map->height() * 2);
  searchmap.resize(map->width() * 2 * map->height() * 2);
  attackmap.resize(map->width() * 2 * map->height() * 2);
  for (int i = 0; i < map->width() * 2 * map->height() * 2; i++) {
    AIOperator op;
    op.used = false;
    searchmap[i] = op;
    attackmap[i] = 0;
  }

  /* Setup the terrains for the new map: */
  for (int y = 0; y < map->height(); y++) {
    for(int x = 0; x < map->width(); x++) {
      fillZone(discreetmap, map->width() * 2, map->terrain(float(x), float(y)), x * 2, y * 2, 2, 2);
      fillZone(bk_discreetmap, map->width() * 2, map->terrain(float(x), float(y)), x * 2, y * 2, 2, 2);
    }
  }

  for (const Building& b: map->buildings) {
    fillZone(discreetmap, map->width() * 2, T_BUILDING, int(b.pos.x / 0.5), int(b.pos.y / 0.5), 2, 2);
  }
}


void AI::deletePrecomputations()
{
  discreetmap.clear();
  bk_discreetmap.clear();
  searchmap.clear();
}


int AI::robotHere(const Vector& pos)
{
  int x = int(pos.x/0.5);
  int y = int(pos.y/0.5);
  int robot = discreetmap[y * (map->width() * 2) + x];

  if (robot != T_ROBOT && robot != T_EROBOT) robot = 0;
  return robot;
}


int AI::killRobot(const Vector& pos)
{
  int x, y, dx, dy;
  robotZone(pos, &x, &y, &dx, &dy);
  int owner = discreetmap[y * (map->width() * 2) + x];
  for (int i = 0; i < dx; i++) {
    for(int j = 0; j < dy; j++) {
      discreetmap[(y + j) * (map->width() * 2) + (x + i)] = bk_discreetmap[(y + j) * (map->width() * 2) + (x + i)];
    }
  }
  return owner;
}


void AI::robotZone(const Vector& pos, int *x, int *y, int *dx, int *dy)
{
  *x = int((pos.x - 0.5) / 0.5);
  *y = int((pos.y - 0.5) / 0.5);

  if ((int(pos.x * 256) % 128) == 0)
    *dx = 2;
  else
    *dx = 3;
  if ((int(pos.y * 256) % 128) == 0)
    *dy = 2;
  else
    *dy = 3;
}


void AI::release(void)
{
  discreetmap.clear();
}


void AI::newRobot(const Vector& pos, const int owner)
{
  int x, y, dx, dy;
  robotZone(pos,&x,&y,&dx,&dy);
  if (owner == 0)
    fillZone(discreetmap, map->width() * 2, T_ROBOT, x, y, dx, dy);
  else
    fillZone(discreetmap, map->width() * 2, T_EROBOT, x, y, dx, dy);
}


void AI::moveRobot(const Vector& oldpos, const Vector& newpos, const int owner)
{
  killRobot(oldpos);
  newRobot(newpos, owner);
}


int AI::worseMapTerrain(const int x, const int y, const int dx, const int dy)
{
  int t = T_GRASS, t2;
  for (int i = 0; i < dx; i++) {
    for (int j = 0; j < dy; j++) {
      if (x + i < 0 || x + i >= map->width() * 2 ||
          y + j < 0 || y + j >= map->height() * 2)
        return T_OUT;
      t2 = discreetmap[(y + j) * (map->width() * 2) + x + i];
      if (t2 > t) t = t2;
    }
  }
  return t;
}


void AI::removeBuilding(const Vector& pos)
{
  int x = int(pos.x / 0.5);
  int y = int(pos.y / 0.5);

  for (int i = 0; i < 2; i++) {
    for (int j = 0; j  <2; j++) {
      discreetmap[(y + j) * (map->width() * 2) + (x + i)] = bk_discreetmap[(y + j) * (map->width() * 2) + (x + i)];
    }
  }
}


void AI::enemy()
{
	Building *in_danger_warbase=0;
	STATE state = STATE::EXPANDING;
	int nrobots[3]={0,0,0};	/* EXPANDING / ATTACING / DEFENDING */ 
	Vector mean_factory_position;
	Building *closest_to_factories_warbase=0;
	float distance_to_factories;
	Building *closest_to_enemy_warbase=0;
	float distance_to_enemy;
	int factories[3]={0,0,0};

//	if (fp==0) {
//		fp=fopen("enemyai.txt","w+");
//	} /* if */  

//	fprintf(fp,"---------------------------------- \n");
//	fprintf(fp,"G: %i\nE: %i\nN: %i\nP: %i\nM: %i\nC: %i\nC: %i\n",
//			resources[1][0],resources[1][1],resources[1][2],
//			resources[1][3],resources[1][4],resources[1][5],resources[1][6]);

	/* Analize the situation and determine strategy: */ 
	/*
	AI_STATE_EXPANDING
	AI_STATE_FIGHTING
	AI_STATE_DEFENDING
	AI_STATE_CONQUERING
	*/ 
	/* Rules: */ 
	/*
	1.- If there is a robot force near a WARBASE (closer than 10 squares) -> state = AI_STATE_DEFENDING
	2.- If there is no WARBASE in danger, and there are available FACTORIES near -> state = AI_STATE_EXPANDING
	2.- If there is no WARBASE in danger and there are enough CAPTURING robots working and
		there are more player robots than computer robots -> state = AI_FIGHTING
	4.- If there is no WARBASE in danger and there are enough CAPTURING robots working and
		there are less player robots than computer robots -> state = AI_CONQUERING
	*/ 

	Robot *tmpr;	// To test is the entrance to Warbases is free

	tmpr=new Robot();
	tmpr->traction=0;
	tmpr->pieces[0]=true;
	tmpr->pieces[1]=false;
	tmpr->pieces[2]=false;
	tmpr->pieces[3]=false;
	tmpr->pieces[4]=false;
	tmpr->angle=0;
	tmpr->program=Robot::PROGRAM_FORWARD;
	tmpr->op=ROBOTOP_NONE;
	tmpr->calculateCMC(Resources::pieceTiles[1]);
	tmpr->shipover=false;


    {
      int forces[2]={0,0};

      mean_factory_position=Vector(0,0,0);
      for (const Building& b: map->buildings) {
        if (b.type==Building::TYPE::FACTORY_ELECTRONICS ||
            b.type==Building::TYPE::FACTORY_NUCLEAR ||
            b.type==Building::TYPE::FACTORY_PHASERS ||
            b.type==Building::TYPE::FACTORY_MISSILES ||
            b.type==Building::TYPE::FACTORY_CANNONS) {
          factories[b.owner]++;
          if (b.owner!=2) mean_factory_position=mean_factory_position+b.pos;
        }
      }
      mean_factory_position=mean_factory_position/(factories[0]+factories[1]);

      for (Building& b: map->buildings) {
        if (b.type==Building::TYPE::WARBASE &&
            b.owner==2) {
          forces[0]=0;
          forces[1]=0;

          tmpr->pos=b.pos+Vector(2.5,0.5,0);
          if (!tmpr->checkCollision(map->buildings, map->robots, true, nether->getShip())) {
            /* Find the closest WARBASE to the available FACTORIES: */ 
            if (closest_to_factories_warbase==0 ||
                (closest_to_factories_warbase->pos-b.pos).norma()<distance_to_factories) {
              closest_to_factories_warbase=&b;
              distance_to_factories=float((closest_to_factories_warbase->pos-b.pos).norma());
            } /* if */ 

            /* Find the closest WARBASE to the enemy: */ 
            if (closest_to_enemy_warbase==0 ||
                closest_to_enemy_warbase->pos.y<distance_to_enemy) {
              closest_to_enemy_warbase=&b;
              distance_to_enemy=float(closest_to_enemy_warbase->pos.y);
            } /* if */ 
          } /* if */ 

          /* Test for WARBASEs in danger: */ 
          for(int i = 0; i < 2; i++) {
            for (Robot* r: map->robots[i]) {
              if ((r->pos-b.pos).norma() < 10.0) {
                /* Robot near: */ 
                forces[i]+= r->cost();

                if (i==1) {
                  if (forces[0]>forces[1] && 
                      (r->program!=Robot::PROGRAM_DESTROY ||
                       r->program_parameter.param != Robot::P_PARAM_ROBOTS)) {
                    r->program=Robot::PROGRAM_DESTROY;
                    r->program_parameter.param = Robot::P_PARAM_ROBOTS;
                    return;
                  } /* if */ 
                } /* if */ 
              } /* if */ 
            } /* while */ 
          } /* for */

          if (forces[0]>forces[1]) {
            state = STATE::DEFENDING;
            in_danger_warbase=&b;
          } /* if */ 
        } /* if */ 
      } /* while */

    }


	/* If the warbase in danger id blocked, build robots from another warbase: */ 
	if (in_danger_warbase!=0) {
		tmpr->pos=in_danger_warbase->pos+Vector(2.0,0.5,0);
		if (tmpr->checkCollision(map->buildings, map->robots, true, nether->getShip())) in_danger_warbase=closest_to_enemy_warbase;
	} /* if */ 

	delete tmpr;
	tmpr=0;

	/* If the enemy has no WARBASES, he cannot do anything: (???) */ 
	if (closest_to_factories_warbase==0) return;


	/* Count the number of robots: */ 
    for (Robot* r: map->robots[1]) {
      if (r->program==Robot::PROGRAM_CAPTURE) nrobots[0]++;
      if (r->program==Robot::PROGRAM_DESTROY) nrobots[1]++;
      if (r->program==Robot::PROGRAM_STOPDEFEND) nrobots[2]++;
    }

	if (in_danger_warbase!=0 &&
		(level>=2 ||
		 (level==1 && (rand()%2)==0) ||
		 (level==0 && (rand()%4)==0))) {
		Robot *r;
		/* Try to build a ROBOT to defend the WARBASE */ 
		
//		fprintf(fp,"Trying to BUILD a robot to DEFEND the WARBASE \n");

		r = enemyNewRobot(STATE::DEFENDING, in_danger_warbase->pos+Vector(2.5,0.5,0));
		if (r!=0) {
			
//			fprintf(fp,"Achieved.\n");

          r->program=Robot::PROGRAM_DESTROY;
          r->program_parameter.param = Robot::P_PARAM_ROBOTS;
		} /* if */ 
	} else {
		if (nrobots[2]>0 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) {
			/* There are too many robots in STOP & DEFEND: */ 
          for (Robot* r: map->robots[1]) {
            if (r->program==Robot::PROGRAM_STOPDEFEND) {
              if (nrobots[0]<6 && factories[2]<(factories[1]+factories[0]) && 
                  (map->robots[0].size()*2)<=nrobots[1]) {
                /* Convert the robot to a conquering one: */ 
                if (factories[1]>factories[0]) {
                  r->program=Robot::PROGRAM_CAPTURE;
                  r->program_parameter.param = Robot::P_PARAM_EFACTORIES;
                  return;
                } else {
                  r->program=Robot::PROGRAM_CAPTURE;
                  r->program_parameter.param = Robot::P_PARAM_NFACTORIES;
                  return;
                } /* if */ 
              } else {
                if ((map->robots[0].size()*2)>nrobots[1]) {
                  r->program=Robot::PROGRAM_DESTROY;
                  r->program_parameter.param =Robot::P_PARAM_ROBOTS;
                  return;
                } else {
                  r->program=Robot::PROGRAM_CAPTURE;
                  r->program_parameter.param = Robot::P_PARAM_WARBASES;
                  return;
                } /* if */ 
              } /* if */ 
            } /* if */ 
          }
		} /* if */ 
		/* Test for near FACTORIES and CAPTURING ROBOTS: */ 
		if (nrobots[0]<6 && factories[2]<(factories[1]+factories[0]) &&
			(map->robots[0].size()*2)<=nrobots[1]) {
			/* I need more conquering robots: */ 

			/* Try to make better robots as time passes: */ 
			switch(level) {
			case 0:
				if (nrobots[0]>=1 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<20) return;
				if (nrobots[0]>=1 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<25) return;
				if (nrobots[0]>=2 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<30) return;
				break;
			case 1:
				if (nrobots[0]>=1 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<20) return;
				if (nrobots[0]>=2 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<25) return;
				if (nrobots[0]>=3 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<30) return;
				break;
			default:
				if (nrobots[0]>=2 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<20) return;
				if (nrobots[0]>=3 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<25) return;
				if (nrobots[0]>=4 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<30) return;
				break;
			} /* switch */  

			Robot *r=enemyNewRobot(STATE::EXPANDING,closest_to_factories_warbase->pos+Vector(2.5,0.5,0));
//			fprintf(fp,"Trying to BUILD a robot to CONQUER FACTORIES \n");
			if (r!=0) {
				if (factories[1]>factories[0]) {			
					
//					fprintf(fp,"Achieved, it will conquer ENEMY FACTORIES\n");

					r->program=Robot::PROGRAM_CAPTURE;
					r->program_parameter.param = Robot::P_PARAM_EFACTORIES;
				} else {

//					fprintf(fp,"Achieved, it will conquer NEUTRAL FACTORIES\n");

					r->program=Robot::PROGRAM_CAPTURE;
					r->program_parameter.param = Robot::P_PARAM_NFACTORIES;
				} /* if */ 
			} /* if */ 
		} else {
			/* I've enough conquering robots: */ 
			if (nrobots[1]>10) {
				/* I've enough attacking robots, let's build a conquering one: */ 
//				fprintf(fp,"Trying to BUILD a robot to CONQUER WARBASES \n");
				
				if (nether->stats.resources[1][0]+nether->stats.resources[1][6]<40) return;
				Robot *r=enemyNewRobot(STATE::CONQUERING,closest_to_enemy_warbase->pos+Vector(2.5,0.5,0));
				if (r!=0) {

//					fprintf(fp,"Achieved.\n");

					r->program=Robot::PROGRAM_CAPTURE;
					r->program_parameter.param = Robot::P_PARAM_WARBASES;
				} /* if */ 
			} else {
				/* I need more attacking robots: */ 
				
				/* Try to make better robots as time passes: */ 
				switch(level) {
				case 0:
					if (nrobots[1]>=1 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<20) return;
					if (nrobots[1]>=1 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<25) return;
					if (nrobots[1]>=2 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<30) return;
					break;
				case 1:
					if (nrobots[1]>=1 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<20) return;
					if (nrobots[1]>=2 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<25) return;
					if (nrobots[1]>=3 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<30) return;
					break;
				default:
					if (nrobots[1]>=2 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<20) return;
					if (nrobots[1]>=3 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<25) return;
					if (nrobots[1]>=4 && nether->stats.resources[1][0]+nether->stats.resources[1][6]<30) return;
					break;
				} /* switch */ 

				Robot *r=enemyNewRobot(STATE::FIGHTING,closest_to_enemy_warbase->pos+Vector(2.5,0.5,0));

//				fprintf(fp,"Trying to BUILD a robot to ATTACK ROBOTS \n");
				
				if (r!=0) {

//					fprintf(fp,"Achieved.\n");
					
					r->program=Robot::PROGRAM_DESTROY;
					r->program_parameter.param = Robot::P_PARAM_ROBOTS;
				} /* if */ 
			} /* if */ 
		} /* if */ 
	} /* if */ 

}


Robot* AI::enemyNewRobot(const STATE state, const Vector& pos)
{
	int traction=0;
	bool pieces[5]={false,false,false,false,false};
	int rg=nether->stats.resources[1][R_GENERAL];

	if (rg+nether->stats.resources[1][R_CHASSIS]>30 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) {
		traction=2;
		rg-=10;
	} else {
		if (rg+nether->stats.resources[1][R_CHASSIS]>20 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) {
			traction=1;
			rg-=5;
		} else {
			traction=0;
			rg-=2;
		} /* if */ 
	} /* if */ 

	switch(state) {
	case STATE::EXPANDING:
		if (rg+nether->stats.resources[1][R_CANNONS]+nether->stats.resources[1][R_MISSILES]+nether->stats.resources[1][R_PHASERS]>40 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) {
			pieces[2]=true;
		} else {
			if (rg+nether->stats.resources[1][R_CANNONS]+nether->stats.resources[1][R_MISSILES]+nether->stats.resources[1][R_PHASERS]>20 &&
				(level>=2 ||
				(level==1 && (rand()%2)==0) ||
				(level==0 && (rand()%4)==0))) {
				pieces[1]=true;
			} else {
				pieces[0]=true;
			} /* if */ 
		} /* if */ 

		if (rg+nether->stats.resources[1][R_ELECTRONICS]>10) pieces[4]=true;
		break;
	case STATE::DEFENDING:
		if (rg+nether->stats.resources[1][R_PHASERS]>20 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) pieces[2]=true;
		if (rg+nether->stats.resources[1][R_MISSILES]>20 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) pieces[1]=true;
		if (rg+nether->stats.resources[1][R_CANNONS]>20 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) pieces[0]=true;
		if (!pieces[0] && !pieces[1] && !pieces[2]) pieces[0]=true;

		if (rg+nether->stats.resources[1][R_ELECTRONICS]>30) pieces[4]=true;
		break;
	case STATE::FIGHTING:
		if (rg+nether->stats.resources[1][R_PHASERS]>20 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) pieces[2]=true;
		if (rg+nether->stats.resources[1][R_MISSILES]>20 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) pieces[1]=true;
		if (rg+nether->stats.resources[1][R_CANNONS]>20 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) pieces[0]=true;
		if (!pieces[0] && !pieces[1] && !pieces[2]) pieces[0]=true;

		if (rg+nether->stats.resources[1][R_ELECTRONICS]>20) pieces[4]=true;
		break;
	case STATE::CONQUERING:
		if (rg+nether->stats.resources[1][R_CANNONS]+nether->stats.resources[1][R_MISSILES]+nether->stats.resources[1][R_PHASERS]>40 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) {
			pieces[2]=true;
		} else {
			if (rg+nether->stats.resources[1][R_CANNONS]+nether->stats.resources[1][R_MISSILES]+nether->stats.resources[1][R_PHASERS]>20 &&
				(level>=2 ||
				(level==1 && (rand()%2)==0) ||
				(level==0 && (rand()%4)==0))) {
				pieces[1]=true;
			} else {
				pieces[0]=true;
			} /* if */ 
		} /* if */ 

		if (rg+nether->stats.resources[1][R_ELECTRONICS]>10) pieces[4]=true;
		break;
	case STATE::DESTROYING:
		if (rg+nether->stats.resources[1][R_NUCLEAR]>40) pieces[3]=true;
		if (rg+nether->stats.resources[1][R_CANNONS]+nether->stats.resources[1][R_MISSILES]+nether->stats.resources[1][R_PHASERS]>40 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) {
			pieces[2]=true;
		} else {
			if (rg+nether->stats.resources[1][R_CANNONS]+nether->stats.resources[1][R_MISSILES]+nether->stats.resources[1][R_PHASERS]>20 &&
				(level>=2 ||
				(level==1 && (rand()%2)==0) ||
				(level==0 && (rand()%4)==0))) {
				pieces[1]=true;
			} else {
				pieces[0]=true;
			} /* if */ 
		} /* if */ 

		if (rg+nether->stats.resources[1][R_ELECTRONICS]>10 &&
			(level>=2 ||
			(level==1 && (rand()%2)==0) ||
			(level==0 && (rand()%4)==0))) pieces[4]=true;
		break;
	} /* switch */ 

	/* Build the robot: */ 
	{
		int i;
		int cost[7];

		Robot *r = new Robot();
		r->traction=traction;
		r->pieces[0]=pieces[0];
		r->pieces[1]=pieces[1];
		r->pieces[2]=pieces[2];
		r->pieces[3]=pieces[3];
		r->pieces[4]=pieces[4];
		r->cost(1, cost, nether->stats.resources);

		for(int i=0;i<7;i++) {
			if (nether->stats.resources[1][i]<cost[i]) {
				/* Not enough resources! */ 
				return 0;
			} /* if */ 
		} /* for */ 


		/* Valid robot, build it: */ 
		r->pos=pos;
		r->angle=0;
		r->program=Robot::PROGRAM_FORWARD;
		r->op=ROBOTOP_NONE;
		r->calculateCMC(Resources::pieceTiles[1]);
		r->shipover=false;

		if (!r->checkCollision(map->buildings, map->robots, true, nether->ship)) {
			map->robots[1].push_back(r);
			newRobot(r->pos,0);

			for(i=0;i<7;i++) nether->stats.resources[1][i]-=cost[i];

			return r;
		} else {
			delete r;
		} /* if */ 
	}

	return 0;

}


void AI::availableOperators(const Robot& robot, std::vector<AIOperator>& l)
{
  int x, y, dx, dy;

  robotZone(robot.pos, &x, &y, &dx, &dy);

  for (int i = 0; i < 4; i++) {
    int terrain = worseMapTerrain(x + xd2[i], y + yd2[i], 2 + xd3[i], 2 + yd3[i]);
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
      terrain = worseMapTerrain(x + xd[i], y + yd[i], 2, 2);
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


bool AI::expandOperators(const int x, const int y, const int angle, const Robot& robot, const int previous,
                                const int oldcost, const int depth)
{
	int terrain,terrain2;
	int n_turns;
	int dif;
	int cost;
	bool deadend=true;

	for (int i = 0; i < 4; i++) {
		int newpos = previous+xd[i]+yd[i]*(map->width()*2);
		if (newpos!=searchmap[previous].previous) {
			terrain=worseMapTerrain(x+xd2[i],y+yd2[i],2+xd3[i],2+yd3[i]);
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

				terrain2=worseMapTerrain(x,y,2,2);
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
						if (!expandOperators(x+xd[i],y+yd[i],dangle[i],robot,newpos,cost,depth-1)) {
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
}


int AI::searchEngine(const Robot& robot, const int goaltype, const Vector& goalpos, const int depth)
{
  int x, y, dx, dy;
  /* Expand the search tree: */
  robotZone(robot.pos, &x, &y, &dx, &dy);

  /* Build a new AI_operator: */
  int offs = y * (map->width() * 2) + x;
  searchmap[offs].used = true;
  searchmap[offs].cost = 0;
  searchmap[offs].previous = -1;
  searchmap[offs].first_robotop = ROBOTOP_NONE;
  searchmap[offs].newpos = robot.pos;
  searchmap[offs].deadend = false;
  expandOperators(x, y, robot.angle, robot, y * (map->width() * 2) + x, 0, depth);

  /* ADVANCE PROGRAM: */
  if (goaltype==Robot::PROGRAM_ADVANCE) {
    double further;
    int mincost;
    bool first = true;
    AIOperator* op;
    AIOperator* bestop = 0;

    for(int i = -depth; i < depth; i++) {
      for(int j = -depth; j < depth; j++) {
        if ((x + i) >= 0 && (x + i) < (map->width() * 2) &&
            (y + j) >= 0 && (y + j) < (map->height() * 2) &&
            (i != 0 || j != 0)) {
          op = &searchmap[(y + j) * (map->width() * 2) + (x + i)];
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
      resetSearch(robot.pos, depth);
      return rop;
    } else {
      resetSearch(robot.pos, depth);
      return ROBOTOP_NONE;
    }

    resetSearch(robot.pos, depth);
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
        if ((x + i) >= 0 && (x + i) < (map->width() * 2) &&
            (y+j)>=0 && (y+j)<(map->height()*2) &&
					(i!=0 || j!=0)) {
					op=&searchmap[(y+j)*(map->width()*2)+(x+i)];
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

			resetSearch(robot.pos,depth);
			return rop;
		} else {
			resetSearch(robot.pos,depth);
			return ROBOTOP_NONE;
		} /* if */ 

		resetSearch(robot.pos,depth);
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
				if ((x+i)>=0 && (x+i)<(map->width()*2) &&
					(y+j)>=0 && (y+j)<(map->height()*2) &&
					(i!=0 || j!=0)) {
					op=&searchmap[(y+j)*(map->width()*2)+(x+i)];
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

			resetSearch(robot.pos,depth);
			return rop;
		} else {
			resetSearch(robot.pos,depth);
			return ROBOTOP_NONE;
		} /* if */ 

		resetSearch(robot.pos,depth);
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
				if ((x+i)>=0 && (x+i)<(map->width()*2) &&
					(y+j)>=0 && (y+j)<(map->height()*2) &&
					(i!=0 || j!=0)) {
					op=&searchmap[(y+j)*(map->width()*2)+(x+i)];
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
						if (attackmap[(y+j)*(map->width()*2)+(x+i)]!=0 ||
							attackmap[(y+j+1)*(map->width()*2)+(x+i)]!=0 ||
							attackmap[(y+j)*(map->width()*2)+(x+i+1)]!=0 ||
							attackmap[(y+j+1)*(map->width()*2)+(x+i+1)]!=0) {
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

			resetSearch(robot.pos,depth);
			return rop;
		} else {
			resetSearch(robot.pos,depth);
			return ROBOTOP_NONE;
		} /* if */ 

		resetSearch(robot.pos, depth);
		if (bestop!=0) return bestop->first_robotop;
				  else return ROBOTOP_NONE;
	} /* if */ 

	return ROBOTOP_NONE;
}


void AI::resetSearch(const Vector& pos, const int depth)
{
  int x, y, dx, dy;

  robotZone(pos, &x, &y, &dx, &dy);

  for (int i = -depth; i < depth; i++) {
    for(int j = -depth; j < depth; j++) {
      if ((x + i) >= 0 && (x + i) < (map->width() * 2) &&
          (y + j) >= 0 && (y + j) < (map->height() * 2)) {
        searchmap[(y + j) * (map->width() * 2) + (x + i)].used = false;
      }
    }
  }
}


int AI::programAdvance(const Robot& robot, const int player)
{
  std::vector<AIOperator> operators;
  Vector tmp_goal;

  int op = programStopDefend(robot, &tmp_goal, player);
  if (op != ROBOTOP_NONE) return op;

  int type = killRobot(robot.pos);

  availableOperators(robot, operators);

  if (operators.size()) {
    if (robot.hasElectronics()) {
      op = searchEngine(robot, Robot::PROGRAM_ADVANCE, Vector(0, 0, 0), WE_SEARCH_DEPTH);
    } else {
      if ((rand() % 4) != 0) {
        op = searchEngine(robot, Robot::PROGRAM_ADVANCE, Vector(0, 0, 0), WOE_SEARCH_DEPTH);
      } else {
        rankOperatorsAdvance(operators);
        op = chooseOperator(operators, 8).first_robotop;
      }
    }
  }

  /* Reconstruct the decreet map: */
  if (type == T_ROBOT)
    newRobot(robot.pos, 0);
  else
    newRobot(robot.pos, 2);

  return op;
}


int AI::programRetreat(const Robot& robot, const int player)
{
  int type;
  std::vector<AIOperator> operators;

  Vector tmp_goal;
  int op = programStopDefend(robot, &tmp_goal, player);
  if (op != ROBOTOP_NONE) return op;

  type = killRobot(robot.pos);

  availableOperators(robot, operators);

  if (operators.size()) {
    /* Choose one operator: */
    if (robot.hasElectronics()) {
      op = searchEngine(robot, Robot::PROGRAM_RETREAT, Vector(0, 0, 0), WE_SEARCH_DEPTH);
    } else {
      if ((rand() % 4) != 0) {
        op = searchEngine(robot, Robot::PROGRAM_RETREAT, Vector(0, 0, 0), WOE_SEARCH_DEPTH);
      } else {
        rankOperatorsRetreat(operators);
        op = chooseOperator(operators, 8).first_robotop;
      }
    }
  }

  /* Reconstruct the decreet map: */
  if (type == T_ROBOT)
    newRobot(robot.pos, 0);
  else
    newRobot(robot.pos, 1);

  return op;
}


int AI::programStopDefend(const Robot& robot, Vector *program_goal, const int player)
{
  /* First of all, delete the robot from the discreet map: */
  int op = ROBOTOP_NONE;
  int type = killRobot(robot.pos);
  std::vector<AIOperator> lops;

  availableOperators(robot, lops);

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
    for (Robot* r: map->robots[2 - player]) {
      robotZone(r->pos, &x, &y, &dx, &dy);
      for (int i = 0; i < dx; i++) {
        for (int j = 0; j < dy; j++) {
          collided = false;
          for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
            if (x + i + k < 0 || x + i + k >= map->width() * 2 ||
                y + j < 0 || y + j >= map->height() * 2 ||
                discreetmap[(y + j) * (map->width() * 2) + (x + i + k)] > 3) {
              collided = true;
            } else {
              attackmap[(y + j) * (map->width() * 2) + (x + i + k)] |= 4;
            }
          }

          collided = false;
          for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
            if (x + i - k < 0 || x + i - k >= map->width() * 2 ||
                y + j < 0 || y + j >= map->height() * 2 ||
                discreetmap[(y + j) * (map->width() * 2) + (x + i - k)] >3) {
              collided = true;
            } else {
              attackmap[(y + j) * (map->width() * 2) + (x + i - k)] |= 1;
            }
          }

          collided = false;
          for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
            if (x + i < 0 || x + i >= map->width() * 2 ||
                y + j + k < 0 || y + j + k >= map->height() * 2 ||
                discreetmap[(y + j + k) * (map->width() * 2) + (x + i)] >3) {
              collided = true;
            } else {
              attackmap[(y + j + k) * (map->width() * 2) + (x + i)] |= 8;
            }
          }

          collided = false;
          for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
            if (x + i < 0 || x + i >= map->width() * 2 ||
                y + j - k < 0 || y + j - k >= map->height() * 2 ||
                discreetmap[(y + j - k) * (map->width() * 2) + (x + i)] > 3) {
              collided = true;
            } else {
              attackmap[(y + j - k) * (map->width() * 2) + (x + i)] |= 2;
            }
          }
        }
      }
    }


    if (map->robots[2 - player].size()) {
      robotZone(robot.pos, &x, &y, &dx, &dy);
      if ((attackmap[y * (map->width() * 2) + x] != 0 ||
           attackmap[(y + 1) * (map->width() * 2) + x] != 0 ||
           attackmap[y * (map->width() * 2) + x + 1] != 0 ||
           attackmap[(y + 1) * (map->width() * 2) + x + 1] != 0)) {
        int prsp = 0, mrsp = 0, crsp = 0, rsp = 0;
        if (robot.pieces[2]) prsp = realShotPaths(x, y, player, PHASER_PERSISTENCE);
        if (robot.pieces[1]) mrsp = realShotPaths(x, y, player, MISSILE_PERSISTENCE);
        if (robot.pieces[0]) crsp = realShotPaths(x, y, player, CANNON_PERSISTENCE);
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
    newRobot(robot.pos, 0);
  else
    newRobot(robot.pos, 1);

  return op;
}


int AI::programCapture(const Robot& robot, Vector *program_goal, const int player)
{
  /* First of all, delete the robot from the discreet map: */
  std::vector<AIOperator> l;

  int op = programStopDefend(robot, program_goal, player);
  if (op != ROBOTOP_NONE) return op;

  int type = killRobot(robot.pos);
  availableOperators(robot, l);

  if (l.size()) {
    /* Choose one operator: */
    {
      /* Seek a goal: */
      bool anygoal = false;
      float distance, minimumdistance;

      *program_goal = Vector(-1, -1, -1);

      for (const Building& b: map->buildings) {
        if (robot.program_parameter.as_int == Robot::P_PARAM_WARBASES &&
            b.type == Building::TYPE::WARBASE &&
            b.owner != player &&
            worseMapTerrain(int((b.pos.x + 2.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
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
            worseMapTerrain(int((b.pos.x + 1.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
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
            worseMapTerrain(int((b.pos.x + 1.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
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
        op = searchEngine(robot, Robot::PROGRAM_CAPTURE, *program_goal, WE_SEARCH_DEPTH);
      } else {
        if ((rand() % 4) != 0) {
          op = searchEngine(robot, Robot::PROGRAM_CAPTURE, *program_goal, WOE_SEARCH_DEPTH);
        } else {
          rankOperatorsCapture(l, *program_goal);
          op = chooseOperator(l, 8).first_robotop;
        }
      }
    }
  }

  /* Reconstruct the decreet map: */
  if (type==T_ROBOT)
    newRobot(robot.pos,0);
  else
    newRobot(robot.pos, 1);

  return op;
}


int AI::programDestroy(const Robot& robot, Vector *program_goal, const int player)
{
  /* First of all, delete the robot from the discreet map: */
  int op = ROBOTOP_NONE;
  int type = killRobot(robot.pos);
  std::vector<AIOperator> lops;

  availableOperators(robot, lops);

  if (lops.size()) {
    /* Choose one operator: */
    if (robot.program_parameter.as_int != Robot::P_PARAM_ROBOTS) {
      /* Seek a goal: */
      bool anygoal = false;
      float distance, minimumdistance;

      *program_goal = Vector(-1, -1, -1);

      for (const Building& b: map->buildings) {
        if (robot.program_parameter.as_int == Robot::P_PARAM_WARBASES &&
            b.type == Building::TYPE::WARBASE &&
            b.owner!=player &&
            worseMapTerrain(int((b.pos.x + 2.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
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
            worseMapTerrain(int((b.pos.x + 1.0) / 0.5), int(b.pos.y / 0.5), 2, 2) <= T_HOLE) {
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
          op = searchEngine(robot, Robot::PROGRAM_CAPTURE, *program_goal, WE_SEARCH_DEPTH);
        } else {
          if ((rand() % 4) != 0) {
            op = searchEngine(robot, Robot::PROGRAM_CAPTURE, *program_goal, WOE_SEARCH_DEPTH);
          } else {
            rankOperatorsCapture(lops, *program_goal);
            op = chooseOperator(lops, 8).first_robotop;
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
      for (Robot* r: map->robots[2 - player]) {
        if (first ||
            (*program_goal - robot.pos).norma() < distance) {
          first = false;
          distance = float((*program_goal - robot.pos).norma());
          *program_goal = r->pos;
        }

        robotZone(r->pos, &x, &y, &dx, &dy);
        for (int i = 0; i < dx; i++) {
          for (int j = 0; j < dy; j++) {
            collided = false;
            for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
              if (x + i + k < 0 || x + i + k >= map->width() * 2 ||
                  y + j < 0 || y + j >= map->height() * 2 ||
                  discreetmap[(y + j) * (map->width() * 2) + (x + i + k)] > 3) {
                collided = true;
              } else {
                attackmap[(y + j) * (map->width() * 2) + (x + i + k)] |= 4;
              }
            }

            collided = false;
            for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
              if (x + i - k < 0 || x + i - k >= map->width() * 2 ||
                  y + j < 0 || y + j >= map->height() * 2 ||
                  discreetmap[(y + j) * (map->width() * 2) + (x + i - k)] > 3) {
                collided = true;
              } else {
                attackmap[(y + j) * (map->width() * 2) + (x + i - k)] |= 1;
              }
            }

            collided = false;
            for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
              if (x + i < 0 || x + i >= map->width() * 2 ||
                  y + j + k < 0 || y + j + k >= map->height() * 2 ||
                  discreetmap[(y + j + k) * (map->width() * 2) + (x + i)] > 3) {
                collided = true;
              } else {
                attackmap[(y + j + k) * (map->width() * 2) + (x + i)] |= 8;
              }
            }

            collided = false;
            for (int k = 1; !collided && k < int((persistence * BULLET_SPEED) / 0.5); k++) {
              if (x + i < 0 || x + i >= map->width() * 2 ||
                  y + j - k < 0 || y + j - k >= map->height() * 2 ||
                  discreetmap[(y + j - k) * (map->width() * 2) + (x + i)] >3) {
                collided = true;
              } else {
                attackmap[(y + j - k) * (map->width() * 2) + (x + i)] |= 2;
              }
            }
          }
        }
      }

      if (!first) {
        robotZone(robot.pos, &x, &y, &dx, &dy);
        if ((attackmap[y * (map->width() * 2) + x] !=0 ||
             attackmap[(y + 1) * (map->width() * 2) + x] != 0 ||
             attackmap[y * (map->width() * 2) + x + 1] !=0 ||
             attackmap[(y + 1) * (map->width() * 2) + x + 1] != 0)) {
          int prsp = 0, mrsp = 0,crsp = 0,rsp = 0;
          if (robot.hasPhasers()) prsp = realShotPaths(x, y, player, PHASER_PERSISTENCE);
          if (robot.hasMissiles()) mrsp = realShotPaths(x, y, player, MISSILE_PERSISTENCE);
          if (robot.hasCannons()) crsp = realShotPaths(x, y, player, CANNON_PERSISTENCE);
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
              op = searchEngine(robot, Robot::PROGRAM_DESTROY, *program_goal, WE_SEARCH_DEPTH);
            } else {
              if ((rand() % 4) != 0) {
                op = searchEngine(robot, Robot::PROGRAM_DESTROY, *program_goal, WOE_SEARCH_DEPTH);
              } else {
                rankOperatorsCapture(lops, *program_goal);
                op = chooseOperator(lops, 8).first_robotop;
              }
            }
          }
        } else {
          if (robot.hasElectronics()) {
            op = searchEngine(robot, Robot::PROGRAM_DESTROY, *program_goal, WE_SEARCH_DEPTH);
          } else {
            if ((rand() % 4) != 0) {
              op = searchEngine(robot, Robot::PROGRAM_DESTROY, *program_goal, WOE_SEARCH_DEPTH);
            } else {
              rankOperatorsCapture(lops, *program_goal);
              op = chooseOperator(lops, 8).first_robotop;
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
    newRobot(robot.pos, 0);
  else
    newRobot(robot.pos, 1);

  return op;
}


void AI::rankOperatorsAdvance(std::vector<AIOperator>& l)
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


void AI::rankOperatorsRetreat(std::vector<AIOperator>& l)
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


void AI::rankOperatorsCapture(std::vector<AIOperator>& l,Vector goal)
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


const AIOperator AI::chooseOperator(std::vector<AIOperator>& l, const int factor)
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


int AI::realShotPaths(const int x, const int y, const int player, const int persistence)
{
	int rsp=0;

	for (int i = 2;i < int((persistence*BULLET_SPEED)/0.5)+2 && (x+i<map->width()*2); i++) {
		if (discreetmap[x+i+y*(map->width()*2)]==T_BUILDING ||
			discreetmap[x+i+(y+1)*(map->width()*2)]==T_BUILDING) break;
		if (player==1) {
			if (discreetmap[x+i+y*(map->width()*2)]==T_ROBOT ||
				discreetmap[x+i+(y+1)*(map->width()*2)]==T_ROBOT) break;
			if (discreetmap[x+i+y*(map->width()*2)]==T_EROBOT ||
				discreetmap[x+i+(y+1)*(map->width()*2)]==T_EROBOT) {
				rsp|=1;
				break;
			} /* if */ 
		} else {
			if (discreetmap[x+i+y*(map->width()*2)]==T_EROBOT ||
				discreetmap[x+i+(y+1)*(map->width()*2)]==T_EROBOT) break;
			if (discreetmap[x+i+y*(map->width()*2)]==T_ROBOT ||
				discreetmap[x+i+(y+1)*(map->width()*2)]==T_ROBOT) {
				rsp|=1;
				break;
			} /* if */ 
		} /* if */ 
	} /* for */ 

	for (int i = 1;i < int((persistence*BULLET_SPEED)/0.5)+1 && (x-i>=0); i++) {
		if (discreetmap[x-i+y*(map->width()*2)]==T_BUILDING ||
			discreetmap[x-i+(y+1)*(map->width()*2)]==T_BUILDING) break;
		if (player==1) {
			if (discreetmap[x-i+y*(map->width()*2)]==T_ROBOT ||
				discreetmap[x-i+(y+1)*(map->width()*2)]==T_ROBOT) break;
			if (discreetmap[x-i+y*(map->width()*2)]==T_EROBOT ||
				discreetmap[x-i+(y+1)*(map->width()*2)]==T_EROBOT) {
				rsp|=4;
				break;
			} /* if */ 
		} else {
			if (discreetmap[x-i+y*(map->width()*2)]==T_EROBOT ||
				discreetmap[x-i+(y+1)*(map->width()*2)]==T_EROBOT) break;
			if (discreetmap[x-i+y*(map->width()*2)]==T_ROBOT ||
				discreetmap[x-i+(y+1)*(map->width()*2)]==T_ROBOT) {
				rsp|=4;
				break;
			} /* if */ 
		} /* if */ 
	} /* for */ 

	for (int i = 2; i < int((persistence*BULLET_SPEED)/0.5)+2 && (y+i<map->height()*2); i++) {
		if (discreetmap[x+(y+i)*(map->width()*2)]==T_BUILDING ||
			discreetmap[(x+1)+(y+i)*(map->width()*2)]==T_BUILDING) break;
		if (player==1) {
			if (discreetmap[x+(y+i)*(map->width()*2)]==T_ROBOT ||
				discreetmap[(x+1)+(y+i)*(map->width()*2)]==T_ROBOT) break;
			if (discreetmap[x+(y+i)*(map->width()*2)]==T_EROBOT ||
				discreetmap[(x+1)+(y+i)*(map->width()*2)]==T_EROBOT) {
				rsp|=2;
				break;
			} /* if */ 
		} else {
			if (discreetmap[x+(y+i)*(map->width()*2)]==T_EROBOT ||
				discreetmap[(x+1)+(y+i)*(map->width()*2)]==T_EROBOT) break;
			if (discreetmap[x+(y+i)*(map->width()*2)]==T_ROBOT ||
				discreetmap[(x+1)+(y+i)*(map->width()*2)]==T_ROBOT) {
				rsp|=2;
				break;
			} /* if */ 
		} /* if */ 
	} /* for */ 

	for (int i = 1; i < int((persistence*BULLET_SPEED)/0.5)+1 && (y-i>=0); i++) {
		if (discreetmap[x+(y-i)*(map->width()*2)]==T_BUILDING ||
			discreetmap[(x+1)+(y-i)*(map->width()*2)]==T_BUILDING) break;
		if (player==1) {
			if (discreetmap[x+(y-i)*(map->width()*2)]==T_ROBOT ||
				discreetmap[(x+1)+(y-i)*(map->width()*2)]==T_ROBOT) break;
			if (discreetmap[x+(y-i)*(map->width()*2)]==T_EROBOT ||
				discreetmap[(x+1)+(y-i)*(map->width()*2)]==T_EROBOT) {
				rsp|=8;
				break;
			} /* if */ 
		} else {
			if (discreetmap[x+(y-i)*(map->width()*2)]==T_EROBOT ||
				discreetmap[(x+1)+(y-i)*(map->width()*2)]==T_EROBOT) break;
			if (discreetmap[x+(y-i)*(map->width()*2)]==T_ROBOT ||
				discreetmap[(x+1)+(y-i)*(map->width()*2)]==T_ROBOT) {
				rsp|=8;
				break;
			} /* if */ 
		} /* if */ 
	} /* for */ 

	return rsp;
}