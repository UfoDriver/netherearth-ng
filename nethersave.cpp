#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>
#include <iomanip>

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


bool NETHER::saveGame(const std::string& filename)
{
  std::ofstream oFile(filename);

  oFile << map_w << ' ' << map_h << '\n';

  for (int i = 0; i < map_h; i++) {
    for (int j = 0; j < map_w; j++) {
      oFile << map[j + i * map_w] << ' ';
    }
    oFile << '\n';
  }

  oFile << std::setw(8) << lightpos[0] << ' '
        << std::setw(8) << lightpos[1] << ' '
        << std::setw(8) << lightpos[2] << ' '
        << std::setw(8) << lightpos[3] << '\n';
  oFile << lightposv
        << camera
        << viewp
        << shipp;

  oFile << shiplanded << '\n';

  oFile << buildings.size() << '\n';
  for (Building& b: buildings) {
    oFile << b;
  }

  for (int i = 0; i < 2; i++) {
    oFile << robots[i].size() << '\n';
    for (Robot* r: robots[i]) {
      oFile << *r;
    }
  }

  oFile << bullets.size() << '\n';
  for (Bullet& bullet: bullets) {
    oFile << std::make_pair(bullet, robots);
  }

  oFile << explosions.size() << '\n';
  for (Explosion& e: explosions) {
    oFile << e;
  }

  oFile << day << ' ' << hour << ' ' << minute << ' ' << second << '\n';
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      oFile << resources[i][j] << ' ';
    }
    oFile << '\n';
   }

  oFile << find_index(robots[0], controlled) << '\n';
  oFile << menu.act_menu << ' ' << menu.act_button << std::endl;

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
      int tile;
      inFile >> tile;
      map.push_back(tile);
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
