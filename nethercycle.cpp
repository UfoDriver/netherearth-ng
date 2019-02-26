#ifdef _WIN32
#include "windows.h"
#endif

#include "string.h"
#include "stdlib.h"

#include <algorithm>
#include <iomanip>

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

#include "glprintf.h"

extern int detaillevel;
extern int frames_per_sec;
extern bool fullscreen;
extern int shadows;
extern bool sound;
extern int up_key, down_key, left_key, right_key, fire_key, pause_key;
extern int level;
extern float MINY, MAXY, MINX, MAXX;


bool NETHER::cycle(unsigned char *keyboard)
{
  camera.cycle(keyboard);
  stats.recompute(map.buildings);

  /* ENEMY Artificial Intelligence: */
  if (stats.second == 0) {
    if (level == 0 && (stats.hour & 0x01) == 0 && stats.minute == 0) ai.enemy();
    if (level == 1 && stats.minute == 0) ai.enemy();
    if (level >= 2 && (stats.minute == 0 || stats.minute == 30)) ai.enemy();
  }

  /* GAME Cycle: */
  ship->landed = false;
  switch(menu.act_menu) {
  case Menu::TYPE::GENERAL:
    /* Free movement of the ship through the map: */
    {
      float x[2], y[2];
      float minz;
      Vector old_shipp;

      ship->pos.x = (int(ship->pos.x * 8.0)) / 8.0;
      ship->pos.y = (int(ship->pos.y * 8.0)) / 8.0;
      old_shipp = ship->pos;
      x[0] = ship->pos.x;
      x[1] = ship->pos.x + 1.0;
      y[0] = ship->pos.y;
      y[1] = ship->pos.y + 1.0;
      minz = map.maxZ(x, y);

      if (ship->op == Ship::OPS::RIGHT)
        if (ship->pos.x < map.width() - 1) {
          ship->pos.x += 0.125;
          if (ship->timemoving >= 50 && (int(ship->pos.x * 8) % 2) == 1)
            ship->pos.x += 0.125;
        }
      if (ship->op == Ship::OPS::LEFT)
        if (ship->pos.x>0) {
          ship->pos.x -= 0.125;
          if (ship->timemoving >= 50 && (int(ship->pos.x * 8) % 2) == 1)
            ship->pos.x -= 0.125;
        }
      if (ship->op2 == Ship::OPS::FORWARD)
        if (ship->pos.y < map.height() - 1) {
          ship->pos.y += 0.125;
          if (ship->timemoving >= 50 && (int(ship->pos.y * 8) % 2) == 1)
            ship->pos.y += 0.125;
        }
      if (ship->op2==Ship::OPS::BACKWARD)
        if (ship->pos.y > 0) {
          ship->pos.y -= 0.125;
          if (ship->timemoving >= 50 && (int(ship->pos.y * 8) % 2) == 1)
            ship->pos.y -= 0.125;
        }
      if (ship->op3 == Ship::OPS::UP)
        if (ship->pos.z < 5.0)
          ship->pos.z += 0.05;
      if (ship->op3 != Ship::OPS::UP && ship->pos.z > minz)
        ship->pos.z -= 0.025;
      if (ship->pos.z < minz)
        ship->pos.z = minz;

      if (ship->op == Ship::OPS::NONE && ship->op2 == Ship::OPS::NONE) {
        ship->timemoving = 0;
      } else {
        ship->timemoving++;
      }

      if (ship->pos != old_shipp) {
        if (ship->checkCollision(map.buildings, map.robots)) {
          ship->timemoving = 0;
          Vector p = ship->pos;
          ship->pos.x = old_shipp.x;
          ship->pos.y = old_shipp.y;
          if (p.z != old_shipp.z && ship->checkCollision(map.buildings, map.robots)) {
            ship->pos.z = old_shipp.z;
            ship->landed = true;
          } else {
            ship->pos.z = p.z;
          }
          ship->pos.x = p.x;
          if (p.x != old_shipp.x && ship->checkCollision(map.buildings, map.robots)) {
            ship->pos.x = old_shipp.x;
          } else {
            ship->pos.x = p.x;
          }
          ship->pos.y = p.y;
          if (p.y != old_shipp.y && ship->checkCollision(map.buildings, map.robots)) {
            ship->pos.y = old_shipp.y;
          } else {
            ship->pos.y = p.y;
          }
        }
      }

      if ((int(ship->pos.x * 8) % 4) == 0)
        ship->op = Ship::OPS::NONE;
      if ((int(ship->pos.y * 8) % 4) == 0)
        ship->op2 = Ship::OPS::NONE;
      if ((int(ship->pos.z * 8) % 4) == 0)
        ship->op3 = Ship::OPS::NONE;

      if (keyboard[left_key]) {
        ship->op = Ship::OPS::LEFT;
      }
      if (keyboard[right_key]) {
        ship->op = Ship::OPS::RIGHT;
      }
      if (keyboard[up_key]) {
        ship->op2 = Ship::OPS::FORWARD;
      }
      if (keyboard[down_key]) {
        ship->op2 = Ship::OPS::BACKWARD;
      }
      if (keyboard[fire_key]) {
        ship->op3 = Ship::OPS::UP;
      }
    }
    break;

  case Menu::TYPE::ROBOT:
    /* Browsing through the ROBOT MENU: */
    {
      if (menu.handleKeys(keyboard)) {
        switch (menu.act_button) {
        case StatusButton::NAME::ROBOT1:
          {
            StatusButton *b = menu.getbutton(menu.act_button);
            if (b != 0) {
              b->color = Color(1.0f, 0.5f, 0.5f);
            }

            menu.act_menu = Menu::TYPE::DIRECTCONTROL;
            menu.needsRedraw=2;
            sManager.playSelect();
          }
          break;
        case StatusButton::NAME::ROBOT2:
          {
            menu.replaceMenu(Menu::TYPE::ROBOT, Menu::TYPE::ORDERS, StatusButton::NAME::ORDERS1);
            sManager.playSelect();
          }
          break;
        case StatusButton::NAME::ROBOT3:
          {
            menu.replaceMenu(Menu::TYPE::ROBOT, Menu::TYPE::COMBATMODE, StatusButton::NAME::COMBAT6);
            sManager.playSelect();
          }
          break;
        case StatusButton::NAME::ROBOT4:
          /* Back to the general menu: */
          controlled->electronics_state += 6;
          controlled->shipover = false;
          controlled = nullptr;
          menu.killmenu(Menu::TYPE::ROBOT);
          menu.newmenu(Menu::TYPE::GENERAL);
          ship->op3 = Ship::OPS::UP;
          sManager.playSelect();
          break;
        }
      }
    }
    break;

  case Menu::TYPE::DIRECTCONTROL:
    /* Direct control of a robot by the user: */
    if (keyboard[fire_key] > 1) {
      menu.needsRedraw = 2;
      menu.act_menu=Menu::TYPE::ROBOT;
    }
    break;

  case Menu::TYPE::DIRECTCONTROL2:
    /* Direct control of a robot by the user: */
    if (keyboard[fire_key] > 1) {
      menu.needsRedraw = 2;
      menu.act_menu=Menu::TYPE::COMBATMODE;
    }
    break;

  case Menu::TYPE::COMBATMODE:
    /* Browsing through the COMBAT MENU: */
    {
      if (menu.handleKeys(keyboard)) {
        switch(menu.act_button) {
        case StatusButton::NAME::COMBAT1:
          /* Fire Nuclear: */
          if ((controlled->angle == 0 || controlled->angle == 90 ||
               controlled->angle == 180 || controlled->angle == 270) &&
              controlled->hasNuclear() &&
              controlled->firetimer == 0) {
            controlled->op=Robot::OPERATOR::NUCLEAR;
          }
          break;
        case StatusButton::NAME::COMBAT2:
          /* Fire Phasers: */
          if ((controlled->angle == 0 || controlled->angle == 90 ||
               controlled->angle == 180 || controlled->angle == 270) &&
              controlled->hasPhasers() &&
              controlled->op == Robot::OPERATOR::NONE) {
            controlled->op = Robot::OPERATOR::PHASERS;
          }
          break;
        case StatusButton::NAME::COMBAT3:
          /* Fire Missiles: */
          if ((controlled->angle == 0 || controlled->angle == 90 ||
               controlled->angle == 180 || controlled->angle == 270) &&
              controlled->hasMissiles() &&
              controlled->op == Robot::OPERATOR::NONE) {
            controlled->op = Robot::OPERATOR::MISSILES;
          }
          break;
        case StatusButton::NAME::COMBAT4:
          /* Fire Canons: */
          if ((controlled->angle == 0 || controlled->angle == 90 ||
               controlled->angle == 180 || controlled->angle == 270) &&
              controlled->hasCannons() &&
              controlled->op == Robot::OPERATOR::NONE) {
            controlled->op = Robot::OPERATOR::CANNONS;
          }
          break;
        case StatusButton::NAME::COMBAT5:
          {
            StatusButton *b = menu.getbutton(menu.act_button);
            if (b != 0) {
              b->color = Color(1.0f, 0.5f, 0.5f);
            }

            menu.act_menu = Menu::TYPE::DIRECTCONTROL2;
            menu.needsRedraw = 2;
            sManager.playSelect();
          }
          break;
        case StatusButton::NAME::COMBAT6:
          /* Back to the robot menu: */
          menu.replaceMenu(Menu::TYPE::COMBATMODE, Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT3);
          sManager.playSelect();
          break;
        }
      }
    }
    break;
  case Menu::TYPE::ORDERS:
    /* Browsing through the ORDERS MENU: */
    {
      if (menu.handleKeys(keyboard)) {
        switch (menu.act_button) {
        case StatusButton::NAME::ORDERS1:
          /* STOP & DEFEND: */
          controlled->program = Robot::PROGRAM_STOPDEFEND;
          controlled->program_goal = Vector(-1, -1, -1);
          menu.replaceMenu(Menu::TYPE::ORDERS, Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
          sManager.playSelect();
          break;
        case StatusButton::NAME::ORDERS2:
          /* ADVANCE ?? MILES: */
          controlled->program = Robot::PROGRAM_ADVANCE;
          controlled->program_parameter.as_int = 0;
          controlled->program_goal = Vector(-1, -1, -1);

          menu.killmenu(Menu::TYPE::ORDERS);
          menu.newmenu(Menu::TYPE::SELECTDISTANCE);
          sManager.playSelect();
          break;
        case StatusButton::NAME::ORDERS3:
          /* RETREAT ?? MILES: */
          controlled->program = Robot::PROGRAM_RETREAT;
          controlled->program_parameter.as_int = 0;
          controlled->program_goal = Vector(-1, -1, -1);

          menu.killmenu(Menu::TYPE::ORDERS);
          menu.newmenu(Menu::TYPE::SELECTDISTANCE);
          sManager.playSelect();
          break;
        case StatusButton::NAME::ORDERS4:
          /* SEARCH AND DESTROY: */
          menu.replaceMenu(Menu::TYPE::ORDERS, Menu::TYPE::TARGET_DESTROY,
                           StatusButton::NAME::TARGET1);
          sManager.playSelect();
          break;
        case StatusButton::NAME::ORDERS5:
          menu.replaceMenu(Menu::TYPE::ORDERS, Menu::TYPE::TARGET_CAPTURE,
                           StatusButton::NAME::TARGET1);
          sManager.playSelect();
          break;
        }
      }
    }
    break;

  case Menu::TYPE::SELECTDISTANCE:
    {
      if (keyboard[up_key] > 1) {
        controlled->program_parameter.as_int += 10;
        if (controlled->program_parameter.as_int > 190)
          controlled->program_parameter.as_int = 190;
        controlled->program_goal = Vector(-1, -1, -1);
        menu.needsRedraw = 2;
      }
      if (keyboard[down_key] > 1) {
        controlled->program_parameter.as_int -= 10;
        if (controlled->program_parameter.as_int < 0)
          controlled->program_parameter.as_int = 0;
        controlled->program_goal = Vector(-1, -1, -1);

        menu.needsRedraw=2;
      }
      if (keyboard[fire_key] > 1) {
        if (controlled->program_parameter.as_int == 0)
          controlled->program = Robot::PROGRAM_STOPDEFEND;
        controlled->program_goal = Vector(-1, -1, -1);

        menu.replaceMenu(Menu::TYPE::SELECTDISTANCE, Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT2);
        sManager.playSelect();
      }
    }
    break;

  case Menu::TYPE::TARGET_DESTROY:
    /* Browsing through the SELECT TARGET FOR DESTROYING MENU: */
    {
      if (menu.handleKeys(keyboard)) {
        switch(menu.act_button) {
        case StatusButton::NAME::TARGET1:
          if (controlled->pieces[0] ||
              controlled->pieces[1] ||
              controlled->pieces[2]) {
            menu.replaceMenu(Menu::TYPE::TARGET_DESTROY, Menu::TYPE::ROBOT,
                             StatusButton::NAME::ROBOT2);
            controlled->program = Robot::PROGRAM_DESTROY;
            controlled->program_parameter.param = Robot::P_PARAM_ROBOTS;
            controlled->program_goal = Vector(-1, -1, -1);
            sManager.playSelect();
          } else {
            /* The robot has no standard WEAPONS!: */
            sManager.playWrong();
          }
          break;
        case StatusButton::NAME::TARGET2:
          if (controlled->pieces[3]) {
            menu.replaceMenu(Menu::TYPE::TARGET_DESTROY, Menu::TYPE::ROBOT,
                             StatusButton::NAME::ROBOT2);
            controlled->program = Robot::PROGRAM_DESTROY;
            controlled->program_parameter.param = Robot::P_PARAM_EFACTORIES;
            controlled->program_goal = Vector(-1, -1, -1);
            sManager.playSelect();
          } else {
            /* The robot has no NUCLEAR weapons: */
            sManager.playWrong();
          }
          break;
        case StatusButton::NAME::TARGET3:
          if (controlled->pieces[3]) {
            menu.replaceMenu(Menu::TYPE::TARGET_DESTROY, Menu::TYPE::ROBOT,
                             StatusButton::NAME::ROBOT2);
            controlled->program = Robot::PROGRAM_DESTROY;
            controlled->program_parameter.param = Robot::P_PARAM_WARBASES;
            controlled->program_goal = Vector(-1, -1, -1);
            sManager.playSelect();
          } else {
            /* The robot has no NUCLEAR weapons: */
            sManager.playWrong();
          }
          break;
        }
      }
    }
    break;

  case Menu::TYPE::TARGET_CAPTURE:
    /* Browsing through the SELECT TARGET FOR CAPTURING MENU: */
    {
      if (menu.handleKeys(keyboard)) {
        switch(menu.act_button) {
        case StatusButton::NAME::TARGET1:
          menu.replaceMenu(Menu::TYPE::TARGET_CAPTURE, Menu::TYPE::ROBOT,
                           StatusButton::NAME::ROBOT2);
          controlled->program = Robot::PROGRAM_CAPTURE;
          controlled->program_parameter.param = Robot::P_PARAM_NFACTORIES;
          controlled->program_goal = Vector(-1, -1, -1);
          sManager.playSelect();
          break;
        case StatusButton::NAME::TARGET2:
          menu.replaceMenu(Menu::TYPE::TARGET_CAPTURE, Menu::TYPE::ROBOT,
                           StatusButton::NAME::ROBOT2);
          controlled->program = Robot::PROGRAM_CAPTURE;
          controlled->program_parameter.param = Robot::P_PARAM_EFACTORIES;
          controlled->program_goal = Vector(-1, -1, -1);
          sManager.playSelect();
          break;
        case StatusButton::NAME::TARGET3:
          menu.replaceMenu(Menu::TYPE::TARGET_CAPTURE, Menu::TYPE::ROBOT,
                           StatusButton::NAME::ROBOT2);
          controlled->program = Robot::PROGRAM_CAPTURE;
          controlled->program_parameter.param = Robot::P_PARAM_WARBASES;
          controlled->program_goal = Vector(-1,-1,-1);
          sManager.playSelect();
          break;
        }
      }
    }
    break;
  }

  viewp.x = ship->pos.x + 0.5;
  viewp.y = ship->pos.y + 0.5;
  viewp.z = std::max(0.0f, ship->pos.z - 3 * camera.zoom);
  if (viewp.x < 3 * camera.zoom) {
    viewp.x = 3 * camera.zoom;
    if (viewp.x > map.width() - 3 * camera.zoom)
      viewp.x = map.width() / 2;
  } else {
    if (viewp.x > map.width() - 3 * camera.zoom) {
      viewp.x = map.width() - 3 * camera.zoom;
      if (viewp.x < 3 * camera.zoom)
        viewp.x = map.width() / 2;
    }
  }
  if (viewp.y < 3 * camera.zoom) {
    viewp.y = 3 * camera.zoom;
    if (viewp.y > map.height() - 3 * camera.zoom)
      viewp.y = map.height() / 2;
  } else {
    if (viewp.y > map.height() - 3 * camera.zoom) {
      viewp.y = map.height() - 3 * camera.zoom;
      if (viewp.y < 3 * camera.zoom)
        viewp.y = map.height() / 2;
    }
  }

  if (stats.tick(level)) {
    menu.updateTime(stats);
  }

  /* Test if the ship has landed over a Factory: */
  {
    for (const Building& b: map.buildings) {
      if (b.type == Building::TYPE::WARBASE && b.owner == 1 &&
          ship->pos.x == b.pos.x && ship->pos.y == b.pos.y && ship->landed) {
        constructionScreen.open(b);
      }
    }
  }

  /* Test if the ship has landed over a robot: */
  if (menu.act_menu == Menu::TYPE::GENERAL &&
      (int(ship->pos.x * 8) % 4) == 0 &&
      (int(ship->pos.y * 8) % 4) == 0) {
    for (Robot* r: map.robots[0]) {
      if (ship->pos.x == (r->pos.x - 0.5) && ship->pos.y == (r->pos.y - 0.5) && ship->landed) {
        /* The ship has landed over a robot: */
        r->shipover = true;
        controlled = r;
        if (controlled->op == Robot::OPERATOR::FORWARD)
          controlled->op = Robot::OPERATOR::NONE;
        if (controlled->program == Robot::PROGRAM_FORWARD)
          controlled->program=Robot::PROGRAM_STOPDEFEND;
        menu.replaceMenu(Menu::TYPE::GENERAL, Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT4);
      }
    }
  }

  map.cycle(keyboard);
  menu.cycle();

  if (gameState == NETHER::STATE::PLAYING && keyboard[pause_key] > 1) {
    optionsScreen.open();
  }

  if (stats.noWarbasesLeft() && gameFinished == 0) {
    gameFinished++;
    gameStarted = 0;
  }
  if (gameFinished > 0) gameFinished++;
  if (gameFinished >= END_TIME) return false;

  if (gameStarted > 0) gameStarted--;

  return true;
}
