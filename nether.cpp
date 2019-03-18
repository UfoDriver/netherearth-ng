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
#include <numeric>
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

  ship = new Ship("models/ship.asc", "textures/", this);
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

  menu.activateMenu(Menu::TYPE::GENERAL, StatusButton::NAME::NONE);
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


void NETHER::gameredraw(int w, int h)
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
  float lightSpecular[4] = {1.0F, 1.0F, 1.0F, 1.0};
  float lightDiffuse[4] = {0.6F, 0.6F, 0.6F, 1.0};
  float lightAmbient[4] = {0.2F, 0.2F, 0.2F, 1.0};
  int split = int((width * 25.0F) / 32.0F);
  int splity = 0;

  if (show_radar)
    splity = int((height * 2.0F) / 15.0F) + 1;
  else
    splity = 0;

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

  std::pair<int, int> robotsCount = getRobotsCount();
  oFile << robotsCount.first << '\n';
  for (Robot* r: map.robots) {
    if (r->getOwner() == 0)
      oFile << *r;
  }
  oFile << robotsCount.second << '\n';
  for (Robot* r: map.robots) {
    if (r->getOwner() == 1)
      oFile << *r;
  }


  oFile << map.bullets.size() << '\n';
  for (const std::unique_ptr<Bullet>& bullet: map.bullets) {
    int index = findRobotIndex(bullet->owner);

    oFile << (int)bullet->type << ' ' << bullet->step << ' ' << bullet->angle << '\n';
    oFile << bullet->pos;
    oFile << map.robots[index]->getOwner() << ' ' << index;
    oFile << bullet->cmc;
  }

  oFile << map.explosions.size() << '\n';
  for (Explosion& e: map.explosions) {
    oFile << e;
  }

  oFile << stats;

  oFile << find_index(map.robots, controlled) << '\n';
  oFile << menu << std::endl;

  return true;
}


bool NETHER::loadGame(const std::string& filename)
{
  menu.activateMenu(Menu::TYPE::GENERAL, StatusButton::NAME::NONE);
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
      map.robots.push_back(new Robot(i, inFile));
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
    controlled = map.robots[i];
  else
    controlled = 0;

  inFile >> menu;

  ai.makePrecomputations();
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

  std::pair<int, int> counters = getRobotsCount();
  for (int i = 0; i < 2; i++) {
    if (i == 0) {
      log << "\n# OF ROBOTS PLAYER " << i << ": " << counters.first << '\n';
    } else {
      log << "\n# OF ROBOTS PLAYER " << i << ": " << counters.second << '\n';
    }

    const char* tractions[3] = {"BIPOD", "TRACKS", "ANTIGRAV"};
    const char* pieces[5] = {"CANNONS", "MISSILES", "PHASERS", "NUCLEAR", "ELECTRONICS"};
    for (Robot* r: map.robots) {
      if (r->getOwner() != i) continue;
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

    log << " OWNER: PLAYER " << bullet->owner->getOwner()
        << " ROBOT " << bullet->owner->getId() << '\n';

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
  log << "\nMENU " << int(menu.getActiveMenu()) << "\nACT BUTTON: " << int(menu.getActiveButton()) << '\n';

  return true;
}


std::pair<int, int> NETHER::getRobotsCount() const
{
  return std::accumulate(map.robots.cbegin(), map.robots.cend(),
                         std::make_pair(0, 0),
                         [](std::pair<int, int>& acc, auto r) {
                           if (r->getOwner() == 0) {
                             acc.first++;
                           } else {
                             acc.second++;
                           }
                           return acc;
                         });
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
  map.robots.push_back(robot);
  ai.newRobot(robot->pos, player);
}


void NETHER::detachShip(Robot* robot)
{
  if (robot == controlled) {
    controlled->electronics_state += 6;
    controlled->shipover = false;
    controlled = nullptr;
    menu.activateMenu(Menu::TYPE::GENERAL, StatusButton::NAME::NONE);
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
  if (menu.getActiveMenu() == Menu::TYPE::GENERAL)
    ship->cycle(keyboard);
  menu.cycle(keyboard);
  camera.updateViewportForShip(ship->pos, map.width(), map.height());
  if (stats.tick(level)) {
    menu.updateTime(stats);
  }

  /* Test if the ship has landed over a Factory: */
  for (const Building& b: map.buildings) {
    if (b.type == Building::TYPE::WARBASE && b.owner == 1 && ship->landedHere(b.pos)) {
      constructionScreen.open(b);
    }
  }

  /* Test if the ship has landed over a robot: */
  if (menu.getActiveMenu() == Menu::TYPE::GENERAL &&
      (int(ship->pos.x * 8) % 4) == 0 &&
      (int(ship->pos.y * 8) % 4) == 0) {
    for (Robot* r: map.robots) {
      if (r->getOwner() == 0 && ship->landedHere(r->pos - Vector(0.5f, 0.5f, 0.0))) {
        r->shipover = true;
        controlled = r;
        if (controlled->op == Robot::OPERATOR::FORWARD)
          controlled->op = Robot::OPERATOR::NONE;
        if (controlled->program == Robot::PROGRAM_FORWARD)
          controlled->program=Robot::PROGRAM_STOPDEFEND;
        menu.activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT4);
      }
    }
  }

  map.cycle(keyboard);

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


int NETHER::findRobotIndex(const Robot* robot)
{
  int index = find_index(map.robots, robot);
  if (index != -1) {
    return index;
  }
  return -1;
}
