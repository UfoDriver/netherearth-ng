#ifdef _WIN32
#include "windows.h"
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

#include "3dobject.h"
#include "building.h"
#include "cmc.h"
#include "explosion.h"
#include "menu.h"
#include "myglutaux.h"
#include "nether.h"
#include "piece3dobject.h"
#include "shadow3dobject.h"
#include "utils.h"
#include "vector.h"


extern int frames_per_sec;
extern bool fullscreen;
extern int shadows;
extern bool sound;
extern int up_key, down_key, left_key, right_key, fire_key, pause_key;
extern int level;
extern bool show_radar;


NETHER::NETHER(const std::string& mapname): map(this), ai(this, &map), menu(this), radar(this),
                                            optionsScreen(this), constructionScreen(this),
                                            camera(0, 0, 0, 0), controlled(nullptr)
{
  if (shadows == 1) {
    light.set(-1000, -3000, 5000, 1);
  } else {
    light.set(0, 0, 5000, 1);
  }

  Resources::instance()->loadObjects();

  ship = new Ship("models/ship.asc", "textures/");
  ship->computeShadow(light.asVector());

  map.loadMap(mapname);

  camera.x = 6;
  camera.y = -6;
  camera.z = 11;
  camera.zoom = 1;
  camera.viewport.x = map.width() / 2;
  camera.viewport.y = 0;

  gameState = NETHER::STATE::PLAYING;
  animationTimer = 0;
  controlled = 0;
  gameFinished = 0;
  gameStarted = INTRO_TIME;

  menu.newmenu(Menu::TYPE::GENERAL);
  menu.requestRedraw();
  radar.needsRedraw = 1;

  ai.makePrecomputations();
}


NETHER::~NETHER()
{
  map.resize(0, 0);
  Resources::instance()->deleteObjects();
  ai.deletePrecomputations();
  delete ship;
  ship = 0;
}


bool NETHER::gamecycle()
{
  bool retval = true;

  SDL_PumpEvents();
  unsigned char* sdlKeyboard = SDL_GetKeyState(NULL);
  unsigned char keyboard[SDLK_LAST];

  for (int i = 0; i < SDLK_LAST; i++) {
    keyboard[i] = sdlKeyboard[i];
    if (sdlKeyboard[i] && !prevKeyboard[i]) {
      keyboard[i] |= 2;
    }
  }

  switch(gameState) {
  case NETHER::STATE::PLAYING:
    retval = cycle(keyboard);
    break;
  case NETHER::STATE::CONSTRUCTION:
    retval = constructionScreen.cycle(keyboard);
    break;
  case NETHER::STATE::PAUSE:
  case NETHER::STATE::SAVINGGAME:
  case NETHER::STATE::LOADINGGAME:
    retval = optionsScreen.cycle(keyboard);
    break;
  }

  for (int i = 0; i < SDLK_LAST; i++)
    prevKeyboard[i] = sdlKeyboard[i];

  return retval;
}


void NETHER::gameredraw(int w,int h)
{
  switch(gameState) {
  case NETHER::STATE::PLAYING:
    draw(w, h);
    break;
  case NETHER::STATE::CONSTRUCTION:
    constructionScreen.draw(w, h, light);
    break;
  case NETHER::STATE::PAUSE:
  case NETHER::STATE::SAVINGGAME:
  case NETHER::STATE::LOADINGGAME:
    draw(w, h);
    optionsScreen.draw(w, h, light);
    break;
  }

  SDL_GL_SwapBuffers();
}


void NETHER::draw(int width, int height)
{
  float tmpls[4] = {1.0F, 1.0F, 1.0F, 1.0};
  float tmpld[4] = {0.6F, 0.6F, 0.6F, 1.0};
  float tmpla[4] = {0.2F, 0.2F, 0.2F, 1.0};
  int split = int((width * 25.0F) / 32.0F);
  int splity = 0;

  if (show_radar)
    splity = int((height * 2.0F) / 15.0F) + 1;
  else
    splity = 0;

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
  glDepthFunc(GL_LEQUAL);
  glClearStencil(0);

  /* Draw the GAME screen: */
  glLightfv(GL_LIGHT0, GL_POSITION, light.raw());
  glClearColor(0, 0, 0, 0.0);
  glViewport(0, splity, split, height - splity);
  float ratio = float(split) / float(height - splity);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, ratio, 1.0, 1024.0);
  glScissor(0, splity, split, height - splity);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  drawGame(false);
  if (shadows) {
    /* Set STENCIL Buffer: */
    glStencilMask(1);
    glEnable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    drawGame(true);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    /* Draw shadow poligon: */
    glDepthFunc(GL_ALWAYS);
    glDisable(GL_CULL_FACE);

    glColor4f(0.0, 0.0, 0.0, 0.4f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glStencilFunc(GL_NOTEQUAL, 0, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-1.0, 1.0, 0.0);
    glVertex3f(-1.0, -1.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    glVertex3f(1.0, -1.0, 0.0);
    glEnd();
    glPopMatrix();

    glDisable(GL_BLEND);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
  }

  if (gameStarted > 0) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, ratio, 1.0, 1024.0);
    gluLookAt(0,0, 30, 0, 0, 0, 0, 1, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (gameStarted > 40) glTranslatef(0, 0, (gameStarted - 40) * 2);
    if (gameStarted < 20) glTranslatef(0, 0, (20 - gameStarted) * 2);
    Resources::messageTiles[0].draw(Color(1.0, 1.0, 1.0));
  }

  if (gameFinished > 100) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, ratio, 1.0, 1024.0);
    gluLookAt(0, 0, 30, 0, 0, 0, 0, 1,0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (gameFinished < 120) glTranslatef(0, 0, (120 - gameFinished) * 2);
    if (gameFinished > 240) glTranslatef(0, 0, (gameFinished - 240) * 2);
    if (stats.stats[0][0] == 0) Resources::messageTiles[2].draw(Color(1.0, 1.0, 1.0));
    else Resources::messageTiles[1].draw(Color(1.0, 1.0, 1.0));
  }

  radar.draw(width, height, split, splity);
  menu.draw(width, height);
}


void NETHER::drawGame(bool shadows)
{
  Vector newLight(light.asVector());
  newLight = newLight / newLight.z;
  map.draw(camera, newLight, shadows);
  camera.drawViewport();
}


bool NETHER::saveGame(const std::string& filename)
{
  std::ofstream oFile(filename);

  oFile << map;

  oFile << light
        << camera
        << camera.viewport
        << *ship;

  oFile << map.buildings.size() << '\n';
  for (Building& b: map.buildings) {
    oFile << b;
  }

  for (int i = 0; i < 2; i++) {
    oFile << map.robots[i].size() << '\n';
    for (Robot* r: map.robots[i]) {
      oFile << *r;
    }
  }

  oFile << map.bullets.size() << '\n';
  for (const std::unique_ptr<Bullet>& bullet: map.bullets) {
    std::pair<int, int> pair = findRobotIndexAndOwner(bullet->owner);

    oFile << (int)bullet->type << ' ' << bullet->step << ' ' << bullet->angle << '\n';
    oFile << bullet->pos;
    oFile << pair.first << ' ' << pair.second;
    oFile << bullet->cmc;
  }

  oFile << map.explosions.size() << '\n';
  for (Explosion& e: map.explosions) {
    oFile << e;
  }

  oFile << stats;

  oFile << find_index(map.robots[0], controlled) << '\n';
  oFile << int(menu.act_menu) << ' ' << int(menu.getActiveButton()) << std::endl;

  return true;
}


bool NETHER::loadGame(const std::string& filename)
{
  menu.killmenu(menu.act_menu);
  std::ifstream inFile(filename);

  ai.deletePrecomputations();

  inFile >> map
         >> light
         >> camera
         >> camera.viewport
         >> *ship;

  int length;
  inFile >> length;
  for (int k = 0; k < length; k++) {
    map.buildings.push_back(Building(inFile));
  }

  for (int i = 0; i < 2; i++) {
    inFile >> length;
    for (int k = 0; k < length; k++) {
      map.robots[i].push_back(new Robot(inFile));
    }
  }

  inFile >> length;
  for (int k = 0; k < length; k++) {
    map.bullets.emplace_back(Bullet::read(inFile, map.robots));
  }

  inFile >> length;
  for (int k = 0; k < length; k++) {
    map.explosions.emplace_back(inFile);
  }

  inFile >> stats;

  int i;
  inFile >> i;
  if (i >= 0)
    controlled = map.robots[0][i];
  else
    controlled = 0;

  int actMenu_, actButton_;
  inFile >> actMenu_ >> actButton_;
  menu.act_menu = Menu::TYPE(actMenu_);
  menu.setActiveButton(StatusButton::NAME(actButton_));

  ai.makePrecomputations();
  menu.newmenu(menu.act_menu);
  return true;
}


bool NETHER::saveDebugReport(const std::string& filename)
{
  std::ofstream log(filename);
  log << "NETHER EARTH NG Debug Report\n\n";
  log << "MAPW: " << map.width() << "\nMAPH: " << map.height() << '\n';
  log << "MAP:\n";
  for (int i = 0; i < map.height(); i++) {
    for (int j = 0; j < map.width(); j++) {
      log << map.map[j + i * map.width()] << ' ';
    }
    log << '\n';
  }

  log << "LIGHTPOS: " << light.raw()[0] << ' '
      << light.raw()[1] << ' '
      << light.raw()[2] << ' '
      << light.raw()[3] << '\n';
  log << "LIGHTPOSV: " << light.asVector();
  log << "CAMERA: " << camera;
  log << "VIEWP: " << camera.viewport;
  log << "SHIPP: " << ship->pos;
  if (ship->landed)
    log << "SHIP LANDED\n";
  else
    log << "SHIP NOT LANDED\n";

  log << "# OF BUILDINGS: " << map.buildings.size() << '\n';
  for (const Building& b: map.buildings) {
    log << "BUILDING:\n"
        << " TYPE: " << int(b.type)
        << "\n OWNER: " << b.owner
        << "\n STATUS: " << b.status << "\n"
        << b.pos;
  }

  for (int i = 0; i < 2; i++) {
    log << "\n# OF ROBOTS PLAYER " << i << ": " << map.robots[i].size() << '\n';

    const char* tractions[3] = {"BIPOD", "TRACKS", "ANTIGRAV"};
    const char* pieces[5] = {"CANNONS", "MISSILES", "PHASERS", "NUCLEAR", "ELECTRONICS"};
    for (Robot* r: map.robots[i]) {
      log << "ROBOT:\n";
      log << ' ' << tractions[r->traction] << '\n';
      for (int j = 0; j < 5; j++) {
        if (r->pieces[j])
          log << ' ' << pieces[j] << '\n';
      }
      log << " PROGRAM: " << r->program << '\n';
      log << " PROGRAM PARAMETER: " << r->program_parameter.as_int << '\n';
      log << " PROGRAM GOAL: ";
      log << r->program_goal;
      log << " ACTUAL OPERATOR: " << int(r->op) << '\n';
      if (r->shipover)
        log << " HAS THE SHIP OVER IT\n";
      else
        log << " HAS NO SHIP OVER IT\n";
      log << " FIRETIMER: " << r->firetimer << "\n STRENGTH: " << r->strength << '\n';
      log << " POSITION: ";
      log << r->pos;
      log << " ANGLE: " << r->angle << '\n';
      log << " MINIMUM CONTAINER BOX:\n";
      log << r->cmc;
      log << " ELECTRONICS STATE: " << r->electronics_state
          <<"\n CHASSIS STATE: " << r->chassis_state;
      log << "\n\n";
    }
  }

  log << "\n# BULLETS: " << map.bullets.size() << '\n';
  for (auto& bullet: map.bullets) {
    log << " BULLET:\n TYPE: " << int(bullet->type)
        << "\n STEP: " << bullet->step
        << "\n ANGLE: " << bullet->angle << '\n';
    log << " POSITION: ";
    log << bullet->pos;

    if (std::count(map.robots[0].cbegin(), map.robots[0].cend(), bullet->owner)) {
      log << " OWNER: PLAYER 0 ROBOT " << bullet->owner->getId() << '\n';
    } else if (std::count(map.robots[1].cbegin(), map.robots[1].cend(), bullet->owner)) {
      log << " OWNER: PLAYER 1 ROBOT " << bullet->owner->getId() << '\n';
    }

    log << " MINIMUM CONTAINER BOX: \n";
    log << bullet->cmc << '\n';
  }

  log << "# EXPLOSIONS " << map.explosions.size() << '\n';
  for (Explosion& e: map.explosions) {
    log << "EXPLOSION:\n POSITION:\n";
    log << e.pos;
    log << " STEP: " << e.step << "\n SIZE: " << e.size << "\n\n";
  }

  log << "\nTIME: DAY " << stats.day << ' ' << stats.hour << ':' << stats.minute << ':' << stats.second << '\n';
  log << "\nRESOURCES:\n";

  for(int i = 0; i < 2; i++) {
    log << "  PLAYER " << i << ": ";
    for(int j = 0; j < 7; j++) {
      log << stats.resources[i][j] << ' ';
    }
    log << '\n';
  }

  log << "\nROBOT UNDER CONTROL: ";
  if (controlled)
    log << controlled->getId() << '\n';
  else
    log << "None\n";
  log << "\nMENU " << int(menu.act_menu) << "\nACT BUTTON: " << int(menu.getActiveButton()) << '\n';

  return true;
}


std::pair<int, int> NETHER::getRobotsCount() const
{
  return std::make_pair(map.robots[0].size(), map.robots[1].size());
}


std::array<std::pair<int, int>, 7> NETHER::getBuildingStats() const
{
  return stats.getBuildingStats();
}


std::array<std::pair<int, int>, 7> NETHER::getResourceStats() const
{
  return stats.getResourceStats();
}


void NETHER::addNewRobot(Robot* robot, int player)
{
  map.robots[player].push_back(robot);
  ai.newRobot(robot->pos, player);
}


void NETHER::detachShip(Robot* robot)
{
  if (robot == controlled) {
    controlled->shipover = false;
    controlled = nullptr;
    menu.killmenu(Menu::TYPE::ALL);
    menu.newmenu(Menu::TYPE::GENERAL);
  }
}


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

      if (ship->op == Ship::OPS::RIGHT && ship->pos.x < map.width() - 1) {
        ship->pos.x += 0.125;
        if (ship->timemoving >= 50 && (int(ship->pos.x * 8) % 2) == 1)
          ship->pos.x += 0.125;
      }
      if (ship->op == Ship::OPS::LEFT && ship->pos.x > 0) {
        ship->pos.x -= 0.125;
        if (ship->timemoving >= 50 && (int(ship->pos.x * 8) % 2) == 1)
          ship->pos.x -= 0.125;
      }
      if (ship->op2 == Ship::OPS::FORWARD && ship->pos.y < map.height() - 1) {
        ship->pos.y += 0.125;
        if (ship->timemoving >= 50 && (int(ship->pos.y * 8) % 2) == 1)
          ship->pos.y += 0.125;
      }
      if (ship->op2==Ship::OPS::BACKWARD && ship->pos.y > 0) {
        ship->pos.y -= 0.125;
        if (ship->timemoving >= 50 && (int(ship->pos.y * 8) % 2) == 1)
          ship->pos.y -= 0.125;
      }

      if (ship->op3 == Ship::OPS::UP && ship->pos.z < 5.0)
        ship->pos.z += 0.05;
      if (ship->op3 != Ship::OPS::UP && ship->pos.z > minz)
        ship->pos.z -= 0.025;
      ship->pos.z = std::max(ship->pos.z, minz);

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
        switch (menu.getActiveButton()) {
        case StatusButton::NAME::ROBOT1:
          {
            menu.setActiveButtonColor({1.0f, 0.5f, 0.5f});
            menu.act_menu = Menu::TYPE::DIRECTCONTROL;
            menu.requestRedraw();
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
      menu.requestRedraw();
      menu.act_menu=Menu::TYPE::ROBOT;
    }
    break;

  case Menu::TYPE::DIRECTCONTROL2:
    /* Direct control of a robot by the user: */
    if (keyboard[fire_key] > 1) {
      menu.requestRedraw();
      menu.act_menu=Menu::TYPE::COMBATMODE;
    }
    break;

  case Menu::TYPE::COMBATMODE:
    /* Browsing through the COMBAT MENU: */
    {
      if (menu.handleKeys(keyboard)) {
        switch(menu.getActiveButton()) {
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
            menu.setActiveButtonColor({1.0f, 0.5f, 0.5f});
            menu.act_menu = Menu::TYPE::DIRECTCONTROL2;
            menu.requestRedraw();
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
        switch (menu.getActiveButton()) {
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
        menu.requestRedraw();
      }
      if (keyboard[down_key] > 1) {
        controlled->program_parameter.as_int -= 10;
        if (controlled->program_parameter.as_int < 0)
          controlled->program_parameter.as_int = 0;
        controlled->program_goal = Vector(-1, -1, -1);

        menu.requestRedraw();
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
        switch(menu.getActiveButton()) {
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
        switch(menu.getActiveButton()) {
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

  camera.updateViewportForShip(ship->pos, map.width(), map.height());

  if (stats.tick(level)) {
    menu.updateTime(stats);
  }

  /* Test if the ship has landed over a Factory: */
  {
    for (const Building& b: map.buildings) {
      if (b.type == Building::TYPE::WARBASE && b.owner == 1 && ship->landedHere(b.pos)) {
        constructionScreen.open(b);
      }
    }
  }

  /* Test if the ship has landed over a robot: */
  if (menu.act_menu == Menu::TYPE::GENERAL &&
      (int(ship->pos.x * 8) % 4) == 0 &&
      (int(ship->pos.y * 8) % 4) == 0) {
    for (Robot* r: map.robots[0]) {
      if (ship->landedHere(r->pos - Vector(0.5f, 0.5f, 0.0))) {
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


std::pair<int, int> NETHER::findRobotIndexAndOwner(const Robot* robot)
{
  for (int owner = 0; owner < 2; owner++) {
    int index = find_index(map.robots[owner], robot);
    if (index != -1) {
      return std::make_pair(owner, index);
    }
  }
  return std::make_pair(-1, -1);
}
