#include <GL/gl.h>
#include <GL/glu.h>

#include "glprintf.h"
#include "nether.h"
#include "optionsscreen.h"

extern int up_key,down_key,left_key,right_key,fire_key,pause_key;


void OptionsScreen::draw(int w, int h, const float lightpos[4])
{
  float tmpls[4] = {1.0F, 1.0F, 1.0F, 1.0};
  float tmpld[4] = {0.6F, 0.6F, 0.6F, 1.0};
  float tmpla[4] = {0.2F, 0.2F, 0.2F, 1.0};
  int splitx[2] = {int(w * 0.3), int(w * 0.7)};
  int splity[2] = {int(h * 0.3), int(h * 0.7)};

  /* Enable Lights, etc.: */
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, tmpla);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, tmpld);
  glLightfv(GL_LIGHT0, GL_SPECULAR, tmpls);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glEnable(GL_SCISSOR_TEST);
  glEnable(GL_DEPTH_TEST);

  /* Draw the MENU: */
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  glClearColor(0, 0, 0, 0.0);
  glViewport(splitx[0], splity[0], splitx[1] - splitx[0], splity[1] - splity[0]);
  float ratio = float(splitx[1] - splitx[0]) / float(splity[1] - splity[0]);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, ratio, 1.0, 1024.0);
  glScissor(splitx[0], splity[0], splitx[1] - splitx[0], splity[1] - splity[0]);
  gluLookAt(0, 0, 30, 0, 0, 0, 0, 1, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (nether->game_state == NETHER::STATE::PAUSE) {
    if (selectedOption == 0)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, 5, 0);
    scaledglprintf(0.01, 0.01, "RETURN TO GAME");

    if (selectedOption == 1)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -5,0);
    scaledglprintf(0.01, 0.01, "LOAD GAME");

    if (selectedOption == 2)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -3, 0);
    scaledglprintf(0.01, 0.01, "SAVE GAME");

    if (selectedOption == 3)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -3, 0);
    scaledglprintf(0.01, 0.01, "QUIT GAME");
  }

  if (nether->game_state == NETHER::STATE::SAVINGGAME) {
    glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, 6, 0);
    scaledglprintf(0.01, 0.01, "CHOOSE SLOT TO SAVE");

    if (selectedOption == 0)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -4, 0);
    scaledglprintf(0.01, 0.01, "CANCEL");

    for (int i = 0;i < 4; i++) {
      if (selectedOption == (i + 1))
        glColor3f(1.0, 0.0, 0.0);
      else
        glColor3f(0.5, 0.5, 1.0);
      glTranslatef(0, -2,0);
      char filename[80];
      sprintf(filename, "savedgame%i.txt", i);
      FILE *fp = fopen(filename, "r");
      if (fp == 0) {
        scaledglprintf(0.01, 0.01, "SLOT%i - EMPTY", i + 1);
      } else {
        scaledglprintf(0.01, 0.01, "SLOT%i - GAME SAVED", i + 1);
        fclose(fp);
      }
    }
  }

  if (nether->game_state == NETHER::STATE::LOADINGGAME) {
    glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, 6, 0);
    scaledglprintf(0.01, 0.01, "CHOOSE SLOT TO LOAD");

    if (selectedOption == 0)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -4,0);
    scaledglprintf(0.01, 0.01, "CANCEL");

    for (int i = 0; i < 4; i++) {
      if (selectedOption == (i + 1))
        glColor3f(1.0, 0.0, 0.0);
      else
        glColor3f(0.5, 0.5, 1.0);
      glTranslatef(0, -2,0);
      char filename[80];
      sprintf(filename, "savedgame%i.txt", i);
      FILE *fp = fopen(filename, "r");
      if (fp == 0) {
        scaledglprintf(0.01, 0.01, "SLOT%i - EMPTY", i + 1);
      } else {
        scaledglprintf(0.01, 0.01, "SLOT%i - GAME SAVED", i + 1);
        fclose(fp);
      }
    }
  }
}


bool OptionsScreen::cycle(unsigned char *keyboard)
{
  switch (nether->game_state) {
  case NETHER::STATE::PAUSE:
    if (keyboard[fire_key] && !nether->old_keyboard[fire_key]) {
      switch(selectedOption) {
      case 0:
        nether->game_state = NETHER::STATE::PLAYING;
        break;
      case 1:
        nether->game_state = NETHER::STATE::LOADINGGAME;
        selectedOption=0;
        break;
      case 2:
        nether->game_state = NETHER::STATE::SAVINGGAME;
        selectedOption=0;
        break;
      case 3:
        return false;
        break;
      }
    }

    if (keyboard[down_key] && !nether->old_keyboard[down_key]) {
      selectedOption++;
      if (selectedOption >= 4) selectedOption = 0;
    }

    if (keyboard[up_key] && !nether->old_keyboard[up_key]) {
      selectedOption--;
      if (selectedOption < 0) selectedOption = 3;
    }
    break;

  case NETHER::STATE::SAVINGGAME:
    if (keyboard[fire_key] && !nether->old_keyboard[fire_key]) {
      switch(selectedOption) {
      case 0:
        nether->game_state = NETHER::STATE::PAUSE;
        selectedOption = 2;
        break;
      case 1:
      case 2:
      case 3:
      case 4:
        {
          char filename[80];
          sprintf(filename,"savedgame%i.txt", selectedOption-1);
          nether->saveGame(filename);
          nether->saveDebugReport("debugreport.txt");
          nether->game_state = NETHER::STATE::PAUSE;
          selectedOption = 2;
          nether->menu.needsRedraw = 2;
          nether->radar.needsRedraw = 1;
        }
      }
    }

    if (keyboard[down_key] && !nether->old_keyboard[down_key]) {
      selectedOption++;
      if (selectedOption >= 5) selectedOption = 0;
    }

    if (keyboard[up_key] && !nether->old_keyboard[up_key]) {
      selectedOption--;
      if (selectedOption < 0) selectedOption = 4;
    }
    break;

  case NETHER::STATE::LOADINGGAME:
    if (keyboard[fire_key] && !nether->old_keyboard[fire_key]) {
      switch (selectedOption) {
      case 0:
        nether->game_state = NETHER::STATE::PAUSE;
        selectedOption = 1;
        break;
      case 1:
      case 2:
      case 3:
      case 4:
        {
          char filename[80];
          sprintf(filename, "savedgame%i.txt", selectedOption - 1);
          nether->menu.killmenu(nether->menu.act_menu);
          nether->loadGame(filename);
          nether->menu.newmenu(nether->menu.act_menu);
          nether->menu.needsRedraw = 2;
          nether->stats.requestRecomputing();
          nether->game_finished = 0;
          nether->game_started = INTRO_TIME;
          nether->game_state = NETHER::STATE::PAUSE;
          selectedOption = 2;
        }
      }
    }

    if (keyboard[down_key] && !nether->old_keyboard[down_key]) {
      selectedOption++;
      if (selectedOption >= 5) selectedOption=0;
    }

    if (keyboard[up_key] && !nether->old_keyboard[up_key]) {
      selectedOption--;
      if (selectedOption < 0) selectedOption = 4;
    }
    break;
  }

  return true;
}


void OptionsScreen::open()
{
  nether->game_state = NETHER::STATE::PAUSE;
  selectedOption = 0;
}
