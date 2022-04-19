#include "robot.h"
#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <sstream>

#include <GL/gl.h>
#include <GL/glu.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <sexp/util.hpp>

#include "3dobject.h"
#include "cmc.h"
#include "glprintf.h"
#include "menu.h"
#include "myglutaux.h"
#include "nether.h"
#include "piece3dobject.h"
#include "shadow3dobject.h"
#include "statusbutton.h"
#include "vector.h"


extern int up_key, down_key, left_key, right_key, fire_key, pause_key;

const char* BUILDINGLABELS[] = {"%i WARBASES %i",
                                "%i ELECTR'S %i",
                                "%i NUCLEAR  %i",
                                "%i PHASERS  %i",
                                "%i MISSILES %i",
                                "%i  CANNON  %i",
                                "%i CHASSIS  %i"};
const char* RESOURCELABELS[] = {"GENERAL %.2i",
                                "ELECTR' %.2i",
                                "NUCLEAR %.2i",
                                "PHASERS %.2i",
                                "MISSILE %.2i",
                                "CANNON  %.2i",
                                "CHASSIS %.2i"};


void Menu::draw(int width, int height)
{
  float lightpos2[4] = {0, 0, 1000, 0};
  int split = int((width * 25.0F) / 32.0F);

  glLightfv(GL_LIGHT0, GL_POSITION, lightpos2);

  glClearColor(0, 0, 0.2, 0);
  glViewport(split, 0, width - split, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, float(width - split), 0, height, -100, 100);
  glScissor(split, 0, width - split, height);
  glScalef(width / 640.0, height / 480.0,1);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  for (auto& b: buttons) {
    b.draw();
  }
  drawStatus();
}


void Menu::drawStatus()
{
  glPushMatrix();
  switch (activeMenu) {
  case TYPE::GENERAL:
    {
      StatusButton& b = findButton(StatusButton::NAME::STATUS);
      if (b.visible()) {
        glColor3f(0.5f, 0.5f, 1.0f);
        glTranslatef(70, 356, 0);

        auto buildingStats = nether->getBuildingStats();
        for (int i = 0; i < buildingStats.size(); i++) {
          scaledglprintf(0.1f, 0.1f, BUILDINGLABELS[i],
                         buildingStats[i].first,
                         buildingStats[i].second),
          glTranslatef(0, -18, 0);
        }

        scaledglprintf(0.1f,0.1f,"%.2i ROBOTS %.2i",
                       nether->map.robots.getRobotCount(1),
                       nether->map.robots.getRobotCount(0));
        glColor3f(0.0f,0.8f,0.0f);
        glTranslatef(0,-65,0);

        auto resourceStats = nether->getResourceStats();
        for (int i = 0; i < resourceStats.size(); i++) {
          scaledglprintf(0.1f, 0.1f, RESOURCELABELS[i], resourceStats[i].first);
          glTranslatef(0, -18, 0);
        }
      }
    }
    break;
  case TYPE::ROBOT:
  case TYPE::DIRECTCONTROL:
    {
      StatusButton& b = findButton(StatusButton::NAME::ROBOT1);
      if (b.visible()) {
        glTranslatef(70,140,0);
        glColor3f(1.0f,1.0f,0.0);
        scaledglprintf(0.1f,0.1f,"-ORDERS-");
        glColor3f(0.5f,0.5f,1.0f);
        switch(nether->getControlled()->program.type) {
        case RobotProgram::STOPDEFEND:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"STOP");
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"AND");
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"DEFEND");
          break;
        case RobotProgram::ADVANCE:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"ADVANCE");
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"%.2i", nether->getControlled()->program.parameter.as_int / 2);
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"MILES");
          break;
        case RobotProgram::RETREAT:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"RETREAT");
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"%.2i",nether->getControlled()->program.parameter.as_int / 2);
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"MILES");
          break;
        case RobotProgram::DESTROY:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"DESTROY");
          switch(nether->getControlled()->program.parameter.param) {
          case RobotProgram::ROBOTS:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ROBOTS");
            break;
          case RobotProgram::WARBASES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"WARBASES");
            break;
          case RobotProgram::NEUTRAL_FACTORIES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"NEUTRAL");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"FACTORIES");
            break;
          case RobotProgram::ENEMY_FACTORIES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"FACTORIES");
            break;
          }
          break;
        case RobotProgram::CAPTURE:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"CAPTURE");
          switch(nether->getControlled()->program.parameter.param) {
          case RobotProgram::ROBOTS:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ROBOTS");
            break;
          case RobotProgram::WARBASES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"WARBASES");
            break;
          case RobotProgram::NEUTRAL_FACTORIES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"NEUTRAL");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"FACTORIES");
            break;
          case RobotProgram::ENEMY_FACTORIES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"FACTORIES");
            break;
          }
          break;
        }

        glTranslatef(0,-44,0);
        glColor3f(1.0f,1.0f,0.0);
        scaledglprintf(0.1f,0.1f,"STRENGTH");
        glTranslatef(0,-18,0);
        glColor3f(1.0f,1.0f,1.0f);
        scaledglprintf(0.1f,0.1f,"%.3i%c", nether->getControlled()->strength,'%');
      }
    }
    break;

  case TYPE::COMBATMODE:
  case TYPE::DIRECTCONTROL2:
    glTranslatef(70,40,0);
    glColor3f(1.0f,1.0f,0.0);
    scaledglprintf(0.1f,0.1f,"STRENGTH");
    glTranslatef(0,-18,0);
    glColor3f(1.0f,1.0f,1.0f);
    scaledglprintf(0.1f,0.1f,"%.3i%c",nether->getControlled()->strength,'%');
    break;

  case TYPE::ORDERS:
      {
        StatusButton& b = findButton(StatusButton::NAME::ORDERS1);
        if (b.visible()) {
          glTranslatef(70,400,0);
          glColor3f(1.0f,1.0f,1.0f);
          scaledglprintf(0.1f,0.1f,"SELECT");
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"ORDERS");

          glTranslatef(0,-340,0);
          glColor3f(1.0f,1.0f,0.0);
          scaledglprintf(0.1f,0.1f,"STRENGTH");
          glTranslatef(0,-18,0);
          glColor3f(1.0f,1.0f,1.0f);
          scaledglprintf(0.1f,0.1f,"%.3i%c", nether->getControlled()->strength,'%');
        }
      }
      break;

  case TYPE::SELECTDISTANCE:
    {
      glTranslatef(70,300,0);
      glColor3f(0.5f,0.5f,1.0f);
      scaledglprintf(0.1f,0.1f,"SELECT");
      glTranslatef(0,-20,0);
      scaledglprintf(0.1f,0.1f,"DISTANCE");

      glColor3f(1.0f,1.0f,0.0);
      glTranslatef(0,-40,0);
      scaledglprintf(0.1f,0.1f,"%.2i MILES", nether->getControlled()->program.parameter.as_int / 2);

      glTranslatef(0,-200,0);
      glColor3f(1.0f,1.0f,0.0);
      scaledglprintf(0.1f,0.1f,"STRENGTH");
      glTranslatef(0,-18,0);
      glColor3f(1.0f,1.0f,1.0f);
      scaledglprintf(0.1f,0.1f,"%.3i%c", nether->getControlled()->strength,'%');
    }
    break;

  case TYPE::TARGET_DESTROY:
  case TYPE::TARGET_CAPTURE:
    {
      glTranslatef(70,350,0);
      glColor3f(0.5f,0.5f,1.0f);
      scaledglprintf(0.1f,0.1f,"SELECT");
      glTranslatef(0,-20,0);
      scaledglprintf(0.1f,0.1f,"TARGET");

      glTranslatef(0,-290,0);
      glColor3f(1.0f,1.0f,0.0);
      scaledglprintf(0.1f,0.1f,"STRENGTH");
      glTranslatef(0,-18,0);
      glColor3f(1.0f,1.0f,1.0f);
      scaledglprintf(0.1f,0.1f,"%.3i%c", nether->getControlled()->strength,'%');
    }
    break;

  }
  glPopMatrix();
}


void Menu::cycle(unsigned char* keyboard)
{
  switch (activeMenu) {
  case Menu::TYPE::GENERAL:
    break;

  case Menu::TYPE::ROBOT:
    /* Browsing through the ROBOT MENU: */
    {
      if (handleKeys(keyboard)) {
        switch (activeButton) {
        case StatusButton::NAME::ROBOT1:
          {
            findButton(activeButton).toggle();
            activeMenu = Menu::TYPE::DIRECTCONTROL;
            nether->sManager.playSelect();
          }
          break;
        case StatusButton::NAME::ROBOT2:
          {
            activateMenu(Menu::TYPE::ORDERS, StatusButton::NAME::ORDERS1);
            nether->sManager.playSelect();
          }
          break;
        case StatusButton::NAME::ROBOT3:
          {
            activateMenu(Menu::TYPE::COMBATMODE, StatusButton::NAME::COMBAT6);
            nether->sManager.playSelect();
          }
          break;
        case StatusButton::NAME::ROBOT4:
          /* Back to the general menu: */
          nether->detachShip(nether->getControlled());
          nether->getShip()->op3 = Ship::OPS::UP;
          nether->sManager.playSelect();
          break;
        }
      }
    }
    break;

  case Menu::TYPE::DIRECTCONTROL:
    /* Direct control of a robot by the user: */
    if (keyboard[fire_key] > 1) {
      findButton(activeButton).untoggle();
      activeMenu = Menu::TYPE::ROBOT;
    }
    break;

  case Menu::TYPE::DIRECTCONTROL2:
    /* Direct control of a robot by the user: */
    if (keyboard[fire_key] > 1) {
      findButton(activeButton).untoggle();
      activeMenu = Menu::TYPE::COMBATMODE;
    }
    break;

  case Menu::TYPE::COMBATMODE:
    /* Browsing through the COMBAT MENU: */
    {
      if (handleKeys(keyboard)) {
        switch (activeButton) {
        case StatusButton::NAME::COMBAT1:
          /* Fire Nuclear: */
          if ((nether->getControlled()->getAngle() % 90 == 0) &&
              nether->getControlled()->hasNuclear() &&
              nether->getControlled()->firetimer == 0) {
            nether->getControlled()->op=Robot::OPERATOR::NUCLEAR;
          }
          break;
        case StatusButton::NAME::COMBAT2:
          /* Fire Phasers: */
          if ((nether->getControlled()->getAngle() % 90 == 0) &&
              nether->getControlled()->hasPhasers() &&
              nether->getControlled()->op == Robot::OPERATOR::NONE) {
            nether->getControlled()->op = Robot::OPERATOR::PHASERS;
          }
          break;
        case StatusButton::NAME::COMBAT3:
          /* Fire Missiles: */
          if ((nether->getControlled()->getAngle() % 90 == 0) &&
              nether->getControlled()->hasMissiles() &&
              nether->getControlled()->op == Robot::OPERATOR::NONE) {
            nether->getControlled()->op = Robot::OPERATOR::MISSILES;
          }
          break;
        case StatusButton::NAME::COMBAT4:
          /* Fire Canons: */
          if ((nether->getControlled()->getAngle() % 90 == 0) &&
              nether->getControlled()->hasCannons() &&
              nether->getControlled()->op == Robot::OPERATOR::NONE) {
            nether->getControlled()->op = Robot::OPERATOR::CANNONS;
          }
          break;
        case StatusButton::NAME::COMBAT5:
          {
            findButton(activeButton).toggle();
            activeMenu = Menu::TYPE::DIRECTCONTROL2;
            nether->sManager.playSelect();
          }
          break;
        case StatusButton::NAME::COMBAT6:
          /* Back to the robot menu: */
          activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT3);
          nether->sManager.playSelect();
          break;
        }
      }
    }
    break;
  case Menu::TYPE::ORDERS:
    /* Browsing through the ORDERS MENU: */
    {
      if (handleKeys(keyboard)) {
        switch (activeButton) {
        case StatusButton::NAME::ORDERS1:
          /* STOP & DEFEND: */
          nether->getControlled()->program.type = RobotProgram::STOPDEFEND;
          nether->getControlled()->program.goal = Vector(-1, -1, -1);
          activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
          nether->sManager.playSelect();
          break;
        case StatusButton::NAME::ORDERS2:
          /* ADVANCE ?? MILES: */
          nether->getControlled()->program.type = RobotProgram::ADVANCE;
          nether->getControlled()->program.parameter.as_int = 0;
          nether->getControlled()->program.goal = Vector(-1, -1, -1);

          activateMenu(Menu::TYPE::SELECTDISTANCE, StatusButton::NAME::NONE);
          nether->sManager.playSelect();
          break;
        case StatusButton::NAME::ORDERS3:
          /* RETREAT ?? MILES: */
          nether->getControlled()->program.type = RobotProgram::RETREAT;
          nether->getControlled()->program.parameter.as_int = 0;
          nether->getControlled()->program.goal = Vector(-1, -1, -1);

          activateMenu(Menu::TYPE::SELECTDISTANCE, StatusButton::NAME::NONE);
          nether->sManager.playSelect();
          break;
        case StatusButton::NAME::ORDERS4:
          /* SEARCH AND DESTROY: */
          activateMenu(Menu::TYPE::TARGET_DESTROY, StatusButton::NAME::TARGET1);
          nether->sManager.playSelect();
          break;
        case StatusButton::NAME::ORDERS5:
          activateMenu(Menu::TYPE::TARGET_CAPTURE, StatusButton::NAME::TARGET1);
          nether->sManager.playSelect();
          break;
        }
      }
    }
    break;

  case Menu::TYPE::SELECTDISTANCE:
    {
      if (keyboard[up_key] > 1) {
        nether->getControlled()->program.parameter.as_int =
          std::min(190, nether->getControlled()->program.parameter.as_int + 10);
        nether->getControlled()->program.goal = Vector(-1, -1, -1);
      }
      if (keyboard[down_key] > 1) {
        nether->getControlled()->program.parameter.as_int =
          std::max(0, nether->getControlled()->program.parameter.as_int - 10);
        nether->getControlled()->program.goal = Vector(-1, -1, -1);
      }
      if (keyboard[fire_key] > 1) {
        if (nether->getControlled()->program.parameter.as_int == 0)
          nether->getControlled()->program.type = RobotProgram::STOPDEFEND;
        nether->getControlled()->program.goal = Vector(-1, -1, -1);

        activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
        nether->sManager.playSelect();
      }
    }
    break;

  case Menu::TYPE::TARGET_DESTROY:
    /* Browsing through the SELECT TARGET FOR DESTROYING MENU: */
    {
      if (handleKeys(keyboard)) {
        switch (activeButton) {
        case StatusButton::NAME::TARGET1:
          if (nether->getControlled()->hasCannons() ||
              nether->getControlled()->hasMissiles() ||
              nether->getControlled()->hasPhasers()) {
            activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
            nether->getControlled()->program.type = RobotProgram::DESTROY;
            nether->getControlled()->program.parameter.param = RobotProgram::ROBOTS;
            nether->getControlled()->program.goal = Vector(-1, -1, -1);
            nether->sManager.playSelect();
          } else {
            /* The robot has no standard WEAPONS!: */
            nether->sManager.playWrong();
          }
          break;
        case StatusButton::NAME::TARGET2:
          if (nether->getControlled()->hasNuclear()) {
            activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
            nether->getControlled()->program.type = RobotProgram::DESTROY;
            nether->getControlled()->program.parameter.param = RobotProgram::ENEMY_FACTORIES;
            nether->getControlled()->program.goal = Vector(-1, -1, -1);
            nether->sManager.playSelect();
          } else {
            /* The robot has no NUCLEAR weapons: */
            nether->sManager.playWrong();
          }
          break;
        case StatusButton::NAME::TARGET3:
          if (nether->getControlled()->hasNuclear()) {
            activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
            nether->getControlled()->program.type = RobotProgram::DESTROY;
            nether->getControlled()->program.parameter.param = RobotProgram::WARBASES;
            nether->getControlled()->program.goal = Vector(-1, -1, -1);
            nether->sManager.playSelect();
          } else {
            /* The robot has no NUCLEAR weapons: */
            nether->sManager.playWrong();
          }
          break;
        }
      }
    }
    break;

  case Menu::TYPE::TARGET_CAPTURE:
    /* Browsing through the SELECT TARGET FOR CAPTURING MENU: */
    {
      if (handleKeys(keyboard)) {
        switch (activeButton) {
        case StatusButton::NAME::TARGET11:
          activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
          nether->getControlled()->program.type = RobotProgram::CAPTURE;
          nether->getControlled()->program.parameter.param = RobotProgram::NEUTRAL_FACTORIES;
          nether->getControlled()->program.goal = Vector(-1, -1, -1);
          nether->sManager.playSelect();
          break;
        case StatusButton::NAME::TARGET21:
          activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
          nether->getControlled()->program.type = RobotProgram::CAPTURE;
          nether->getControlled()->program.parameter.param = RobotProgram::ENEMY_FACTORIES;
          nether->getControlled()->program.goal = Vector(-1, -1, -1);
          nether->sManager.playSelect();
          break;
        case StatusButton::NAME::TARGET31:
          activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
          nether->getControlled()->program.type = RobotProgram::CAPTURE;
          nether->getControlled()->program.parameter.param = RobotProgram::WARBASES;
          nether->getControlled()->program.goal = Vector(-1,-1,-1);
          nether->sManager.playSelect();
          break;
        }
      }
    }
    break;
  default:
    break;
  }

  for (auto& b : buttons) {
    b.cycle();
  }
}


void Menu::showMenu(TYPE menu)
{
  switch(menu) {
  case TYPE::GENERAL:
    showButtons({StatusButton::NAME::TIME, StatusButton::NAME::STATUS,
                 StatusButton::NAME::RESOURCE});
    activeMenu = TYPE::GENERAL;
    break;

  case TYPE::ROBOT:
    showButtons({StatusButton::NAME::ROBOT1, StatusButton::NAME::ROBOT2,
                 StatusButton::NAME::ROBOT3, StatusButton::NAME::ROBOT4});
    activeMenu = TYPE::ROBOT;
    break;

  case TYPE::COMBATMODE:
    showButtons({StatusButton::NAME::COMBAT1, StatusButton::NAME::COMBAT2,
                 StatusButton::NAME::COMBAT3, StatusButton::NAME::COMBAT4,
                 StatusButton::NAME::COMBAT5, StatusButton::NAME::COMBAT6});
    activeMenu = TYPE::COMBATMODE;
    break;

  case TYPE::ORDERS:
    showButtons({StatusButton::NAME::ORDERS1, StatusButton::NAME::ORDERS2,
                 StatusButton::NAME::ORDERS3, StatusButton::NAME::ORDERS4,
                 StatusButton::NAME::ORDERS5});
    activeMenu = TYPE::ORDERS;
    break;

  case TYPE::SELECTDISTANCE:
    if (nether->getControlled()->program.type == RobotProgram::ADVANCE)
      showButtons({StatusButton::NAME::ORDERS11});
    if (nether->getControlled()->program.type == RobotProgram::RETREAT)
      showButtons({StatusButton::NAME::ORDERS12});
    activeMenu = TYPE::SELECTDISTANCE;
    break;

  case TYPE::TARGET_DESTROY:
    showButtons({StatusButton::NAME::ORDERS13, StatusButton::NAME::TARGET1,
                 StatusButton::NAME::TARGET2, StatusButton::NAME::TARGET3});
    activeMenu = TYPE::TARGET_DESTROY;
    break;

  case TYPE::TARGET_CAPTURE:
    showButtons({StatusButton::NAME::ORDERS, StatusButton::NAME::TARGET11,
                 StatusButton::NAME::TARGET21, StatusButton::NAME::TARGET31});
    activeMenu = TYPE::TARGET_CAPTURE;
    break;
  }
}


void Menu::hideMenu()
{
  switch (activeMenu) {
  case TYPE::GENERAL:
    hideButtons({StatusButton::NAME::STATUS, StatusButton::NAME::RESOURCE});
    break;

  case TYPE::ROBOT:
    hideButtons({StatusButton::NAME::ROBOT1, StatusButton::NAME::ROBOT2,
                 StatusButton::NAME::ROBOT3, StatusButton::NAME::ROBOT4});
    break;

  case TYPE::COMBATMODE:
    hideButtons({StatusButton::NAME::COMBAT1, StatusButton::NAME::COMBAT2,
                 StatusButton::NAME::COMBAT3, StatusButton::NAME::COMBAT4,
                 StatusButton::NAME::COMBAT5, StatusButton::NAME::COMBAT6});
    break;

  case TYPE::ORDERS:
    hideButtons({StatusButton::NAME::ORDERS1, StatusButton::NAME::ORDERS2,
                 StatusButton::NAME::ORDERS3, StatusButton::NAME::ORDERS4,
                 StatusButton::NAME::ORDERS5});
    break;

  case TYPE::SELECTDISTANCE:
    hideButtons({StatusButton::NAME::ORDERS11, StatusButton::NAME::ORDERS12});
    break;

  case TYPE::TARGET_DESTROY:
    hideButtons({StatusButton::NAME::ORDERS13, StatusButton::NAME::TARGET1,
                 StatusButton::NAME::TARGET2, StatusButton::NAME::TARGET3});
    break;

  case TYPE::TARGET_CAPTURE:
    hideButtons({StatusButton::NAME::ORDERS, StatusButton::NAME::TARGET11,
                 StatusButton::NAME::TARGET21, StatusButton::NAME::TARGET31});
    break;

  case TYPE::ALL:
    hideButtons({StatusButton::NAME::STATUS, StatusButton::NAME::RESOURCE,
                 StatusButton::NAME::ROBOT1, StatusButton::NAME::ROBOT2,
                 StatusButton::NAME::ROBOT3, StatusButton::NAME::ROBOT4,
                 StatusButton::NAME::COMBAT1, StatusButton::NAME::COMBAT2,
                 StatusButton::NAME::COMBAT3, StatusButton::NAME::COMBAT4,
                 StatusButton::NAME::COMBAT5, StatusButton::NAME::COMBAT6,
                 StatusButton::NAME::ORDERS1, StatusButton::NAME::ORDERS2,
                 StatusButton::NAME::ORDERS3, StatusButton::NAME::ORDERS4,
                 StatusButton::NAME::ORDERS5, StatusButton::NAME::ORDERS,
                 StatusButton::NAME::ORDERS, StatusButton::NAME::TARGET1,
                 StatusButton::NAME::TARGET2, StatusButton::NAME::TARGET3});
    break;
  }
}


void Menu::hideButtons(const std::unordered_set<StatusButton::NAME>& ids)
{
  for (auto& b : buttons) {
    if (ids.count(b.id)) {
      b.status = 1;
    }
    if (b.isInteractive())
      b.color = Color(0.0f, 0.0f, 0.8f);
  }
}


void Menu::showButtons(const std::unordered_set<StatusButton::NAME>& ids)
{
  for (auto& b : buttons) {
    if (ids.count(b.id)) {
      if (b.id == StatusButton::NAME::TIME) {
        b.status = 0;
      } else {
        b.status = -32;
      }
    }
  }
}


void Menu::activateMenu(TYPE newMenu, StatusButton::NAME newActiveButton)
{
  hideMenu();
  showMenu(newMenu);
  activeButton = newActiveButton;
  if (findButton(activeButton).isInteractive())
    findButton(activeButton).color = Color(0.5f, 0.5f, 1.0f);
}


bool Menu::handleKeys(unsigned char* keyboard)
{
  if (keyboard[up_key] > 1 || keyboard[down_key] > 1) {
    int index = 0;
    for (auto& b : buttons) {
      if (b.id == activeButton)
        break;
      index++;
    }
    if (keyboard[up_key] > 1) {
      for (index = (index - 1 + buttons.size()) % buttons.size();
           !buttons[index].isInteractive();
           index = (index - 1 + buttons.size()) % buttons.size());
    }

    if (keyboard[down_key] > 1) {
      for (index = ++index % buttons.size();
           !buttons[index].isInteractive();
           index = ++index % buttons.size());
    }

    activeButton = buttons[index].id;

    for (auto& button: buttons) {
      if (button.isInteractive()) {
        if (button.id == activeButton) {
          button.color = Color(0.5f, 0.5f, 1.0f);
        } else {
          button.color = Color(0.0f, 0.0f, 0.8f);
        }
      }
    }
  }

  return keyboard[fire_key] > 1;
}


void Menu::updateTime(const Stats& stats)
{
  StatusButton& timeb = findButton(StatusButton::NAME::TIME);
  if (timeb.visible()) {
    std::ostringstream t1Formatter;
    t1Formatter << "Day: " << stats.day;
    timeb.text1 = t1Formatter.str();
    std::ostringstream t2Formatter;
    t2Formatter << "Hour: " << std::setw(2) << stats.hour << ':' << std::setw(2) << std::setfill('0') << stats.minute;
    timeb.text2 = t2Formatter.str();
  }
}


std::ostream& operator<<(std::ostream& out, const Menu& menu)
{
  return out << int(menu.activeMenu) << ' ' << int(menu.activeButton);
}


std::istream& operator>>(std::istream& in, Menu& menu)
{
  int actMenu_, actButton_;
  in >> actMenu_ >> actButton_;
  menu.activateMenu(Menu::TYPE(actMenu_), StatusButton::NAME(actButton_));
  return in;
}


StatusButton& Menu::findButton(StatusButton::NAME id)
{
  for (auto& b : buttons) {
    if (b.id == id) {
      return b;
    }
  }
  return buttons[0];
}


sexp::Value Menu::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("menu"),
    sexp::Value::integer((int)activeMenu),
    sexp::Value::integer((int)activeButton)
  );
}


bool Menu::fromSexp(const sexp::Value& value)
{
  activeMenu = (Menu::TYPE)sexp::cdar(value).as_int();
  activeButton = (StatusButton::NAME)sexp::cddar(value).as_int();

  return true;
}
