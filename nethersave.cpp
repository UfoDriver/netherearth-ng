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
#include "utils.h"

#include "glprintf.h"


extern int frames_per_sec;
extern bool fullscreen;
extern int shadows;
extern bool sound;
extern int up_key,down_key,left_key,right_key,fire_key,pause_key;
extern int level;


bool NETHER::save_game(char *filename)
{
  FILE *fp;

  fp=fopen(filename,"w");
  if (fp==0) return false;

  fprintf(fp,"%i %i\n",map_w,map_h);

  for (int i = 0; i < map_h; i++) {
    for (int j = 0; j < map_w; j++) {
      fprintf(fp,"%i ",map[j+i*map_w]);
    }
    fprintf(fp,"\n");
  }

  fprintf(fp,"%.8f %.8f %.8f %.8f\n",lightpos[0],lightpos[1],lightpos[2],lightpos[3]);
  lightposv.save(fp);

  camera.save(fp);
  viewp.save(fp);
  shipp.save(fp);
  if (shiplanded)
    fprintf(fp,"1\n");
  else
    fprintf(fp,"0\n");

  fprintf(fp,"%i\n", buildings.size());
  for (Building& b: buildings) {
    fprintf(fp,"%i %i %i\n",b.type,b.owner,b.status);
    b.pos.save(fp);
  }

  for (int i = 0; i < 2; i++) {
    fprintf(fp,"%i\n",robots[i].size());
    for (Robot& r: robots[i]) {
      fprintf(fp,"%i\n",r.traction);
      for (int j = 0; j < 5; j++) {
        if (r.pieces[j])
          fprintf(fp,"1\n");
        else
          fprintf(fp,"0\n");
      }
      fprintf(fp,"%i %i\n",r.program,r.program_parameter);
      r.program_goal.save(fp);
      fprintf(fp,"%i\n",r.op);
      if (r.shipover)
        fprintf(fp,"1\n");
      else
        fprintf(fp,"0\n");
      fprintf(fp,"%i %i\n",r.firetimer,r.strength);
      r.pos.save(fp);
      fprintf(fp,"%i\n",r.angle);
      r.cmc.save(fp);

      fprintf(fp,"%i %i\n",r.electronics_state,r.chassis_state);
    }
  }

  fprintf(fp,"%i\n", bullets.size());

  for (Bullet& bullet: bullets) {
    fprintf(fp,"%i %i %i\n", bullet.type, bullet.step, bullet.angle);
    bullet.pos.save(fp);
    int i = find_index(robots[0], *bullet.owner);
    if (i==-1) {
      i = find_index(robots[1], *bullet.owner);
      fprintf(fp,"1 %i\n",i);
    } else {
      fprintf(fp,"0 %i\n",i);
    }
    bullet.cmc.save(fp);
  }

  fprintf(fp,"%i\n", explosions.size());
  for (Explosion& e: explosions) {
    e.pos.save(fp);
    fprintf(fp,"%i %i\n",e.step, e.size);
  }

  fprintf(fp,"%i %i %i %i\n",day,hour,minute,second);
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      fprintf(fp,"%i ",resources[i][j]);
    }
    fprintf(fp,"\n");
  }
  fprintf(fp,"%i\n", find_index(robots[0], *controlled));

  fprintf(fp,"%i %i\n",menu.act_menu,menu.act_button);

  fclose(fp);
  return true;
}


bool NETHER::loadGame(const std::string& filename)
{
  int booltmp,length;
  FILE *fp;

  fp=fopen(filename.c_str(),"r");
  if (fp==0) return false;

  AI_deleteprecomputations();
  if (2!=fscanf(fp,"%i %i",&map_w,&map_h)) return false;

  explosions.clear();
  buildings.clear();
  for(int i = 0; i < 2; i++) robots[i].clear();
  bullets.clear();
  map.clear();
  map.reserve(map_w * map_h);
  for(int i = 0; i < map_h; i++) {
    for(int j = 0; j < map_w; j++) {
      if (1!=fscanf(fp,"%i",&(map[j+i*map_w]))) return false;
    }
  }
  if (4!=fscanf(fp,"%f %f %f %f",&(lightpos[0]),&(lightpos[1]),&(lightpos[2]),&(lightpos[3]))) return false;
  lightposv.load(fp);

  if (!camera.load(fp) ||
      !viewp.load(fp) ||
      !shipp.load(fp)) return false;
  if (1!=fscanf(fp,"%i",&booltmp)) return false;
  if (booltmp==1) shiplanded=true;
  else shiplanded=false;

  if (1!=fscanf(fp,"%i",&length)) return false;
  for (int k = 0; k < length; k++) {
    Building::BUILDINGS_AND_WALLS type;
    int owner;
    int status;
    if (3!=fscanf(fp,"%i %i %i", &type, &owner, &status)) return false;
    Building b(Vector(0, 0, 0), type, owner, status);
    if (!b.pos.load(fp)) return false;
    buildings.push_back(b);
  }

  for (int i = 0; i < 2; i++) {
    if (1!=fscanf(fp,"%i",&length)) return false;
    for (int k = 0; k < length; k++) {
      Robot r;
      if (1!=fscanf(fp,"%i", &r.traction)) return false;
      for (int j = 0; j < 5; j++) {
        if (1!=fscanf(fp,"%i",&booltmp)) return false;
        if (booltmp==1)
          r.pieces[j]=true;
        else
          r.pieces[j]=false;
      }
      if (2!=fscanf(fp,"%i %i",&r.program,&r.program_parameter)) return false;
      r.program_goal.load(fp);
      if (1!=fscanf(fp,"%i\n",&r.op)) return false;
      if (1!=fscanf(fp,"%i",&booltmp)) return false;
      if (booltmp==1)
        r.shipover=true;
      else
        r.shipover=false;
      if (2!=fscanf(fp,"%i %i",&r.firetimer,&r.strength)) return false;
      r.pos.load(fp);
      if (1!=fscanf(fp,"%i",&r.angle)) return false;
      r.cmc.load(fp);

      if (2!=fscanf(fp,"%i %i",&r.electronics_state,&r.chassis_state)) return false;

      robots[i].push_back(r);
    }
  }

  if (1!=fscanf(fp,"%i",&length)) return false;

  for (int k = 0; k < length; k++) {
    Bullet bullet;
    int i, j;
    if (3!=fscanf(fp,"%i %i %i", &bullet.type, &bullet.step, &bullet.angle)) return false;
    bullet.pos.load(fp);
    if (2!=fscanf(fp,"%i %i", &j, &i )) return false;
    if (i >= 0) bullet.owner=&robots[j][i];
      else bullet.owner=0;
      bullet.cmc.load(fp);
      bullets.push_back(bullet);
  }

  if (1!=fscanf(fp,"%i",&length)) return false;
  for (int k = 0; k < length; k++) {
    Explosion e;
    e.pos.load(fp);
    if (2!=fscanf(fp,"%i %i",&e.step,&e.size)) return false;
    explosions.push_back(e);
  }

  if (4!=fscanf(fp,"%i %i %i %i",&day,&hour,&minute,&second)) return false;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      if (1!=fscanf(fp,"%i",&(resources[i][j]))) return false;
    }
  }
  int i;
  if (1!=fscanf(fp,"%i",&i)) return false;
  if ( i>=0 )
    controlled=&robots[0][i];
  else
    controlled=0;

  if (2!=fscanf(fp,"%i %i",&menu.act_menu,&menu.act_button)) return false;

  fclose(fp);
  AI_precomputations();
  return true;
}
