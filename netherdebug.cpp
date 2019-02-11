#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>
#include <fstream>
#include <string>

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


bool NETHER::saveDebugReport(const std::string& filename)
{
  std::ofstream log(filename);
  log << "NETHER EARTH NG Debug Report\n\n";
  log << "MAPW: " << map_w << "\nMAPH: " << map_h << '\n';

  log << "MAP:\n";
  for(int i = 0; i < map_h; i++) {
    for(int j = 0; j < map_w; j++) {
      log << map[j+i*map_w] << ' ';
    }
    log << '\n';
  }

  log << "LIGHTPOS: " << lightpos[0] << ' '
      << lightpos[1] << ' '
      << lightpos[2] << ' '
      << lightpos[3] << '\n';
  log << "LIGHTPOSV: " << lightposv;
  log << "CAMERA: " << camera;
  log << "VIEWP: " << viewp;
  log << "SHIPP: " << shipp;
  if (shiplanded)
    log << "SHIP LANDED\n";
  else
    log << "SHIP NOT LANDED\n";

  log << "# OF BUILDINGS: " << buildings.Length() << '\n';
  buildings.Rewind();
  Building *b;
  while(buildings.Iterate(b)) {
    log << "BUILDING:\n TYPE: " << b->type
        << "\n OWNER: " << b->owner
        << "\n STATUS: " << b->status << "\n\n";
    log << b->pos;
  }

  for(int i = 0; i < 2; i++) {
    log << "\n# OF ROBOTS PLAYER " << i << ": " << robots[i].Length() << '\n';
    robots[i].Rewind();
    Robot *r;

    const char* tractions[3] = {"BIPOD", "TRACKS", "ANTIGRAV"};
    const char* pieces[5] = {"CANNONS", "MISSILES", "PHASERS", "NUCLEAR", "ELECTRONICS"};
    while(robots[i].Iterate(r)) {
      log << "ROBOT:\n";
      log << ' ' << tractions[r->traction] << '\n';
      for (int j = 0; j < 5; j++) {
        if (r->pieces[j])
          log << ' ' << pieces[j] << '\n';
      }
      log << " PROGRAM: " << r->program << "\n PROGRAM PARAMETER: " << r->program_parameter.as_int << '\n';
      log << " PROGRAM GOAL: ";
      log << r->program_goal;
      log << " ACTUAL OPERATOR: " << r->op << '\n';
      if (r->shipover)
        log << " HAS THE SHIP OVER IT\n";
      else
        log << " HAS NO SHIP OVER IT\n";
      log << " FIRETIMER: " << r->firetimer << "\n STRENGTH: " << r->strength << '\n';
      log << " POSITION: ";
      log << r->pos;
      log << " ANGLE: " << r->angle << '\n';
      log << " MINIMUM CONTAINER BOX: \n";
      log << r->cmc;
      log << " ELECTRONICS STATE: " << r->electronics_state
          <<"\n CHASSIS STATE: " << r->chassis_state;
      log << "\n\n";
    }
  }

  log << "\n# BULLETS: " << bullets.size() << '\n';
  std::for_each(bullets.begin(), bullets.end(),
                [&log, this](const auto& bullet) {
                  log << " BULLET:\n TYPE: " << bullet.type
                      << "\n STEP: " << bullet.step
                      << "\n ANGLE: " << bullet.angle << '\n';
                  log << " POSITION: ";
                  log << bullet.pos;
                  int pos = robots[0].SearchObjRef(bullet.owner);
                  if (pos == -1) {
                    pos = robots[1].SearchObjRef(bullet.owner);
                    log << " OWNER: PLAYER 1 ROBOT " << pos << '\n';
                  } else {
                    log << " OWNER: PLAYER 0 ROBOT " << pos << '\n';
                  }

                  log << " MINIMUM CONTAINER BOX: \n";
                  log << bullet.cmc << '\n';
                });

  log << "# EXPLOSIONS " << explosions.Length() << '\n';
  explosions.Rewind();
  Explosion *e;
  while(explosions.Iterate(e)) {
    log << "EXPLOSION:\n POSITION:\n";
    log << e->pos;
    log << " STEP: " << e->step << "\n SIZE: " << e->size << "\n\n";
  }

  log << "\nTIME: DAY " << day << ' ' << hour << ':' << minute << ':' << second << '\n';
  log << "\nRESOURCES:\n";

  for(int i = 0; i < 2; i++) {
    log << "PLAYER " << i << ':';
    for(int j = 0; j < 7; j++) {
      log << resources[i][j] << ' ';
    }
    log << '\n';
  }

  log << "\nROBOT UNDER CONTROL: " << robots[0].SearchObjRef(controlled) << "\n";
  log << "\nMENU " << menu.act_menu << "\nACT BUTTON: " << menu.act_button << "\n";

  return true;
}
