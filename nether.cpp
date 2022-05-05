#include "soundmanager.h"
#include <bitset>
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

#include <sexp/io.hpp>
#include <sexp/parser.hpp>
#include <sexp/util.hpp>

#include "3dobject.h"
#include "buildingblock.h"
#include "cmc.h"
#include "explosion.h"
#include "menu.h"
#include "myglutaux.h"
#include "nether.h"
#include "piece3dobject.h"
#include "shadow3dobject.h"


extern int shadows;
extern int up_key, down_key, left_key, right_key, fire_key, pause_key;


NETHER::NETHER(const std::string& mapname, Config& config)
  : scene{this, mapname}, config{config}, ai{&scene, &stats, config.level}, menu{*this}, radar{this},
    optionsScreen{this}, constructionScreen{this},
    camera{0, 0, 0, 0}, controlled{nullptr}, sManager{config.sound}
{
  if (shadows == 1) {
    light.set(-1000, -3000, 5000, 1);
  } else {
    light.set(0, 0, 5000, 1);
  }

  Resources::instance()->loadObjects();

  scene.map.loadMap(mapname);

  camera.x = 6;
  camera.y = -6;
  camera.z = 11;
  camera.zoom = 1;
  camera.viewport.x = scene.map.getWidth() / 2;
  camera.viewport.y = 0;

  gameState = NETHER::STATE::PLAYING;
  animationTimer = 0;
  controlled = 0;
  gameFinished = 0;
  gameStarted = INTRO_TIME;

  menu.activateMenu(Menu::TYPE::GENERAL, StatusButton::NAME::NONE);

  ai.makePrecomputations();
}


NETHER::~NETHER()
{
  ai.deletePrecomputations();
  scene.map.resize(0, 0);
  Resources::instance()->deleteObjects();
}


bool NETHER::gamecycle()
{
  bool retval = true;

  SDL_PumpEvents();
  SDLMod modifiers = SDL_GetModState();
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
    retval = cycle(keyboard) && scene.cycle(keyboard);
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

  if (config.showRadar)
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

  if (config.showRadar) {
    radar.draw(width, height, split, splity);
  }
  menu.draw(width, height);
}


void NETHER::drawGame(bool shadows)
{
  Vector newLight(light.asVector());
  newLight = newLight / newLight.z;
  scene.draw(camera, newLight, shadows);
  camera.drawViewport();
}


bool NETHER::saveGame(const std::string& filename)
{
  sexp::Value robotList = sexp::Value::nil();
  for (std::shared_ptr<Robot> r: scene.robots) {
    robotList = sexp::Value::cons(r->toSexp(), std::move(robotList));
  }

  sexp::Value explosionList = sexp::Value::nil();
  for (Explosion& e: scene.explosions) {
    explosionList = sexp::Value::cons(e.toSexp(), std::move(explosionList));
  }

  sexp::Value bulletList = sexp::Value::nil();
  for (const std::shared_ptr<Bullet>& bullet: scene.bullets) {
    bulletList = sexp::Value::cons(bullet->toSexp(), std::move(bulletList));
  }

  sexp::Value buildingList = sexp::Value::nil();
  for (auto& b: scene.map.buildings) {
    buildingList = sexp::Value::cons(b->toSexp(), std::move(buildingList));
  }

  sexp::Value gamestate = sexp::Value::list(
    sexp::Value::symbol("gamestate"),
    scene.map.toSexp(),
    light.toSexp(),
    camera.toSexp(),
    scene.ship.toSexp(),
    sexp::Value::cons(
      sexp::Value::symbol("buildings"),
      std::move(buildingList)
    ),
    sexp::Value::cons(
      sexp::Value::symbol("robots"),
      std::move(robotList)
    ),
    sexp::Value::cons(
      sexp::Value::symbol("bullets"),
      std::move(bulletList)
    ),
    sexp::Value::cons(
      sexp::Value::symbol("explosions"),
      std::move(explosionList)
    ),
    stats.toSexp(),
    sexp::Value::list(
      sexp::Value::symbol("controlled-robot"),
      sexp::Value::integer(scene.robots.findIndex(controlled))
    ),
    menu.toSexp()
  );

  std::ofstream(filename, std::ios::binary) << gamestate;
  return true;
}


bool NETHER::loadGame(const std::string& filename)
{
  ai.deletePrecomputations();

  std::ifstream inFile(filename, std::ios::binary);
  sexp::Value gamestate = sexp::Parser::from_stream(inFile);

  if (gamestate.get_car().as_string() != "gamestate1") {
    std::cerr << "Not a gamestate" << std::endl;
    return false;
  }

  // @TODO: loading order of sexps matters but code doesn't reflect this. That means in some sexp
  // structure, it's possible to load bullets without having robots
  for (const sexp::Value& value: sexp::ListAdapter(sexp::cdr(gamestate))) {
    if (sexp::car(value).as_string() == "map") {
      scene.map.fromSexp(value);
    } else if (sexp::car(value).as_string() == "light") {
      light.fromSexp(value);
    } else if (sexp::car(value).as_string() == "camera") {
      camera.fromSexp(value);
    } else if (sexp::car(value).as_string() == "ship") {
      scene.ship.fromSexp(value);
    } else if (sexp::car(value).as_string() == "buildings") {
      scene.map.buildings.clear();
      for (const sexp::Value& building: sexp::ListAdapter(sexp::cdr(value))) {
        scene.map.buildings.emplace_back(Building::fromSexp(building));
      }
    } else if (sexp::car(value).as_string() == "robots") {
      scene.robots.clear();
      for (const sexp::Value& robot: sexp::ListAdapter(sexp::cdr(value))) {
        scene.robots.emplace_back(new Robot(robot));
      }
    } else if (sexp::car(value).as_string() == "bullets") {
      scene.bullets.clear();
      for (const sexp::Value& bullet: sexp::ListAdapter(sexp::cdr(value))) {
        scene.bullets.emplace_back(Bullet::fromSexp(bullet));
      }
    } else if (sexp::car(value).as_string() == "explosions") {
      scene.explosions.clear();
      for (const sexp::Value& explosion: sexp::ListAdapter(sexp::cdr(value))) {
        scene.explosions.emplace_back(explosion);
      }
    } else if (sexp::car(value).as_string() == "stats") {
      stats.fromSexp(value);
    } else if (sexp::car(value).as_string() == "controlled-robot") {
      int controlledRobot = sexp::cdar(value).as_int();
      if (controlledRobot >= 0)
        controlled = scene.robots[controlledRobot];
      else
        controlled = 0;
    } else if (sexp::car(value).as_string() == "menu") {
      menu.fromSexp(value);
    }
  }

  ai.makePrecomputations();
  return true;
}


std::array<std::pair<int, int>, 7> NETHER::getBuildingStats() const
{
  return stats.getBuildingStats();
}


std::array<std::pair<int, int>, 7> NETHER::getResourceStats() const
{
  return stats.getResourceStats();
}


void NETHER::addNewRobot(std::shared_ptr<Robot> robot, int player)
{
  scene.robots.push_back(robot);
  ai.newRobot(robot->pos, player);
}


void NETHER::detachShip(std::shared_ptr<Robot> robot)
{
  if (robot == controlled) {
    controlled->detachShip();
    controlled = nullptr;
    menu.activateMenu(Menu::TYPE::GENERAL, StatusButton::NAME::NONE);
  }
}


bool NETHER::cycle(unsigned char *keyboard)
{
  camera.cycle(keyboard);
  stats.recompute(scene.map.buildings);

  /* ENEMY Artificial Intelligence: */
  if (stats.second == 0) {
    if (config.level == 0 && (stats.hour & 0x01) == 0 && stats.minute == 0) ai.enemy();
    if (config.level == 1 && stats.minute == 0) ai.enemy();
    if (config.level >= 2 && (stats.minute == 0 || stats.minute == 30)) ai.enemy();
  }

  /* GAME Cycle: */
  scene.ship.landed = false;
  if (menu.getActiveMenu() == Menu::TYPE::GENERAL)
    scene.ship.cycle(keyboard);
  menu.cycle(keyboard);
  camera.updateViewportForShip(scene.ship.pos, scene.map.getWidth(), scene.map.getHeight());
  if (stats.tick(config.level)) {
    menu.updateTime(stats);
  }

  /* Test if the ship has landed over a Factory: */
  for (const auto& b: scene.map.buildings) {
    if (b->type == Building::TYPE::WARBASE && b->owner == 1 and
        scene.ship.landedHere(b->pos)) {
      constructionScreen.open(b->pos);
    }
  }

  /* Test if the ship has landed over a robot: */
  if (menu.getActiveMenu() == Menu::TYPE::GENERAL &&
      (int(scene.ship.pos.x * 8) % 4) == 0 &&
      (int(scene.ship.pos.y * 8) % 4) == 0) {
    for (std::shared_ptr<Robot> r: scene.robots) {
      if (r->getOwner() == 0 and scene.ship.landedHere(r->pos - Vector(0.5f, 0.5f, 0.0))) {
        r->attachShip();
        controlled = r;
        if (controlled->op == Robot::OPERATOR::FORWARD)
          controlled->op = Robot::OPERATOR::NONE;
        if (controlled->program.type == RobotProgram::FORWARD)
          controlled->program.type=RobotProgram::STOPDEFEND;
        menu.activateMenu(Menu::TYPE::ROBOT, StatusButton::NAME::ROBOT4);
      }
    }
  }

  scene.cycle(keyboard);

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
