#ifdef _WIN32
#include "windows.h"
#else
#include <stddef.h>
#include <sys/types.h>
#include <dirent.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>

#include "3dobject.h"
#include "cmc.h"
#include "glprintf.h"
#include "myglutaux.h"
#include "piece3dobject.h"
#include "shadow3dobject.h"
#include "vector.h"
#include "mainmenu.h"


#ifndef __WINDOWS__
char *strupr(char *in)
{
  static char buffer[1024];
  char *c;

  for (c = buffer; *in != 0; c++, in++) {
    *c = toupper(*in);
  }
  *c = 0;

  return buffer;
 }
#endif


std::vector<std::pair<int, int>> SCREEN_SIZES {
  {320, 240},
  {400, 300},
  {640, 480},
  {800, 600},
  {1024, 768},
  {1280, 1024},
  {1366, 768},
  {1600, 900},
  {1920, 1080},
  {2560, 1440},
  {3440, 1440},
  {3840, 2160}
};


// Function name is really awful
template <typename Container, typename Value>
typename Container::const_iterator
find_next_pair_looped(const Container& container, Value value)
{
  typename Container::const_iterator found_value =
    std::find_if(container.cbegin(), container.cend(),
                 [value](const auto& pair) {
                   return pair.first == value;
                 });
  return found_value == container.cend() ? container.begin() : found_value;
}


extern int SCREEN_X;
extern int SCREEN_Y;
extern int COLOUR_DEPTH;
extern bool fullscreen;
extern int shadows;
extern bool sound;
extern int level;
extern int up_key,down_key,left_key,right_key,fire_key,pause_key;
extern bool show_radar;


MainMenu::MainMenu()
{
  loadConfiguration();
  title = new C3DObject("models/tittle.asc", "textures/");
  title->normalize(7.0);
}


void MainMenu::populateMaps()
{
#ifdef _WIN32
  // NOT TESTED BRANCH
  WIN32_FIND_DATA finfo;
  HANDLE h;

  h=FindFirstFile("maps/*.map",&finfo);
  if (h!=INVALID_HANDLE_VALUE) {
    if (strcmp(finfo.cFileName,".")!=0 &&
        strcmp(finfo.cFileName,"..")!=0) {
      char *name;
      name=new char[strlen(finfo.cFileName)+1];
      strcpy(name,finfo.cFileName);
      mapnames.push_back(name);
    }

    while(FindNextFile(h,&finfo)==TRUE) {

      if (strcmp(finfo.cFileName,".")!=0 &&
          strcmp(finfo.cFileName,"..")!=0) {
        char *name;
        name=new char[strlen(finfo.cFileName)+1];
        strcpy(name,finfo.cFileName);
        mapnames.push_back(name);
      }
    }
  }
#else
  std::transform(std::filesystem::directory_iterator("maps/"),
                 std::filesystem::directory_iterator(),
                 std::back_inserter(mapnames),
                 [](auto& entry) {
                   if (entry.path().extension() == ".map")
                     return entry.path().stem().string();
                   else
                     return std::string();
                 });
  mapnames.erase(std::remove(mapnames.begin(), mapnames.end(), ""), mapnames.end());
#endif
}


MainMenu::ACTION MainMenu::cycle(int, int)
{
  ACTION retval = ACTION::NONE;
  unsigned char* keyboard;

  SDL_PumpEvents();
  keyboard = SDL_GetKeyState(NULL);

  switch (status) {
  case 0:
    substatus++;
    if (substatus >= 40) {
      substatus = 0;
      status = 1;
    }
    break;
  case 1:
    substatus++;
    if ((keyboard[fire_key] && !old_keyboard[fire_key]) ||
        (keyboard[SDLK_1] && !old_keyboard[SDLK_1])) {
      status = 4;
      substatus = 0;
    }
    if (keyboard[SDLK_2] && !old_keyboard[SDLK_2]) {
      status = 6;
      substatus = 0;
    }
    if (keyboard[SDLK_3] && !old_keyboard[SDLK_3]) {
      status = 2;
      substatus = 0;
    }
    if (keyboard[SDLK_4] && !old_keyboard[SDLK_4]) {
      if (mapnames.size()) {
        ++mapnameIter;
        if (mapnameIter == mapnames.end()) mapnameIter = mapnames.begin();
      }
      saveConfiguration();
    }
    if (keyboard[SDLK_5] && !old_keyboard[SDLK_5]) {
      status = 5;
      substatus = 0;
    }
    break;
  case 2:
    substatus++;
    if (substatus >= 40) {
      substatus = 0;
      status = 3;
    }
    break;
  case 3:
    if (keyboard[SDLK_1] && !old_keyboard[SDLK_1]) {
      auto screen_size {find_next_pair_looped(SCREEN_SIZES, SCREEN_X)};
      screen_size++;
      if (screen_size == SCREEN_SIZES.cend()) {
        screen_size = SCREEN_SIZES.cbegin();
      }
      SCREEN_X = screen_size->first;
      SCREEN_Y = screen_size->second;
      retval = ACTION::RESTARTVIDEO;
      saveConfiguration();
    }
    if (keyboard[SDLK_2] && !old_keyboard[SDLK_2]) {
      switch (COLOUR_DEPTH) {
      case 8:
        COLOUR_DEPTH = 16;
        break;
      case 16:
        COLOUR_DEPTH = 24;
        break;
      case 24:
        COLOUR_DEPTH = 32;
        break;
      default:
        COLOUR_DEPTH = 8;
      }
      retval = ACTION::RESTARTVIDEO;
      saveConfiguration();
    }
    if (keyboard[SDLK_3] && !old_keyboard[SDLK_3]) {
      fullscreen = !fullscreen;
      retval = ACTION::RESTARTVIDEO;
      saveConfiguration();
    }
    if (keyboard[SDLK_4] && !old_keyboard[SDLK_4]) {
      shadows = ++shadows % 3;
      saveConfiguration();
    }
    if (keyboard[SDLK_5] && !old_keyboard[SDLK_5]) {
      sound = !sound;
      saveConfiguration();
    }
    if (keyboard[SDLK_6] && !old_keyboard[SDLK_6]) {
      level = (++level) % 4;
      saveConfiguration();
    }
    if (keyboard[SDLK_7] && !old_keyboard[SDLK_7]) {
      show_radar = !show_radar;
      saveConfiguration();
    }
    if (keyboard[SDLK_8] && !old_keyboard[SDLK_8]) {
      status = 0;
      substatus = 0;
    }
    break;
  case 4:
    substatus++;
    if (substatus >= 40) {
      retval = ACTION::START;
    }
    break;
  case 5:
    substatus++;
    if (substatus >= 40) {
      retval = ACTION::QUIT;
    }
    break;
  case 6:
    substatus++;
    if (substatus >= 40) {
      status = 7;
      substatus = 0;
    }
    break;
  case 7:
    {
      for (int i = 0; i < SDLK_LAST; i++) {
        if (keyboard[i] && !old_keyboard[i]) {
          switch (substatus) {
          case 0:
            up_key = i;
            break;
          case 1:
            down_key = i;
            break;
          case 2:
            left_key = i;
            break;
          case 3:
            right_key = i;
            break;
          case 4:
            fire_key = i;
            break;
          case 5:
            pause_key = i;
            break;
          }
          substatus++;
          if (substatus == 7) {
            status = 0;
            substatus = 0;
            saveConfiguration();
          }
        }
      }
    }
    break;
  }

  for (int i = 0; i < SDLK_LAST; i++) old_keyboard[i] = keyboard[i];
  return retval;
}


void MainMenu::draw(int width, int height)
{
  float lightPosition[4] {0.0f, 0.0f, 1000.0f, 0.0f};
  float lightSpecular[4] {1.0f, 1.0f, 1.0f, 1.0f};
  float lightDiffuse[4] {0.6f, 0.6f, 0.6f, 1.0f};
  float lightAmbient[4] {0.2f, 0.2f, 0.2f, 1.0f};

  /* Enable Lights, etc.: */
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
  glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glDisable(GL_SCISSOR_TEST);
  glEnable(GL_DEPTH_TEST);

  glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
  glClearColor(0, 0, 0, 0.0);
  glViewport(0, 0, width, height);
  float ratio = (float)width / float(height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, ratio, 1.0, 1024.0);
  gluLookAt(0, 0, 30, 0, 0, 0, 0, 1, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  switch(status) {
  case 0:
    glTranslatef(0, 3, 40 - substatus);
    title->draw(Color(1.0, 1.0, 1.0));
    break;
  case 1:
    glTranslatef(0, 3, 0);
    glPushMatrix();
    glRotatef(sin(substatus * 0.02) * 5.0f, 0, 1, 0);
    title->draw(Color(1.0, 1.0, 1.0));
    glPopMatrix();
    glColor3f(0.5, 0.5, 1.0);
    glTranslatef(-6, -6, 0);
    scaledglprintf2(0.005, 0.005, "1 - START NEW GAME   ");
    glTranslatef(0, -1, 0);
    scaledglprintf2(0.005, 0.005, "2 - REDEFINE KEYBOARD");
    glTranslatef(0, -1, 0);
    scaledglprintf2(0.005, 0.005, "3 - OPTIONS          ");
    glTranslatef(0, -1, 0);
    scaledglprintf2(0.005, 0.005, "4 - MAP: %s", (*mapnameIter).c_str());
    glTranslatef(0, -1, 0);
    scaledglprintf2(0.005, 0.005, "5 - EXIT GAME        ");
    break;
  case 2:
  case 4:
  case 5:
  case 6:
    glTranslatef(0, 3, substatus);
    title->draw(Color(1.0, 1.0, 1.0));
    break;
  case 3:
    glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, 3.5, 0);

    {
      auto screen_size {find_next_pair_looped(SCREEN_SIZES, SCREEN_X)};
      scaledglprintf(0.005, 0.005, "1 - RESOLUTION: %4dx%-4d", screen_size->first, screen_size->second);
    }

    glTranslatef(0, -1, 0);
    if (COLOUR_DEPTH == 8) scaledglprintf(0.005, 0.005, "2 - COLOR DEPTH:  8bit   ");
    if (COLOUR_DEPTH == 16) scaledglprintf(0.005, 0.005, "2 - COLOR DEPTH: 16bit   ");
    if (COLOUR_DEPTH == 24) scaledglprintf(0.005, 0.005, "2 - COLOR DEPTH: 24bit   ");
    if (COLOUR_DEPTH == 32) scaledglprintf(0.005, 0.005, "2 - COLOR DEPTH: 32bit   ");
    glTranslatef(0, -1, 0);
    if (fullscreen)
      scaledglprintf(0.005, 0.005, "3 - WINDOWED             ");
    else
      scaledglprintf(0.005, 0.005, "3 - FULLSCREEN           ");
    glTranslatef(0, -1, 0);
    if (shadows == 0) scaledglprintf(0.005, 0.005, "4 - SHADOWS: OFF         ");
    if (shadows == 1) scaledglprintf(0.005, 0.005, "4 - SHADOWS: ON - DIAG   ");
    if (shadows == 2) scaledglprintf(0.005, 0.005, "4 - SHADOWS: ON - VERT   ");
    glTranslatef(0, -1, 0);
    if (sound)
      scaledglprintf(0.005, 0.005, "5 - SOUND: ON            ");
    else
      scaledglprintf(0.005, 0.005, "5 - SOUND: OFF           ");
    glTranslatef(0, -1, 0);
    if (level == 0) scaledglprintf(0.005, 0.005, "6 - LEVEL: EASY          ");
    if (level == 1) scaledglprintf(0.005, 0.005, "6 - LEVEL: NORMAL        ");
    if (level == 2) scaledglprintf(0.005, 0.005, "6 - LEVEL: HARD          ");
    if (level == 3) scaledglprintf(0.005, 0.005, "6 - LEVEL: IMPOSSIBLE    ");
    glTranslatef(0, -1, 0);
    if (show_radar)
      scaledglprintf(0.005, 0.005, "7 - RADAR: OFF           ");
    else
      scaledglprintf(0.005, 0.005, "7 - RADAR: ON            ");
    glTranslatef(0, -1, 0);
    scaledglprintf(0.005, 0.005, "8 - BACK                 ");
    break;
  case 7:
    {
      char tmp[256];

      glColor3f(0.5, 0.5, 1.0);
      glTranslatef(0, 5, 0);
      scaledglprintf(0.005, 0.005, "REDEFINE KEYBOARD");
      glTranslatef(0, -2, 0);
      if (substatus != 0)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp, "PRESS A KEY FOR UP: %s", strupr(SDL_GetKeyName((SDLKey) up_key)));
      scaledglprintf(0.005, 0.005, tmp);
      glTranslatef(0, -1, 0);
      if (substatus != 1)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp, "PRESS A KEY FOR DOWN: %s", strupr(SDL_GetKeyName((SDLKey) down_key)));
      scaledglprintf(0.005, 0.005, tmp);
      glTranslatef(0, -1, 0);
      if (substatus != 2)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp,"PRESS A KEY FOR LEFT: %s", strupr(SDL_GetKeyName((SDLKey) left_key)));
      scaledglprintf(0.005, 0.005, tmp);
      glTranslatef(0, -1, 0);
      if (substatus != 3)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp, "PRESS A KEY FOR RIGHT: %s", strupr(SDL_GetKeyName((SDLKey) right_key)));
      scaledglprintf(0.005, 0.005, tmp);
      glTranslatef(0, -1, 0);
      if (substatus != 4)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp, "PRESS A KEY FOR FIRE: %s", strupr(SDL_GetKeyName((SDLKey) fire_key)));
      scaledglprintf(0.005, 0.005, tmp);

      glTranslatef(0, -1, 0);
      if (substatus != 5)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp, "PRESS A KEY FOR PAUSE/MENU: %s", strupr(SDL_GetKeyName((SDLKey) pause_key)));
      scaledglprintf(0.005, 0.005, tmp);

      glColor3f(0.5, 0.5, 1.0);
      glTranslatef(0, -2, 0);
      scaledglprintf(0.005, 0.005, "PG.UP/PG.DOWN CHANGE THE ZOOM");

      if (substatus > 5) {
        glColor3f(1, 1, 1);
        glTranslatef(0, -2, 0);
        scaledglprintf(0.005, 0.005, "PRESS ANY KEY TO RETURN TO MAIN MENU");
      }
    }
    break;
  }
  SDL_GL_SwapBuffers();
}


void MainMenu::refreshDisplayLists()
{
  if (title)
    title->refresh_display_lists();
}


std::string MainMenu::getMapPath()
{
  return "maps/" + *mapnameIter + ".map";
}

void MainMenu::loadConfiguration(void)
{
    std::string mapname;
    std::ifstream configFile("nether.cfg");
    configFile >> SCREEN_X >> SCREEN_Y >> fullscreen >> shadows
               >> up_key >> down_key >> left_key >> right_key >> fire_key >> pause_key
               >> sound >> level >> mapname;

    populateMaps();
    mapnameIter = std::find(mapnames.begin(), mapnames.end(), mapname);
    if (mapnameIter == mapnames.end()) {
      mapnameIter = mapnames.begin();
    }
}


void MainMenu::saveConfiguration(void)
{
  std::ofstream configFile("nether.cfg");
  configFile << SCREEN_X << ' ' << SCREEN_Y << std::endl
             << fullscreen << ' ' << shadows  << std::endl
             << up_key << ' ' << down_key << ' ' << left_key << ' ' << right_key << ' '
             << fire_key << ' ' << pause_key << std::endl
             << sound << std::endl
             << level << std::endl
             << *mapnameIter << std::endl;
}
