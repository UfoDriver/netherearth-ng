#include <GL/gl.h>
#include <GL/glu.h>
#include <numeric>
#include <cmath>

#include "constructionscreen.h"
#include "glprintf.h"
#include "myglutaux.h"
#include "nether.h"

extern int up_key, down_key, left_key, right_key, fire_key;


ConstructionScreen::MENU operator++(ConstructionScreen::MENU &m, int) {
  using IntType = typename std::underlying_type<ConstructionScreen::MENU>::type;
  m = static_cast<ConstructionScreen::MENU>(static_cast<IntType >(m) + 1);
  return m;
}


ConstructionScreen::MENU operator--(ConstructionScreen::MENU &m, int) {
  using IntType = typename std::underlying_type<ConstructionScreen::MENU>::type;
  m = static_cast<ConstructionScreen::MENU>(static_cast<IntType >(m) - 1);
  return m;
}


bool ConstructionScreen::cycle(unsigned char *keyboard)
{
  if (menuPointer == MENU::START && keyboard[right_key] && !nether->old_keyboard[right_key])
    menuPointer = MENU::BIPOD;
  if (menuPointer == MENU::EXIT && keyboard[right_key] && !nether->old_keyboard[right_key])
    menuPointer = MENU::START;
  if (menuPointer == MENU::START && keyboard[left_key] && !nether->old_keyboard[left_key])
    menuPointer = MENU::EXIT;
  if (menuPointer >= MENU::BIPOD && keyboard[left_key] && !nether->old_keyboard[left_key])
    menuPointer = MENU::START;

  if (menuPointer >= MENU::BIPOD && menuPointer < MENU::ELECTRONICS &&
      keyboard[up_key] && !nether->old_keyboard[up_key]) menuPointer++;
  if (menuPointer > MENU::BIPOD && menuPointer <= MENU::ELECTRONICS &&
      keyboard[down_key] && !nether->old_keyboard[down_key]) menuPointer--;

  if (menuPointer >= MENU::BIPOD && keyboard[fire_key] && !nether->old_keyboard[fire_key]) {
    constructRobot();
  }

  if (menuPointer == MENU::EXIT && keyboard[fire_key] && !nether->old_keyboard[fire_key]) {
    delete staple;
    nether->setGameState(NETHER::STATE::PLAYING);
    nether->getShip()->pos.z = 2.0;
  }

  if (menuPointer == MENU::START && keyboard[fire_key] && !nether->old_keyboard[fire_key]) {
    buildRobot();
  }

  nether->redrawMenu();
  nether->redrawRadar();
  return true;
}


void ConstructionScreen::constructRobot()
{
  Robot proto(*staple);

  if (menuPointer >= MENU::CANNON) {
    proto.pieces[int(menuPointer) - 23] = !proto.pieces[int(menuPointer) - 23];
  }

  if (menuPointer <= MENU::ANTIGRAV) {
    if (proto.traction == int(menuPointer) - 20) {
      proto.traction = -1;
    } else {
      proto.traction = int(menuPointer) - 20;
    }
  }

  if (nether->stats.canBuildRobot(0, proto)) {
    staple->copyDesign(proto);
    nether->sManager.playSelect();
  } else {
    nether->sManager.playWrong();
  }
}


void ConstructionScreen::buildRobot()
{
  if (staple->valid()) {
    staple->angle = 0;
    staple->program = Robot::PROGRAM_FORWARD;
    staple->op = Robot::OPERATOR::NONE;
    staple->calculateCMC(Resources::pieceTiles[0]);
    staple->shipover = false;

    if (!staple->checkCollision(nether->map.buildings, nether->map.robots, true, nether->getShip())) {
      nether->addNewRobot(staple, 0);
      nether->stats.spendRobotResources(0, *staple);
      nether->getShip()->pos.z = 2.0;
      nether->setGameState(NETHER::STATE::PLAYING);
      nether->sManager.playConstruction();
      staple = nullptr;
    }
  } else {
    nether->sManager.playWrong();
  }
}


void ConstructionScreen::draw(int width, int height, const Light& light)
{
  float lightpos2[4] = {0, 0, 1000, 1};
  float tmpls[4] = {1.0F, 1.0F, 1.0F, 1.0};
  float tmpld[4] = {0.6F, 0.6F, 0.6F, 1.0};
  float tmpla[4] = {0.1F, 0.1F, 0.1F, 1.0};

  /* Enable Lights, etc.: */
  // glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, tmpla);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, tmpld);
  glLightfv(GL_LIGHT0, GL_SPECULAR, tmpls);
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos2);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glDisable(GL_SCISSOR_TEST);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);

  /* Draw the CONSTRUCTION screen: */
  glClearColor(0, 0, 0.0, 0);
  glViewport(0, 0, width, height);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, float(width) / float(height), 1.0, 1024.0);
  gluLookAt(0, 0, 64, 0, 0, 0, 0, 1, 0);

  /* Clear the color and depth buffers. */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glPushMatrix();
  glColor3f(1.0f, 1.0f, 0.0);
  glTranslatef(-12, 5, 0);
  scaledglprintf(0.01f, 0.01f, "-- RESOURCES --");
  glTranslatef(0, -1.4, 0);
  scaledglprintf(0.01f, 0.01f, "-- AVAILABLE --");

  {
    std::array<std::pair<int, int>, 7>resources {nether->stats.getResourceStats()};
    std::array<int, 7> receipt {nether->stats.calculateCost(*staple)};
    std::array<int, 7> normalized { nether->stats.normalizeCost(0, receipt)};

    glColor3f(0.5f, 1.0f, 0.5f);
    glTranslatef(0, -2.4, 0);
    scaledglprintf(0.01f, 0.01f, "    GENERAL %.2i", resources[0].first - normalized[0], 0);
    glTranslatef(0, -2.4, 0);
    scaledglprintf(0.01f, 0.01f, "ELECTRONICS %.2i", resources[1].first - normalized[1], 0);
    glTranslatef(0, -1.4, 0);
    scaledglprintf(0.01f, 0.01f, "    NUCLEAR %.2i", resources[2].first - normalized[2], 0);
    glTranslatef(0, -1.4, 0);
    scaledglprintf(0.01f, 0.01f, "    PHASERS %.2i", resources[3].first - normalized[3], 0);
    glTranslatef(0,-1.4,0);
    scaledglprintf(0.01f, 0.01f, "   MISSILES %.2i", resources[4].first - normalized[4], 0);
    glTranslatef(0, -1.4, 0);
    scaledglprintf(0.01f, 0.01f, "     CANNON %.2i", resources[5].first - normalized[5], 0);
    glTranslatef(0, -1.4, 0);
    scaledglprintf(0.01f, 0.01f, "    CHASSIS %.2i", resources[6].first - normalized[6], 0);

    glTranslatef(0, -2.4, 0);
    int total = std::accumulate(receipt.cbegin(), receipt.cend(), 0);
    scaledglprintf(0.01f, 0.01f, "      TOTAL %.2i", total);
  }

  glColor3f(0.3f, 0.8f, 1.0f);
  glTranslatef(0, -3,0);
  scaledglprintf(0.01f, 0.01f, "    EXIT  START");
  glTranslatef(0, -1.4, 0);
  scaledglprintf(0.01f, 0.01f, "    MENU  ROBOT");
  glPopMatrix();

  glPushMatrix();
  glTranslatef(-10, 12, 0);
  {
    float angle = float(sin(nether->getAnimationTimer() / 2) * 10);
    float angle2 = float(sin(nether->getAnimationTimer() / 4) * 15);
    glRotatef(angle2-10, 1, 0, 0);
    glRotatef(angle, 0, 1, 0);
    nether->increaseAnimationTimer(0.02);
  }
  Resources::constructionTiles[0].draw(Color(0.5f, 0.5f, 0.5f));
  glTranslatef(0.0, -1.1, 1);
  Resources::constructionTiles[1].draw(Color(0.8f, 0.8f, 0.8f));
  glTranslatef(0.0, 2.2, 0);
  Resources::constructionTiles[2].draw(Color(0.8f, 0.8f, 0.8f));
  glPopMatrix();

  glPushMatrix();
  glColor3f(1.0f, 0.0f, 0.0f);
  glTranslatef(12, 15, 0);
  staple->calculateCMC(Resources::pieceTiles[0]);
  if (staple->checkCollision(nether->map.buildings, nether->map.robots, true, nether->getShip()) &&
      (int(nether->getAnimationTimer() * 4) % 2) == 0) {
    scaledglprintf(0.01f, 0.01f, "ENTRANCE BLOCKED!");
  }

  glColor3f(0.3f, 0.8f, 1.0f);
  glTranslatef(3, -3,0);
  scaledglprintf(0.01f, 0.01f, "ELECTRONICS");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "NUCLEAR");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "PHASERS");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "MISSILES");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "CANNON");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "ANTI-GRAV");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "TRACKS");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "BIPOD");
  glPopMatrix();

  glPushMatrix();
  glColor3f(0.8f, 0.3f, 1.0f);
  glTranslatef(15, 10.6, 0);
  scaledglprintf(0.01f, 0.01f, "3");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "20");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "4");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "4");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "2");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "10");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "5");
  glTranslatef(0, -3.5, 0);
  scaledglprintf(0.01f, 0.01f, "3");
  glPopMatrix();

  {
    for (int i = 0; i < 8; i++) {
      glPushMatrix();
      glTranslatef(6, - (13.5 - i * 3.5), 0);
      glScalef(2.4, 2.4, 2.4);
      glRotatef(30, 1, 0, 0);
      glRotatef(nether->getAnimationTimer() * 32, 0, 1, 0);
      glRotatef(-90, 1, 0, 0);

      if (i < 3) {
        if (staple->traction == i) {
          Resources::pieceTiles[0][i].draw_notexture(Color(1.0, 1.0, 1.0));
        } else {
          Resources::pieceTiles[0][i].draw_notexture(Color(0.5, 0.5, 0.5));
        }
      } else {
        if (staple->pieces[i - 3]) {
          Resources::pieceTiles[0][i].draw_notexture(Color(1.0, 1.0, 1.0));
        } else {
          Resources::pieceTiles[0][i].draw_notexture(Color(0.5, 0.5, 0.5));
        }
      }

      glPopMatrix();
    }

    glPushMatrix();
    glTranslatef(0, -8,0);
    glScalef(4.0f, 4.0f, 4.0f);
    glRotatef(30, 1, 0, 0);
    glRotatef(nether->getAnimationTimer() * 32, 0, 1, 0);
    glRotatef(-90, 1, 0, 0);
    staple->draw(0, false, Resources::pieceTiles, light.asVector());
    glPopMatrix();
  }

  glPushMatrix();
  glColor3f(0.75f, 0.0, 0.0);
  if (menuPointer == MENU::EXIT) {
    glTranslatef(-14, -14, -1);
    glutSolidBox(3, 2, 0.5);
  }
  if (menuPointer == MENU::START) {
    glTranslatef(-7, -14, -1);
    glutSolidBox(3, 2, 0.5);
  }
  if (menuPointer == MENU::BIPOD) {
    glTranslatef(15.3, -13, -1);
    glutSolidBox(6, 1.7, 0.5);
  }
  if (menuPointer == MENU::TRACKS) {
    glTranslatef(15.3, -9.2, -1);
    glutSolidBox(6, 1.7, 0.5);
  }
  if (menuPointer == MENU::ANTIGRAV) {
    glTranslatef(15.3, -5.7, -1);
    glutSolidBox(6, 1.7, 0.5);
  }
  if (menuPointer == MENU::CANNON) {
    glTranslatef(15.3, -2.2, -1);
    glutSolidBox(6, 1.7, 0.5);
  }
  if (menuPointer == MENU::MISSILES) {
    glTranslatef(15.3, 1.2, -1);
    glutSolidBox(6, 1.7, 0.5);
  }
  if (menuPointer == MENU::PHASERS) {
    glTranslatef(15.3, 4.7, -1);
    glutSolidBox(6, 1.7, 0.5);
  }
  if (menuPointer == MENU::NUCLEAR) {
    glTranslatef(15.3, 8.2, -1);
    glutSolidBox(6, 1.7, 0.5);
  }
  if (menuPointer == MENU::ELECTRONICS) {
    glTranslatef(15.3, 12, -1);
    glutSolidBox(6, 1.7, 0.5);
  }
  glPopMatrix();
}


void ConstructionScreen::open(const Building& factory)
{
  nether->setGameState(NETHER::STATE::CONSTRUCTION);
  menuPointer = MENU::EXIT;
  staple = new Robot;
  staple->pos = factory.pos + Vector(2.5, 0.5, 0);
}
