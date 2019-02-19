#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
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
#include "building.h"
#include "explosion.h"
#include "nether.h"
#include "menu.h"
#include "utils.h"

#include "glprintf.h"

extern int frames_per_sec;
extern bool fullscreen;
extern int shadows;
extern bool sound;
extern int up_key, down_key, left_key, right_key, fire_key, pause_key;
extern int level;
extern float MINY, MAXY, MINX, MAXX;
extern bool show_radar;


NETHER::NETHER(const std::string& mapname): menu(this), radar(this), optionsScreen(this),
                                            constructionScreen(this), ai(this, &map),
                                            camera(0, 0, 0, 0), controlled(NULL)
{
  if (shadows == 1) {
    light.set(-1000, -3000, 5000, 1);
  } else {
    light.set(0, 0, 5000, 1);
  }

  Resources::instance()->loadObjects();

  ship = new Ship("models/ship.asc", "textures/");
  ship->ComputeShadow(light.asVector());

  map.loadMap(mapname);

  viewp.x = map.width() / 2;
  viewp.y = 0;
  camera.x = 6;
  camera.y = -6;
  camera.z = 11;
  camera.zoom = 1;

  game_state = NETHER::STATE::PLAYING;
  animation_timer = 0;
  controlled = 0;
  game_finished = 0;
  game_started = INTRO_TIME;

  menu.newmenu(Menu::TYPE::GENERAL);
  menu.needsRedraw = 2;
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
  unsigned char* keyboard = SDL_GetKeyState(NULL);

  switch(game_state) {
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
    old_keyboard[i] = keyboard[i];

  return retval;
}


void NETHER::gameredraw(int w,int h)
{
  switch(game_state) {
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

  if (game_started > 0) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, ratio, 1.0, 1024.0);
    gluLookAt(0,0, 30, 0, 0, 0, 0, 1, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (game_started > 40) glTranslatef(0, 0, (game_started - 40) * 2);
    if (game_started < 20) glTranslatef(0, 0, (20 - game_started) * 2);
    Resources::messageTiles[0].draw(Color(1.0, 1.0, 1.0));
  }

  if (game_finished > 100) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, ratio, 1.0, 1024.0);
    gluLookAt(0, 0, 30, 0, 0, 0, 0, 1,0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (game_finished < 120) glTranslatef(0, 0, (120 - game_finished) * 2);
    if (game_finished > 240) glTranslatef(0, 0, (game_finished - 240) * 2);
    if (stats.stats[0][0] == 0) Resources::messageTiles[2].draw(Color(1.0, 1.0, 1.0));
    else Resources::messageTiles[1].draw(Color(1.0, 1.0, 1.0));
  }

  radar.draw(width, height, split, splity);
  menu.draw(width, height);
}


void NETHER::drawGame(bool shadows)
{
  MINY =- 8 * camera.zoom;
  MINX =- (10 + viewp.z * 4) * camera.zoom;
  MAXY = (9 + viewp.z * 4) * camera.zoom;
  MAXX = 8 * camera.zoom;

  Vector newLight(light.asVector());
  newLight = newLight / newLight.z;
  map.draw(viewp, shadows, newLight, camera);
  ship->draw(shadows, newLight, map, controlled);
}


bool NETHER::saveGame(const std::string& filename)
{
  std::ofstream oFile(filename);

  oFile << map;

  oFile << light
        << camera
        << viewp
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
  for (Bullet& bullet: map.bullets) {
    oFile << std::make_pair(bullet, map.robots);
  }

  oFile << map.explosions.size() << '\n';
  for (Explosion& e: map.explosions) {
    oFile << e;
  }

  oFile << stats;

  oFile << find_index(map.robots[0], controlled) << '\n';
  oFile << int(menu.act_menu) << ' ' << int(menu.act_button) << std::endl;

  return true;
}


bool NETHER::loadGame(const std::string& filename)
{
  std::ifstream inFile(filename);

  ai.deletePrecomputations();

  inFile >> map
         >> light
         >> camera
         >> viewp
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
    map.bullets.emplace_back(inFile, map.robots);
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
  menu.act_button = StatusButton::NAME(actButton_);

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
  log << "VIEWP: " << viewp;
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
  for (Bullet& bullet: map.bullets) {
    log << " BULLET:\n TYPE: " << int(bullet.type)
        << "\n STEP: " << bullet.step
        << "\n ANGLE: " << bullet.angle << '\n';
    log << " POSITION: ";
    log << bullet.pos;

    if (std::count(map.robots[0].cbegin(), map.robots[0].cend(), bullet.owner)) {
      log << " OWNER: PLAYER 0 ROBOT " << bullet.owner->getId() << '\n';
    } else if (std::count(map.robots[1].cbegin(), map.robots[1].cend(), bullet.owner)) {
      log << " OWNER: PLAYER 1 ROBOT " << bullet.owner->getId() << '\n';
    }

    log << " MINIMUM CONTAINER BOX: \n";
    log << bullet.cmc << '\n';
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
  log << "\nMENU " << int(menu.act_menu) << "\nACT BUTTON: " << int(menu.act_button) << '\n';

  return true;
}
