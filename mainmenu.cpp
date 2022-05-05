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
#include "mainmenu.h"
#include "myglutaux.h"
#include "piece3dobject.h"
#include "shadow3dobject.h"
#include "utils.h"
#include "vector.h"


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


MainMenu::MainMenu(Config& config)
  : config{config}
{
  config.load();
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
  std::transform(
    std::filesystem::directory_iterator("maps/"),
    std::filesystem::directory_iterator(),
    std::back_inserter(mapnames),
    [](auto& entry) {
      if (entry.path().extension() == ".map")
        return entry.path().stem().string();
      else
        return std::string();
    }
  );
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
    if ((keyboard[config.keyFire] && !old_keyboard[config.keyFire]) ||
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
      config.save();
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
      auto screen_size {find_next_pair_looped(SCREEN_SIZES, config.screenX)};
      screen_size++;
      if (screen_size == SCREEN_SIZES.cend()) {
        screen_size = SCREEN_SIZES.cbegin();
      }
      config.screenX = screen_size->first;
      config.screenY = screen_size->second;
      config.save();
      std::cerr << "Trying to restart video to " << config.screenX << ':' << config.screenY << std::endl;
      retval = ACTION::RESTART_VIDEO;
    }
    if (keyboard[SDLK_2] && !old_keyboard[SDLK_2]) {
      switch (config.colorDepth) {
      case 8:
        config.colorDepth = 16;
        break;
      case 16:
        config.colorDepth = 24;
        break;
      case 24:
        config.colorDepth = 32;
        break;
      default:
        config.colorDepth = 8;
      }
      retval = ACTION::RESTART_VIDEO;
      config.save();
    }
    if (keyboard[SDLK_3] && !old_keyboard[SDLK_3]) {
      config.fullscreenMode = !config.fullscreenMode;
      retval = ACTION::RESTART_VIDEO;
      config.save();
    }
    if (keyboard[SDLK_4] && !old_keyboard[SDLK_4]) {
      config.shadows = ++config.shadows % 3;
      config.save();
    }
    if (keyboard[SDLK_5] && !old_keyboard[SDLK_5]) {
      config.sound = !config.sound;
      config.save();
    }
    if (keyboard[SDLK_6] && !old_keyboard[SDLK_6]) {
      config.level = (++config.level) % 4;
      config.save();
    }
    if (keyboard[SDLK_7] && !old_keyboard[SDLK_7]) {
      config.showRadar = !config.showRadar;
      config.save();
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
            config.keyUp = i;
            break;
          case 1:
            config.keyDown = i;
            break;
          case 2:
            config.keyLeft = i;
            break;
          case 3:
            config.keyRight = i;
            break;
          case 4:
            config.keyFire = i;
            break;
          case 5:
            config.keyPause = i;
            break;
          }
          substatus++;
          if (substatus == 7) {
            status = 0;
            substatus = 0;
            config.save();
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
      auto screen_size {find_next_pair_looped(SCREEN_SIZES, config.screenX)};
      scaledglprintf(0.005, 0.005, "1 - RESOLUTION: %4dx%-4d", screen_size->first, screen_size->second);
    }

    glTranslatef(0, -1, 0);
    scaledglprintf(0.005, 0.005, "2 - COLOR DEPTH: %2dbit   ", config.colorDepth);
    glTranslatef(0, -1, 0);
    if (config.fullscreenMode)
      scaledglprintf(0.005, 0.005, "3 - WINDOWED             ");
    else
      scaledglprintf(0.005, 0.005, "3 - FULLSCREEN           ");
    glTranslatef(0, -1, 0);
    if (config.shadows == 0) scaledglprintf(0.005, 0.005, "4 - SHADOWS: OFF         ");
    if (config.shadows == 1) scaledglprintf(0.005, 0.005, "4 - SHADOWS: ON - DIAG   ");
    if (config.shadows == 2) scaledglprintf(0.005, 0.005, "4 - SHADOWS: ON - VERT   ");
    glTranslatef(0, -1, 0);
    if (config.sound)
      scaledglprintf(0.005, 0.005, "5 - SOUND: ON            ");
    else
      scaledglprintf(0.005, 0.005, "5 - SOUND: OFF           ");
    glTranslatef(0, -1, 0);
    if (config.level == 0) scaledglprintf(0.005, 0.005, "6 - LEVEL: EASY          ");
    if (config.level == 1) scaledglprintf(0.005, 0.005, "6 - LEVEL: NORMAL        ");
    if (config.level == 2) scaledglprintf(0.005, 0.005, "6 - LEVEL: HARD          ");
    if (config.level == 3) scaledglprintf(0.005, 0.005, "6 - LEVEL: IMPOSSIBLE    ");
    glTranslatef(0, -1, 0);
    if (config.showRadar)
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
      sprintf(tmp, "PRESS A KEY FOR UP: %s", strupr(SDL_GetKeyName((SDLKey) config.keyUp)));
      scaledglprintf(0.005, 0.005, tmp);
      glTranslatef(0, -1, 0);
      if (substatus != 1)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp, "PRESS A KEY FOR DOWN: %s", strupr(SDL_GetKeyName((SDLKey) config.keyDown)));
      scaledglprintf(0.005, 0.005, tmp);
      glTranslatef(0, -1, 0);
      if (substatus != 2)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp,"PRESS A KEY FOR LEFT: %s", strupr(SDL_GetKeyName((SDLKey) config.keyLeft)));
      scaledglprintf(0.005, 0.005, tmp);
      glTranslatef(0, -1, 0);
      if (substatus != 3)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp, "PRESS A KEY FOR RIGHT: %s", strupr(SDL_GetKeyName((SDLKey) config.keyRight)));
      scaledglprintf(0.005, 0.005, tmp);
      glTranslatef(0, -1, 0);
      if (substatus != 4)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp, "PRESS A KEY FOR FIRE: %s", strupr(SDL_GetKeyName((SDLKey) config.keyFire)));
      scaledglprintf(0.005, 0.005, tmp);

      glTranslatef(0, -1, 0);
      if (substatus != 5)
        glColor3f(0.5, 0.5, 1.0);
      else
        glColor3f(1.0, 0.0, 0.0);
      sprintf(tmp, "PRESS A KEY FOR PAUSE/MENU: %s", strupr(SDL_GetKeyName((SDLKey) config.keyPause)));
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
