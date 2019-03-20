#include <GL/gl.h>
#include <algorithm>
#include <cmath>

#include "bullet.h"
#include "bulletcannon.h"
#include "bulletmissile.h"
#include "bulletphaser.h"
#include "building.h"
#include "constants.h"
#include "nether.h"
#include "piece3dobject.h"
#include "resources.h"
#include "robot.h"
#include "ship.h"

extern int detaillevel;
extern int up_key, down_key, left_key, right_key, fire_key, pause_key;


const float Robot::MS[4][3]={{0.0078125, 0.015625, 0.03125},
                             {0.00390625, 0.0078125, 0.03125},
                             {0, 0.0078125, 0.015625},
                             {0, 0, 0.03125}};

const int Robot::RS[4][3]={{2, 3, 5},
                           {1, 2, 5},
                           {0, 2, 3},
                           {0, 0, 5}};
int Robot::counter = 0;


Robot::Robot(unsigned short owner) : owner {owner}
{
}

Robot::Robot(unsigned short owner, std::istream &in) : owner {owner}
{
  in >> traction;
  for (int j = 0; j < 5; j++) {
    in >> pieces[j];
  }
  int program_;
  int op_;
  in >> program_ >> program_parameter.as_int;
  program = Robot::ROBOT_PROGRAMS(program_);
  in >> program_goal;
  in >> op_;
  in >> shipover;
  in >> firetimer >> strength;
  in >> pos;
  in >> angle;
  in >> cmc;
  in >> electronicsState >> chassisState;
  op = Robot::OPERATOR(op_);
}

bool Robot::valid() const
{
  if (traction == -1) return false;
  return pieces[0] || pieces[1] || pieces[2] || pieces[3];
}


int Robot::npieces() const
{
  return std::count(std::begin(pieces), std::end(pieces), true);
}


bool Robot::bulletHit(const std::unique_ptr<Bullet>& bullet)
{
  int damage = bullet->getDamageForRobot(this);
  strength -= damage;
  return strength > 0;
}


float Robot::piecez(int piece)
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
    if (chassisState >= 0) {
      if (chassisState < 32) {
        bipod_v = chassisState * 0.00390625;
      } else {
        bipod_v = (64 - chassisState) * 0.00390625;
      }
    } else {
      if (chassisState > -31) {
        bipod_v = chassisState * 0.00390625;
      } else {
        bipod_v = (-63 - chassisState) * 0.00390625;
      }
    }
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle, 0, 0, 1);
      if (detaillevel >= 3)
        Resources::pieceTiles[owner][8].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][8].draw_notexture(colors[owner]);
      glPushMatrix();
      glTranslatef(bipod_v,0,0);
      if (detaillevel>=3)
        Resources::pieceTiles[owner][9].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][9].draw_notexture(colors[owner]);
      glPopMatrix();
      glPushMatrix();
      glTranslatef(-bipod_v,0,0);
      if (detaillevel>=3)
        Resources::pieceTiles[owner][10].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][10].draw_notexture(colors[owner]);
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
      if (detaillevel>=3)
        Resources::pieceTiles[owner][1].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][1].draw_notexture(colors[owner]);
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
    z+=(cos(chassisState/15.0)*0.1)-0.1;
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (detaillevel>=3)
        Resources::pieceTiles[owner][2].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][2].draw_notexture(colors[owner]);
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
  if (pieces[0]) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (detaillevel>=3)
        Resources::pieceTiles[owner][3].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][3].draw_notexture(colors[owner]);
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
  if (pieces[1]) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (detaillevel>=3)
        Resources::pieceTiles[owner][4].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][4].draw_notexture(colors[owner]);
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
  if (pieces[2]) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (detaillevel>=3)
        Resources::pieceTiles[owner][5].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][5].draw_notexture(colors[owner]);
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
  if (pieces[3]) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (detaillevel>=3)
        Resources::pieceTiles[owner][6].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][6].draw_notexture(colors[owner]);
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
  if (pieces[4]) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (!pieces[3]) glTranslatef(-0.2,0,0);
      glRotatef(electronicsState,0,0,1);
      if (detaillevel >= 3)
        Resources::pieceTiles[owner][7].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][7].draw_notexture(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle+electronicsState,0,0,1);
      Resources::pieceTiles[owner][7].drawShadow(angle+electronicsState, lightposv, Color(0, 0, 0, 0.5));
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


float Robot::robotSpeed(int terrain) const
{
  if (terrain < 4 && traction < 3)
    return MS[terrain][traction];
  else
    return 0;
}

int Robot::robotRotationSpeed(int terrain) const
{
  if (terrain < 4 && traction < 3)
    return RS[terrain][traction];
  else
    return 0;
}


bool Robot::walkable(int terrain) const {
  return robotSpeed(terrain) !=0;
}

bool Robot::checkCollision(const std::vector<Building>& buildings,
                           const Robots& robots, bool complete, Ship* ship)
{
  float m1[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  pos.x, pos.y, pos.z, 1};
  float m2[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1};
  {
    float m2[16]={1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  ship->pos.x, ship->pos.y, ship->pos.z, 1};
    if (cmc.collision_simple(m1, ship->cmc, m2)) return true;
  }

  if (!complete) return false;

  for (const Building& b: buildings) {
    if (b.pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
      m2[12] = b.pos.x;
      m2[13] = b.pos.y;
      m2[14] = b.pos.z;
      if (cmc.collision_simple(m1, b.getCMC(), m2)) return true;
    }
  }

  for(Robot* rt: robots) {
    if (rt->pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
      if (rt != this) {
        m2[12] = rt->pos.x;
        m2[13] = rt->pos.y;
        m2[14] = rt->pos.z;
        if (cmc.collision_simple(m1, rt->cmc, m2)) return true;
      }
    }
  }

  return false;
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

  if (pieces[0]) {
    cmc.expand(pieceTiles[3].cmc, m);
    m[14] += 0.5;
  }
  if (pieces[1]) {
    cmc.expand(pieceTiles[4].cmc, m);
    m[14] += 0.35;
  }
  if (pieces[2]) {
    cmc.expand(pieceTiles[5].cmc, m);
    m[14] += 0.5;
  }
  if (pieces[3]) {
    cmc.expand(pieceTiles[6].cmc, m);
    m[14] += 0.8;
  }
  if (pieces[4]) {
    cmc.expand(pieceTiles[7].cmc, m);
  }
}


std::ostream& operator<<(std::ostream& out, const Robot& robot)
{
  out << robot.traction << '\n';
  for (int j = 0; j < 5; j++) {
    out << robot.pieces[j] << '\n';
  }
  out << robot.program << ' ' << robot.program_parameter.as_int << '\n';
  return out << robot.program_goal
             << int(robot.op) << '\n'
             << robot.shipover << '\n'
             << robot.firetimer << ' ' << robot.strength << '\n'
             << robot.pos
             << robot.angle << '\n'
             << robot.cmc
             << robot.electronicsState << ' ' << robot.chassisState << '\n';
}


void Robot::copyDesign(const Robot& robot)
{
  traction = robot.traction;
  std::copy(std::begin(robot.pieces), std::end(robot.pieces), std::begin(pieces));
}


void Robot::cycle()
{
  if (electronicsState) {
    electronicsState = (electronicsState + 6) % 360;
  }

  if (traction == 2) {
    chassisState++;
  }
}


void Robot::processOperator(NETHER* nether, unsigned char* keyboard)
{
  /* Apply ROBOT operator: */
  Vector old_pos {pos};
  int old_chassis_state {chassisState};
  float x[2], y[2];
  x[0] = pos.x - 0.5;
  x[1] = pos.x + 0.5;
  y[0] = pos.y - 0.5;
  y[1] = pos.y + 0.5;
  float minz = nether->map.maxZ(x, y);
  int terrain = nether->map.worseTerrain(x, y);

  /* Avoid that a Robot can walk agains another and they both get stuck: */
  if (op == Robot::OPERATOR::FORWARD &&
      (int(pos.x * 256) % 128) == 0 &&
      (int(pos.y * 256) % 128) == 0) {
    switch(angle) {
    case 0:
      if (nether->ai.worseMapTerrain(int((pos.x + 0.5) / 0.5), int((pos.y - 0.5) / 0.5), 1, 2) > T_HOLE)
        op = Robot::OPERATOR::NONE;
      break;
    case 90:
      if (nether->ai.worseMapTerrain(int((pos.x - 0.5) / 0.5), int((pos.y + 0.5) / 0.5), 2, 1) > T_HOLE)
        op = Robot::OPERATOR::NONE;
      break;
    case 180:
      if (nether->ai.worseMapTerrain(int((pos.x - 1.0) / 0.5), int((pos.y - 0.5) / 0.5), 1, 2) > T_HOLE)
        op = Robot::OPERATOR::NONE;
      break;
    case 270:
      if (nether->ai.worseMapTerrain(int((pos.x - 0.5) / 0.5), int((pos.y - 1.0) / 0.5), 2, 1) > T_HOLE)
        op = Robot::OPERATOR::NONE;
      break;
    }
  }

  if (op == Robot::OPERATOR::FORWARD) {
    float speed = robotSpeed(terrain);
    /* BIPOD ANIMATION: */
    if (traction == 0) {
      chassisState += int(speed / 0.00390625);
      if (chassisState > 64)
        chassisState =- 63;
    }

    /* TRACKS PARTICLES: */
    if (traction == 1) {
      if (detaillevel >= 4) {
        Vector pos, sp1;

        for (int i= 0; i < 2; i++) {
          pos.x = pos.x + float(rand() % 10) / 100.0;
          pos.y = pos.y + float(rand() % 10) / 100.0;
          pos.z = 0;
          Color color(0.9F + float(rand() % 21 - 10) / 100.0,
                      0.7F + float(rand() % 21 - 10) / 100.0,
                      0.5F + float(rand() % 21 - 10) / 100.0);
          switch (angle) {
          case 0:
            sp1 = Vector(-0.05, float(rand() % 9 - 4) / 200.0, 0);
            pos.x -= 0.25;
            pos.y += ((rand() % 2) == 0 ? -0.5 : 0.5);
            break;
          case 90:
            sp1 = Vector(float(rand() % 9 - 4) / 200.0, -0.05, 0);
            pos.y -= 0.25;
            pos.x += ((rand() % 2) == 0 ? -0.5 : 0.5);
            break;
          case 180:
            sp1 = Vector(0.05, float(rand() % 9 - 4) / 200.0, 0);
            pos.x += 0.25;
            pos.y += ((rand() % 2) == 0 ? -0.5 : 0.5);
            break;
          case 270:
            sp1 = Vector(float(rand() % 9 - 4) / 200.0, 0.05, 0);
            pos.y += 0.25;
            pos.x += ((rand() % 2) == 0 ? -0.5 : 0.5);
            break;
          }
          nether->map.particles.emplace_back(pos, sp1, Vector(0, 0, 0.05), 0, 0.3, color, 1.0, 0.0, 20 + (rand() % 10));
        }
      }
    }

    switch (angle) {
    case 0:
      if (pos.x < nether->map.width() - 0.5)
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
      if (pos.y < nether->map.height() - 0.5)
        pos.y -= speed;
      break;
    }
  } else {
    if (traction == 0)
      chassisState = 0;
  }

  if (op == Robot::OPERATOR::LEFT)
    angle -= robotRotationSpeed(terrain);
  if (op == Robot::OPERATOR::RIGHT)
    angle += robotRotationSpeed(terrain);
  if (angle >= 360)
    angle -= 360;
  if (angle < 0)
    angle += 360;

  if (op == Robot::OPERATOR::CANNONS && firetimer == 0) {
    Vector pos(pos);
    pos.z = piecez(0) + 0.3f;
    nether->map.bullets.emplace_back(new BulletCannon(pos, this));
    nether->sManager.playShot(nether->getShip()->pos, pos);
  }

  if (op == Robot::OPERATOR::MISSILES && firetimer == 0) {
    Vector pos (pos);
    pos.z = piecez(1) + 0.2f;
    nether->map.bullets.emplace_back(new BulletMissile(pos, this));
    nether->sManager.playShot(nether->getShip()->pos, pos);
  }

  if (op == Robot::OPERATOR::PHASERS && firetimer == 0) {
    Vector pos(pos);
    pos.z = piecez(2) + 0.3f;
    nether->map.bullets.emplace_back(new BulletPhaser(pos, this));
    nether->sManager.playShot(nether->getShip()->pos, pos);
  }

  if (op == Robot::OPERATOR::CANNONS ||
      op == Robot::OPERATOR::MISSILES ||
      op == Robot::OPERATOR::PHASERS)
    firetimer++;

  if (op == Robot::OPERATOR::NUCLEAR) {
    Explosion exp(pos, 2);

    nether->map.explosions.emplace_back(exp);

    /* Robot destroyed: */
    nether->detachShip(this);

    /* Find Robots to destroy: */
    // Ops, modifying container while iterating
    // @TODO: map should handle nuclear explosion
    nether->map.robots.erase(std::remove_if(nether->map.robots.begin(), nether->map.robots.end(),
                                            [exp, nether] (auto& r) {
                                              float distance=(r->pos - exp.pos).norma();
                                              if (distance <= NUCLEAR_RADIUS) {
                                                nether->ai.killRobot(r->pos);
                                                return true;
                                              } else {
                                                return false;
                                              }
                                            }),
                             nether->map.robots.end());

    /* Find buildings to destroy: */
    nether->map.buildings.erase(std::remove_if(nether->map.buildings.begin(), nether->map.buildings.end(),
                                               [exp, nether](auto& b) {
                                                 float distance = (b.pos - (exp.pos - Vector(0.5, 0.5, 0.5))).norma();
                                                 if (distance <= NUCLEAR_RADIUS) {
                                                   nether->ai.removeBuilding(b.pos);
                                                   return true;
                                                 } else {
                                                   return false;
                                                 }
                                               }),
                                nether->map.buildings.end());
    nether->sManager.playExplosion(nether->getShip()->pos, pos);
    nether->stats.requestRecomputing();
  }

  {
    x[0] = pos.x - 0.5;
    x[1] = pos.x + 0.5;
    y[0] = pos.y - 0.5;
    y[1] = pos.y + 0.5;
    minz = nether->map.maxZ(x, y);
    terrain = nether->map.worseTerrain(x, y);
    pos.z = minz;

    if (shipover) {
      nether->getShip()->pos.x = pos.x - 0.5;
      nether->getShip()->pos.y = pos.y - 0.5;
      nether->getShip()->pos.z = pos.z + cmc.z[1];
    }

    /* Collision: */
    if (checkCollision(nether->map.buildings, nether->map.robots, false, nether->getShip()) ||
        !walkable(terrain)) {
      pos = old_pos;
      if (traction == 0)
        chassisState = old_chassis_state;
      if (shipover) {
        nether->getShip()->pos.x = pos.x - 0.5;
        nether->getShip()->pos.y = pos.y - 0.5;
        nether->getShip()->pos.z = pos.z + cmc.z[1];
      }
    } else {
      nether->ai.moveRobot(old_pos, pos, getOwner());
    }

    if (op == Robot::OPERATOR::FORWARD &&
        (angle == 0 || angle == 180) &&
        (int(pos.x * 256) % 128) == 0)
      op = Robot::OPERATOR::NONE;
    if (op == Robot::OPERATOR::FORWARD &&
        (angle == 90 || angle == 270) &&
        (int(pos.y * 256) % 128) == 0)
      op = Robot::OPERATOR::NONE;
    if (op == Robot::OPERATOR::LEFT &&(angle % 90) == 0)
      op = Robot::OPERATOR::NONE;
    if (op == Robot::OPERATOR::RIGHT && (angle % 90) == 0)
      op = Robot::OPERATOR::NONE;
    if (op == Robot::OPERATOR::CANNONS && firetimer >= 64) {
      op = Robot::OPERATOR::NONE;
      firetimer = 0;
    }
    if (op == Robot::OPERATOR::MISSILES && firetimer >= 64) {
      op = Robot::OPERATOR::NONE;
      firetimer = 0;
    }
    if (op == Robot::OPERATOR::PHASERS && firetimer >= 64) {
      op = Robot::OPERATOR::NONE;
      firetimer = 0;
    }

    /* Follow ROBOT program: */
    if (op == Robot::OPERATOR::NONE && !shipover) {
      switch (program) {
      case Robot::PROGRAM_NONE:
        break;
      case Robot::PROGRAM_FORWARD:
        op = Robot::OPERATOR::FORWARD;
        break;
      case Robot::PROGRAM_STOPDEFEND:
        op = nether->ai.programStopDefend(*this, &(program_goal), owner + 1);
        break;
      case Robot::PROGRAM_ADVANCE:
        op = nether->ai.programAdvance(*this, getOwner() + 1);
        if (op == Robot::OPERATOR::FORWARD && angle == 90)
          program_parameter.as_int--;
        if (op == Robot::OPERATOR::FORWARD && angle == 270)
          program_parameter.as_int++;
        if (program_parameter.as_int == 0)
          program = Robot::PROGRAM_STOPDEFEND;
        break;
      case Robot::PROGRAM_RETREAT:
        op = nether->ai.programRetreat(*this, owner + 1);
        if (op == Robot::OPERATOR::FORWARD && angle == 270)
          program_parameter.as_int--;
        if (op == Robot::OPERATOR::FORWARD && angle == 90)
          program_parameter.as_int++;
        if (program_parameter.as_int == 0)
          program = Robot::PROGRAM_STOPDEFEND;
        break;
      case Robot::PROGRAM_DESTROY:
        op = nether->ai.programDestroy(*this, &(program_goal), owner + 1);
        break;
      case Robot::PROGRAM_CAPTURE:
        op = nether->ai.programCapture(*this, &(program_goal), owner + 1);
        break;
      }
    }

    /* Follow USER's command: */
    if (op == Robot::OPERATOR::NONE && shipover &&
        (nether->getActiveMenu() == Menu::TYPE::DIRECTCONTROL ||
         nether->getActiveMenu() == Menu::TYPE::DIRECTCONTROL2)) {
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
  }
}
