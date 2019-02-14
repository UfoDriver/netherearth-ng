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
    for (Robot* r: robots[i]) {
      fprintf(fp,"%i\n",r->traction);
      for (int j = 0; j < 5; j++) {
        if (r->pieces[j])
          fprintf(fp,"1\n");
        else
          fprintf(fp,"0\n");
      }
      fprintf(fp,"%i %i\n",r->program,r->program_parameter);
      r->program_goal.save(fp);
      fprintf(fp,"%i\n",r->op);
      if (r->shipover)
        fprintf(fp,"1\n");
      else
        fprintf(fp,"0\n");
      fprintf(fp,"%i %i\n",r->firetimer,r->strength);
      r->pos.save(fp);
      fprintf(fp,"%i\n",r->angle);
      r->cmc.save(fp);

      fprintf(fp,"%i %i\n",r->electronics_state,r->chassis_state);
    }
  }

  fprintf(fp,"%i\n", bullets.size());

  for (Bullet& bullet: bullets) {
    fprintf(fp,"%i %i %i\n", bullet.type, bullet.step, bullet.angle);
    bullet.pos.save(fp);
    int i = find_index(robots[0], bullet.owner);
    if (i==-1) {
      i = find_index(robots[1], bullet.owner);
      fprintf(fp,"1 %i\n", bullet.owner->getId());
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
  fprintf(fp,"%i\n", find_index(robots[0], controlled));

  fprintf(fp,"%i %i\n",menu.act_menu,menu.act_button);

  fclose(fp);
  return true;
}


bool NETHER::loadGame(const std::string& filename)
{
  std::ifstream inFile(filename);

  AI_deleteprecomputations();

  inFile >> map_w >> map_h;

  explosions.clear();
  buildings.clear();
  for (int i = 0; i < 2; i++) {
    for (Robot* r: robots[i]) delete r;
    robots[i].clear();
  }
  bullets.clear();
  map.clear();
  map.reserve(map_w * map_h);
  for (int i = 0; i < map_h; i++) {
    for (int j = 0; j < map_w; j++) {
      inFile >> map[j + i * map_w];
    }
  }

  inFile >> lightpos[0] >> lightpos[1] >> lightpos[2] >> lightpos[3];
  inFile >> lightposv
         >> camera
         >> viewp
         >> shipp;

  inFile >> shiplanded;

  int length;
  inFile >> length;
  for (int k = 0; k < length; k++) {
    buildings.push_back(Building(inFile));
  }

  for (int i = 0; i < 2; i++) {
    inFile >> length;
    for (int k = 0; k < length; k++) {
      robots[i].push_back(new Robot(inFile));
    }
  }

  inFile >> length;

  for (int k = 0; k < length; k++) {
    bullets.emplace_back(inFile, robots);
  }

  inFile >> length;
  for (int k = 0; k < length; k++) {
    explosions.emplace_back(inFile);
  }

  inFile >> day >> hour >> minute >> second;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      inFile >> resources[i][j];
    }
  }

  int i;
  inFile >> i;
  if (i >= 0)
    controlled = robots[0][i];
  else
    controlled = 0;

  inFile >> menu.act_menu >> menu.act_button;

  AI_precomputations();
  return true;
}
