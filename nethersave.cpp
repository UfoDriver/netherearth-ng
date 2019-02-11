#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>

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


extern int frames_per_sec;
extern bool fullscreen;
extern int shadows;
extern bool sound;
extern int up_key,down_key,left_key,right_key,fire_key,pause_key;
extern int level;


bool NETHER::save_game(char *filename)
{
	int i,j;
	FILE *fp;
	Building *b;
	Robot *r;
	Bullet *bul;
	Explosion *e;

	fp=fopen(filename,"w");
	if (fp==0) return false;

/*
	int map_w,map_h;
	int *map;
	float lightpos[4];
	Vector lightposv;
*/

	fprintf(fp,"%i %i\n",map_w,map_h);

	for(i=0;i<map_h;i++) {
		for(j=0;j<map_w;j++) {
			fprintf(fp,"%i ",map[j+i*map_w]);
		} /* for */ 
		fprintf(fp,"\n");
	} /* for */ 

	fprintf(fp,"%.8f %.8f %.8f %.8f\n",lightpos[0],lightpos[1],lightpos[2],lightpos[3]);
	lightposv.save(fp);

/*
	Vector camera,viewp;
	Vector shipp;
	bool shiplanded;
	int ship_op,ship_op2,ship_op3;
*/

	camera.save(fp);
	viewp.save(fp);
	shipp.save(fp);
	if (shiplanded) fprintf(fp,"1\n");
			   else fprintf(fp,"0\n");

/*
	List<BUILDING> buildings;
	List<ROBOT> robots[2];
	List<BULLET> bullets;
	List<EXPLOSION> explosions;
*/ 

	fprintf(fp,"%i\n",buildings.Length());
	buildings.Rewind();
	while(buildings.Iterate(b)) {
/*
		int type;
		Vector pos;
		int owner;
		int status;	
*/ 
		fprintf(fp,"%i %i %i\n",b->type,b->owner,b->status);
		b->pos.save(fp);
	} /* while */ 

	for(i=0;i<2;i++) {
		fprintf(fp,"%i\n",robots[i].Length());
		robots[i].Rewind();
		while(robots[i].Iterate(r)) {
/*
			int traction;
			bool pieces[5];
			int program;
			int program_parameter;
			Vector program_goal;
			int op;
			bool shipover;
			int firetimer;
			int strength;
			Vector pos;
			int angle;
			CMC cmc;
*/
			fprintf(fp,"%i\n",r->traction);
			for(j=0;j<5;j++) {
				if (r->pieces[j]) fprintf(fp,"1\n");
							 else fprintf(fp,"0\n");
			} /* for */ 
			fprintf(fp,"%i %i\n",r->program,r->program_parameter);
			r->program_goal.save(fp);
			fprintf(fp,"%i\n",r->op);
			if (r->shipover) fprintf(fp,"1\n");
						else fprintf(fp,"0\n");
			fprintf(fp,"%i %i\n",r->firetimer,r->strength);
			r->pos.save(fp);
			fprintf(fp,"%i\n",r->angle);
			r->cmc.save(fp);

			fprintf(fp,"%i %i\n",r->electronics_state,r->chassis_state);
		} /* while */ 
	} /* for */ 

	fprintf(fp,"%i\n", bullets.size());

    std::for_each(bullets.begin(), bullets.end(),
                  [fp, this](auto& bullet) {
                    fprintf(fp,"%i %i %i\n", bullet.type, bullet.step, bullet.angle);
                    bullet.pos.save(fp);
                    int i = robots[0].SearchObjRef(bullet.owner);
                    if (i==-1) {
                      i=robots[1].SearchObjRef(bullet.owner);
                      fprintf(fp,"1 %i\n",i);
                    } else {
                      fprintf(fp,"0 %i\n",i);
                    }
                    bullet.cmc.save(fp);
                  });

    fprintf(fp,"%i\n", explosions.size());
    std::for_each(explosions.begin(), explosions.end(),
                  [fp](auto& e) {
                    e.pos.save(fp);
                    fprintf(fp,"%i %i\n",e.step, e.size);
                  });
/*
	int day,hour,minute,second;
	int resources[2][7];
	ROBOT *controlled;
*/ 

	fprintf(fp,"%i %i %i %i\n",day,hour,minute,second);
	for(i=0;i<2;i++) {
		for(j=0;j<7;j++) {
			fprintf(fp,"%i ",resources[i][j]);
		} /* for */ 
		fprintf(fp,"\n");
	} /* for */ 
	fprintf(fp,"%i\n",robots[0].SearchObjRef(controlled));

/*
	int act_menu;
	int act_button;
*/ 
	fprintf(fp,"%i %i\n",menu.act_menu,menu.act_button);

	fclose(fp);
	return true;
} /* NETHER::save_game */ 


bool NETHER::loadGame(const std::string& filename)
{
	int i,j,k,booltmp,length;
	FILE *fp;
	Building *b;
	Robot *r;
	Bullet *bul;
	Explosion *e;

	fp=fopen(filename.c_str(),"r");
	if (fp==0) return false;

/*
	int map_w,map_h;
	int *map;
	float lightpos[4];
	Vector lightposv;
*/

	AI_deleteprecomputations();
	if (2!=fscanf(fp,"%i %i",&map_w,&map_h)) return false;

	explosions.clear();
	buildings.Delete();
	for(i=0;i<2;i++) robots[i].Delete();
	bullets.clear();
	delete map;
	map=new int[map_w*map_h];
	for(i=0;i<map_h;i++) {
		for(j=0;j<map_w;j++) {
			if (1!=fscanf(fp,"%i",&(map[j+i*map_w]))) return false;
		} /* for */ 
	} /* for */ 
	if (4!=fscanf(fp,"%f %f %f %f",&(lightpos[0]),&(lightpos[1]),&(lightpos[2]),&(lightpos[3]))) return false;
	lightposv.load(fp);

/*
	Vector camera,viewp;
	Vector shipp;
	bool shiplanded;
	int ship_op,ship_op2,ship_op3;
*/

	if (!camera.load(fp) ||
		!viewp.load(fp) ||
		!shipp.load(fp)) return false;
	if (1!=fscanf(fp,"%i",&booltmp)) return false;
	if (booltmp==1) shiplanded=true;
			   else shiplanded=false;

/*
	List<BUILDING> buildings;
	List<ROBOT> robots[2];
	List<BULLET> bullets;
	List<EXPLOSION> explosions;
*/ 

	if (1!=fscanf(fp,"%i",&length)) return false;	
	for(k=0;k<length;k++) {
/*
		int type;
		Vector pos;
		int owner;
		int status;	
*/
      Building::BUILDINGS_AND_WALLS type;
      int owner;
      int status;
      if (3!=fscanf(fp,"%i %i %i", &type, &owner, &status)) return false;
      b=new Building(Vector(0, 0, 0), type, owner, status);
      if (!b->pos.load(fp)) return false;
      buildings.Add(b);
	} /* for */ 

	for(i=0;i<2;i++) {

		if (1!=fscanf(fp,"%i",&length)) return false;
		for(k=0;k<length;k++) {
/*
			int traction;
			bool pieces[5];
			int program;
			int program_parameter;
			Vector program_goal;
			int op;
			bool shipover;
			int firetimer;
			int strength;
			Vector pos;
			int angle;
			CMC cmc;
*/
			r=new Robot();
			if (1!=fscanf(fp,"%i",&r->traction)) return false;
			for(j=0;j<5;j++) {
				if (1!=fscanf(fp,"%i",&booltmp)) return false;
				if (booltmp==1) r->pieces[j]=true;
						   else r->pieces[j]=false;
			} /* for */ 
			if (2!=fscanf(fp,"%i %i",&r->program,&r->program_parameter)) return false;
			r->program_goal.load(fp);
			if (1!=fscanf(fp,"%i\n",&r->op)) return false;
			if (1!=fscanf(fp,"%i",&booltmp)) return false;
			if (booltmp==1) r->shipover=true;
					   else r->shipover=false;
			if (2!=fscanf(fp,"%i %i",&r->firetimer,&r->strength)) return false;
			r->pos.load(fp);
			if (1!=fscanf(fp,"%i",&r->angle)) return false;
			r->cmc.load(fp);

			if (2!=fscanf(fp,"%i %i",&r->electronics_state,&r->chassis_state)) return false;

			robots[i].Add(r);
		} /* for */ 
	} /* for */ 

	if (1!=fscanf(fp,"%i",&length)) return false;
    
	for (int k = 0; k < length; k++) {
      Bullet bullet;
      if (3!=fscanf(fp,"%i %i %i", &bullet.type, &bullet.step, &bullet.angle)) return false;
      bullet.pos.load(fp);
      if (2!=fscanf(fp,"%i %i",&j,&i)) return false;
      if (i>=0) bullet.owner=robots[j][i];
      else bullet.owner=0;
      bullet.cmc.load(fp);
      bullets.push_back(bullet);
	}

	if (1!=fscanf(fp,"%i",&length)) return false;	
    for(int k = 0; k < length; k++) {
      Explosion e;
      e.pos.load(fp);
      if (2!=fscanf(fp,"%i %i",&e.step,&e.size)) return false;
      explosions.push_back(e);
    }

/*
	int day,hour,minute,second;
	int resources[2][7];
	ROBOT *controlled;
*/ 

	if (4!=fscanf(fp,"%i %i %i %i",&day,&hour,&minute,&second)) return false;
	for(i=0;i<2;i++) {
		for(j=0;j<7;j++) {
			if (1!=fscanf(fp,"%i",&(resources[i][j]))) return false;
		} /* for */ 
	} /* for */ 
	if (1!=fscanf(fp,"%i",&i)) return false;
	if (i>=0) controlled=robots[0][i];
		 else controlled=0;

/*
	int act_menu;
	int act_button;
*/ 
	if (2!=fscanf(fp,"%i %i",&menu.act_menu,&menu.act_button)) return false;

	fclose(fp);
	AI_precomputations();
	return true;
} /* NETHER::load_game */ 
