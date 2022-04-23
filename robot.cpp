#include <algorithm>
#include <cmath>

#include <GL/gl.h>
#include <memory>
#include <sexp/util.hpp>
#include <sexp/value.hpp>

#include "bullet.h"
#include "bulletcannon.h"
#include "bulletmissile.h"
#include "bulletphaser.h"
#include "buildingblock.h"
#include "constants.h"
#include "nether.h"
#include "piece3dobject.h"
#include "resources.h"
#include "robot.h"
#include "ship.h"


extern int up_key, down_key, left_key, right_key, fire_key, pause_key;


const float Robot::MOVING_SPEED[4][3] = {
  {0.0078125, 0.015625, 0.03125},
  {0.00390625, 0.0078125, 0.03125},
  {0, 0.0078125, 0.015625},
  {0, 0, 0.03125}
};

const int Robot::ROTATION_SPEED[4][3] = {
  {2, 3, 5},
  {1, 2, 5},
  {0, 2, 3},
  {0, 0, 5}
};

int Robot::counter = 0;


Robot::Robot(unsigned short owner) : owner {owner}
{
}


bool Robot::valid() const
{
  if (traction == -1) return false;
  return hasCannons() || hasMissiles() || hasPhasers() || hasNuclear();
}


bool Robot::bulletHit(const std::unique_ptr<Bullet>& bullet)
{
  int damage = bullet->getDamageForRobot(this);
  strength -= damage;
  return strength > 0;
}


float Robot::piecesHeight(int piece)
{
  const float tractions[] {1.0, 0.35, 0.25};
  float z = tractions[traction];

  if (hasCannons()) {
    if (piece == 0) return z;
    z += 0.5;
  }
  if (hasMissiles()) {
    if (piece == 1) return z;
    z += 0.35;
  }
  if (hasPhasers()) {
    if (piece == 2) return z;
    z += 0.5;
  }
  if (hasNuclear()) {
    if (piece == 3) return z;
    z += 0.8;
  }

  return z;
}


void Robot::draw(Vector lightposv, bool shadows) const
{
  Color colors[2] = {{0.9, 0.9, 0.9}, {0.4, 0.4, 0.4}};
  float z = 0;
  float bipod_v = 0;
  Vector light;

  light = lightposv;
  light = light / light.z;

  glPushMatrix();
  switch(traction) {
  case 0:
    if (animation.chassis >= 0) {
      if (animation.chassis < 32) {
        bipod_v = animation.chassis * 0.00390625;
      } else {
        bipod_v = (64 - animation.chassis) * 0.00390625;
      }
    } else {
      if (animation.chassis > -31) {
        bipod_v = animation.chassis * 0.00390625;
      } else {
        bipod_v = (-63 - animation.chassis) * 0.00390625;
      }
    }
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle, 0, 0, 1);
      Resources::pieceTiles[owner][8].draw(colors[owner]);
      glPushMatrix();
      glTranslatef(bipod_v,0,0);
      Resources::pieceTiles[owner][9].draw(colors[owner]);
      glPopMatrix();
      glPushMatrix();
      glTranslatef(-bipod_v,0,0);
      Resources::pieceTiles[owner][10].draw(colors[owner]);
      glPopMatrix();
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][8].drawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPushMatrix();
      glTranslatef(bipod_v,0,0);
      Resources::pieceTiles[owner][9].drawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
      glPushMatrix();
      glTranslatef(-bipod_v,0,0);
      Resources::pieceTiles[owner][10].drawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
      glPopMatrix();
    } /* if */
    z+=1.0;
    break;
  case 1:
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][1].draw(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][1].drawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    } /* if */
    z+=0.35;
    break;
  case 2:
    z+=(cos(animation.chassis/15.0)*0.1)-0.1;
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][2].draw(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][2].drawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    } /* if */
    z+=0.45;
    break;
  } /* switch */
  if (hasCannons()) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][3].draw(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][3].drawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    } /* if */
    z+=0.5;
  } /* if */
  if (hasMissiles()) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][4].draw(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][4].drawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    }
    z+=0.35;
  } /* if */
  if (hasPhasers()) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][5].draw(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][5].drawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    } /* if */
    z+=0.5;
  } /* if */
  if (hasNuclear()) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][6].draw(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      Resources::pieceTiles[owner][6].drawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    } /* if */
    z+=0.8;
  } /* if */
  if (hasElectronics()) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (!hasNuclear()) glTranslatef(-0.2,0,0);
      glRotatef(animation.electronics,0,0,1);
      Resources::pieceTiles[owner][7].draw(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle+animation.electronics,0,0,1);
      Resources::pieceTiles[owner][7].drawShadow(angle+animation.electronics, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    } /* if */
  } /* if */
  glPopMatrix();
}


int Robot::cost() const
{
  int total = 0;
  int cost[5] = {2, 4, 4, 20, 3};

  if (traction == 0) total = 3;
  if (traction == 1) total = 5;
  if (traction == 2) total = 10;

  for (int i = 0; i < 5; i++) {
    if (pieces[i]) total += cost[i];
  }

  return total;
}


float Robot::movingSpeed(int terrain) const
{
  if (terrain < 4 && traction < 3)
    return MOVING_SPEED[terrain][traction];
  else
    return 0;
}


int Robot::rotationSpeed(int terrain) const
{
  if (terrain < 4 && traction < 3)
    return ROTATION_SPEED[terrain][traction];
  else
    return 0;
}


bool Robot::walkable(int terrain) const
{
  return movingSpeed(terrain) != 0;
}


bool Robot::checkCollision(const std::vector<std::shared_ptr<BuildingBlock>>& buildingBlocks,
                           const Robots& robots, bool complete, Ship* ship) const
{
  if (checkCollision(ship)) return true;
  if (!complete) return false;

  return
    checkCollision(buildingBlocks) or
    checkCollision(robots);
}


bool Robot::checkCollision(const std::vector<std::shared_ptr<BuildingBlock>>& buildingBlocks) const
{
  float m1[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  pos.x, pos.y, pos.z, 1};
  float m2[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1};
  for (const auto& b: buildingBlocks) {
    if (b->pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
      m2[12] = b->pos.x;
      m2[13] = b->pos.y;
      m2[14] = b->pos.z;
      // if (cmc.collision_simple(m1, b->getCMC(), m2)) return true;
      if (b->collisionCheck(cmc, m1)) return true;
    }
  }
  return false;
}


bool Robot::checkCollision(const Robots& robots) const
{
  float m1[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  pos.x, pos.y, pos.z, 1};
  float m2[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1};
  for(std::shared_ptr<Robot> rt: robots) {
    if (rt->pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
      if (rt.get() != this) {
        m2[12] = rt->pos.x;
        m2[13] = rt->pos.y;
        m2[14] = rt->pos.z;
        if (cmc.collision_simple(m1, rt->cmc, m2)) return true;
      }
    }
  }
  return false;
}


bool Robot::checkCollision(Ship* ship) const
{
  float m1[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  pos.x, pos.y, pos.z, 1};
  float m2[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  ship->pos.x, ship->pos.y, ship->pos.z, 1};
  return cmc.collision_simple(m1, ship->cmc, m2);
}


void Robot::calculateCMC(std::vector<Piece3DObject>& pieceTiles)
{
  float m[16] = {1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1};

  switch(traction) {
  case 0:
    cmc = pieceTiles[0].cmc;
    m[14] = 1.0;
    break;
  case 1:
    cmc = pieceTiles[1].cmc;
    m[14] = 0.35;
    break;
  case 2:
    cmc = pieceTiles[2].cmc;
    m[14] = 0.45;
    break;
  }

  if (hasCannons()) {
    cmc.expand(pieceTiles[3].cmc, m);
    m[14] += 0.5;
  }
  if (hasMissiles()) {
    cmc.expand(pieceTiles[4].cmc, m);
    m[14] += 0.35;
  }
  if (hasPhasers()) {
    cmc.expand(pieceTiles[5].cmc, m);
    m[14] += 0.5;
  }
  if (hasNuclear()) {
    cmc.expand(pieceTiles[6].cmc, m);
    m[14] += 0.8;
  }
  if (hasElectronics()) {
    cmc.expand(pieceTiles[7].cmc, m);
  }
}


void Robot::copyDesign(const Robot& robot)
{
  traction = robot.traction;
  pieces = robot.pieces;
}


void Robot::cycle(NETHER* nether)
{
  if (animation.electronics) {
    animation.electronics = (animation.electronics + 6) % 360;
  }

  if (traction == 2) {
    animation.chassis++;
  }

  if (op == OPERATOR::FORWARD) {
    if (traction == 0) { // Bipod
      animation.chassis = (animation.chassis + int(movingSpeed(nether->scene.map.worseTerrain(pos)) / 0.00390625)) % 64;
    }

    if (traction == 1) { // Tracks
      for (int i= 0; i < 2; i++) {
        Vector particlePos, particleSpeed;
        particlePos.x = pos.x + float(rand() % 10) / 100.0;
        particlePos.y = pos.y + float(rand() % 10) / 100.0;
        particlePos.z = 0;
        Color color(0.9F + float(rand() % 21 - 10) / 100.0,
                    0.7F + float(rand() % 21 - 10) / 100.0,
                    0.5F + float(rand() % 21 - 10) / 100.0);
        switch (angle) {
        case 0:
          particleSpeed = Vector(-0.05, float(rand() % 9 - 4) / 200.0, 0);
          particlePos.x -= 0.25;
          particlePos.y += ((rand() % 2) == 0 ? -0.5 : 0.5);
          break;
        case 90:
          particleSpeed = Vector(float(rand() % 9 - 4) / 200.0, -0.05, 0);
          particlePos.y -= 0.25;
          particlePos.x += ((rand() % 2) == 0 ? -0.5 : 0.5);
          break;
        case 180:
          particleSpeed = Vector(0.05, float(rand() % 9 - 4) / 200.0, 0);
          particlePos.x += 0.25;
          particlePos.y += ((rand() % 2) == 0 ? -0.5 : 0.5);
          break;
        case 270:
          particleSpeed = Vector(float(rand() % 9 - 4) / 200.0, 0.05, 0);
          particlePos.y += 0.25;
          particlePos.x += ((rand() % 2) == 0 ? -0.5 : 0.5);
          break;
        }
        nether->scene.particles.emplace_back(particlePos, particleSpeed, Vector(0, 0, 0.05), 0, 0.3, color, 1.0, 0.0, 20 + (rand() % 10));
      }
    }
  } else if (traction == 0) {
    animation.chassis = 0;
  }
}


void Robot::dispatchOperator(NETHER* nether, unsigned char* keyboard)
{
  Vector oldPos {pos};
  pos.z = nether->scene.map.getMaxZ(pos);

  switch (op) {
  case OPERATOR::FORWARD:
    processOperatorForward(nether, keyboard);
    break;
  case OPERATOR::LEFT:
    processOperatorLeft(nether, keyboard);
    break;
  case OPERATOR::RIGHT:
    processOperatorRight(nether, keyboard);
    break;
  case OPERATOR::CANNONS:
    processOperatorCannons(nether, keyboard);
    break;
  case OPERATOR::MISSILES:
    processOperatorMissiles(nether, keyboard);
    break;
  case OPERATOR::PHASERS:
    processOperatorPhasers(nether, keyboard);
    break;
  case OPERATOR::NUCLEAR:
    processOperatorNuclear(nether, keyboard);
    break;
  case OPERATOR::NONE:
    processOperatorNone(nether, keyboard);
    break;
  }

  if (checkCollision(nether->scene.map.buildingBlocks) or
      checkCollision(nether->scene.robots) or
      checkCollision(&nether->scene.ship) or
      !walkable(nether->scene.map.worseTerrain(pos))) {
    pos = oldPos;
  } else {
    nether->ai.moveRobot(oldPos, pos, owner);
  }

  if (shipover) {
    nether->scene.ship.pos.x = pos.x - 0.5;
    nether->scene.ship.pos.y = pos.y - 0.5;
    nether->scene.ship.pos.z = pos.z + cmc.z[1];
  }
}


void Robot::processOperatorForward(NETHER* nether, unsigned char*)
{
  // Avoid that a Robot can walk agains another and they both get stuck:
  if ((int(pos.x * 256) % 128) == 0 &&
      (int(pos.y * 256) % 128) == 0) {
    switch(angle) {
    case 0:
      if (nether->ai.worseMapTerrain(int((pos.x + 0.5) / 0.5), int((pos.y - 0.5) / 0.5), 1, 2) > Map::T_HOLE)
        op = Robot::OPERATOR::NONE;
      break;
    case 90:
      if (nether->ai.worseMapTerrain(int((pos.x - 0.5) / 0.5), int((pos.y + 0.5) / 0.5), 2, 1) > Map::T_HOLE)
        op = Robot::OPERATOR::NONE;
      break;
    case 180:
      if (nether->ai.worseMapTerrain(int((pos.x - 1.0) / 0.5), int((pos.y - 0.5) / 0.5), 1, 2) > Map::T_HOLE)
        op = Robot::OPERATOR::NONE;
      break;
    case 270:
      if (nether->ai.worseMapTerrain(int((pos.x - 0.5) / 0.5), int((pos.y - 1.0) / 0.5), 2, 1) > Map::T_HOLE)
        op = Robot::OPERATOR::NONE;
      break;
    }
  }

  float speed = movingSpeed(nether->scene.map.worseTerrain(pos));
  switch (angle) {
  case 0:
    if (pos.x < nether->scene.map.getWidth() - 0.5)
      pos.x += speed;
    break;
  case 90:
    if (pos.y > 0.5)
      pos.y += speed;
    break;
  case 180:
    if (pos.x > 0.5)
      pos.x -= speed;
    break;
  case 270:
    if (pos.y < nether->scene.map.getHeight() - 0.5)
      pos.y -= speed;
    break;
  }

  if ((angle == 0 || angle == 180) and (int(pos.x * 256) % 128) == 0)
    op = Robot::OPERATOR::NONE;

  if ((angle == 90 || angle == 270) and (int(pos.y * 256) % 128) == 0)
    op = Robot::OPERATOR::NONE;
}


void Robot::processOperatorLeft(NETHER *nether, unsigned char *)
{
  int terrain = nether->scene.map.worseTerrain(pos);
  angle = (angle + 360 - rotationSpeed(terrain)) % 360;

  if (angle % 90 == 0)
    op = Robot::OPERATOR::NONE;
}


void Robot::processOperatorRight(NETHER *nether, unsigned char *)
{
  int terrain = nether->scene.map.worseTerrain(pos);
  angle = (angle + rotationSpeed(terrain)) % 360;
  if (op == Robot::OPERATOR::RIGHT && (angle % 90) == 0)
    op = Robot::OPERATOR::NONE;
}


void Robot::processOperatorCannons(NETHER *nether, unsigned char *)
{
  if (firetimer == 0) {
    Vector bulletPos {pos};
    bulletPos.z = piecesHeight(0) + 0.3f;
    nether->scene.bullets.emplace_back(new BulletCannon(bulletPos, this));
    nether->sManager.playShot(nether->scene.ship.pos, bulletPos);
  }
  firetimer++;
  if (firetimer >= 64) {
    op = Robot::OPERATOR::NONE;
    firetimer = 0;
  }
}


void Robot::processOperatorMissiles(NETHER *nether, unsigned char *)
{
  if (firetimer == 0) {
    Vector bulletPos {pos};
    bulletPos.z = piecesHeight(1) + 0.2f;
    nether->scene.bullets.emplace_back(new BulletMissile(bulletPos, this));
    nether->sManager.playShot(nether->scene.ship.pos, bulletPos);
  }
  firetimer++;
  if (firetimer >= 64) {
    op = Robot::OPERATOR::NONE;
    firetimer = 0;
  }
}


void Robot::processOperatorPhasers(NETHER *nether, unsigned char *)
{
  if (firetimer == 0) {
    Vector bulletPos {pos};
    bulletPos.z = piecesHeight(2) + 0.3f;
    nether->scene.bullets.emplace_back(new BulletPhaser(bulletPos, this));
    nether->sManager.playShot(nether->scene.ship.pos, bulletPos);
  }
  firetimer++;
  if (firetimer >= 64) {
    op = Robot::OPERATOR::NONE;
    firetimer = 0;
  }
}


void Robot::processOperatorNuclear(NETHER *nether, unsigned char *)
{
  nether->scene.map.nuclearExplosionAt(pos);
  nether->scene.nuclearExplosionAt(pos);
}


void Robot::processOperatorNone(NETHER* nether, unsigned char* keyboard)
{
  if (!shipover) {
    processProgram(nether, keyboard);
  } else {
    if (nether->getActiveMenu() == Menu::TYPE::DIRECTCONTROL ||
        nether->getActiveMenu() == Menu::TYPE::DIRECTCONTROL2)
      processDirectInput(nether, keyboard);
  }
}


void Robot::processProgram(NETHER* nether, unsigned char*)
{
  switch (program.type) {
  case RobotProgram::NONE:
    break;
  case RobotProgram::FORWARD:
    op = Robot::OPERATOR::FORWARD;
    break;
  case RobotProgram::STOPDEFEND:
    op = nether->ai.programStopDefend(*this, &program.goal, owner + 1);
    break;
  case RobotProgram::ADVANCE:
    op = nether->ai.programAdvance(*this, getOwner() + 1);
    if (op == Robot::OPERATOR::FORWARD && angle == 90)
      program.parameter.as_int--;
    if (op == Robot::OPERATOR::FORWARD && angle == 270)
      program.parameter.as_int++;
    if (program.parameter.as_int == 0)
      program.type = RobotProgram::STOPDEFEND;
    break;
  case RobotProgram::RETREAT:
    op = nether->ai.programRetreat(*this, owner + 1);
    if (op == Robot::OPERATOR::FORWARD && angle == 270)
      program.parameter.as_int--;
    if (op == Robot::OPERATOR::FORWARD && angle == 90)
      program.parameter.as_int++;
    if (program.parameter.as_int == 0)
      program.type = RobotProgram::STOPDEFEND;
    break;
  case RobotProgram::DESTROY:
    op = nether->ai.programDestroy(*this, &program.goal, owner + 1);
    break;
  case RobotProgram::CAPTURE:
    op = nether->ai.programCapture(*this, &program.goal, owner + 1);
    break;
  }
}


void Robot::processDirectInput(NETHER*, unsigned char* keyboard)
{
  if (keyboard[right_key]) {
    if (angle == 0) {
      op = Robot::OPERATOR::FORWARD;
    } else {
      if (angle == 270)
        op = Robot::OPERATOR::RIGHT;
      else
        op = Robot::OPERATOR::LEFT;
    }
  }
  if (keyboard[left_key]) {
    if (angle == 180) {
      op = Robot::OPERATOR::FORWARD;
    } else {
      if (angle == 90)
        op = Robot::OPERATOR::RIGHT;
      else
        op = Robot::OPERATOR::LEFT;
    }
  }
  if (keyboard[up_key]) {
    if (angle == 90) {
      op = Robot::OPERATOR::FORWARD;
    } else {
      if (angle == 0)
        op = Robot::OPERATOR::RIGHT;
      else
        op = Robot::OPERATOR::LEFT;
    }
  }
  if (keyboard[down_key]) {
    if (angle == 270) {
      op = Robot::OPERATOR::FORWARD;
    } else {
      if (angle == 180)
        op = Robot::OPERATOR::RIGHT;
      else
        op = Robot::OPERATOR::LEFT;
    }
  }
}


sexp::Value Robot::toSexp() const
{
  sexp::Value programSexp = sexp::Value::list(
    sexp::Value::symbol("program"),
    sexp::Value::integer(program.type),
    sexp::Value::integer(program.parameter.as_int),
    program.goal.toSexp()
  );

  sexp::Value piecesSexp = sexp::Value::list(
    sexp::Value::symbol("pieces"),
    sexp::Value::integer(pieces[0]),
    sexp::Value::integer(pieces[1]),
    sexp::Value::integer(pieces[2]),
    sexp::Value::integer(pieces[3]),
    sexp::Value::integer(pieces[4])
  );

  return sexp::Value::list(
    sexp::Value::symbol("robot"),
    sexp::Value::integer(owner),
    sexp::Value::integer(traction),
    piecesSexp,
    programSexp,
    sexp::Value::integer((int)op),
    sexp::Value::boolean(shipover),
    sexp::Value::integer(firetimer),
    sexp::Value::integer(strength),
    pos.toSexp(),
    sexp::Value::integer(angle),
    cmc.toSexp(),
    sexp::Value::integer(animation.electronics),
    sexp::Value::integer(animation.chassis)
  );
}


bool Robot::fromSexp(const sexp::Value& value)
{
  owner = sexp::cdar(value).as_int();
  traction = sexp::cddar(value).as_int();
  sexp::Value piecesSexp = sexp::cdddar(value);
  pieces[0] = sexp::cdar(piecesSexp).as_int();
  pieces[1] = sexp::cddar(piecesSexp).as_int();
  pieces[2] = sexp::cdddar(piecesSexp).as_int();
  pieces[3] = sexp::cdddar(piecesSexp.get_cdr()).as_int();
  pieces[4] = sexp::cdddar(piecesSexp.get_cdr().get_cdr()).as_int();

  sexp::Value tmpVal1 = sexp::cdddar(value).get_cdr();
  sexp::Value programSexp = sexp::car(tmpVal1);
  program.type = (RobotProgram::TYPE)sexp::cdar(programSexp).as_int();
  program.parameter.as_int = sexp::cddar(programSexp).as_int();
  program.goal.fromSexp(sexp::cdddar(programSexp));

  op = (Robot::OPERATOR)sexp::cdar(tmpVal1).as_int();
  shipover = sexp::cddar(tmpVal1).as_bool();
  firetimer = sexp::cdddar(tmpVal1).as_int();

  sexp::Value tmpVal2 = sexp::cdddar(tmpVal1).get_cdr();
  strength = sexp::car(tmpVal2).as_int();
  pos.fromSexp(sexp::cdar(tmpVal2));
  angle = sexp::cddar(tmpVal2).as_int();
  cmc.fromSexp(sexp::cdddar(tmpVal2));

  sexp::Value tmpVal3 = sexp::cdddar(tmpVal2).get_cdr();
  animation.electronics = sexp::car(tmpVal3).as_int();
  animation.chassis = sexp::cdar(tmpVal3).as_int();

  return true;
}
