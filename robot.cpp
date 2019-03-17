#include <GL/gl.h>
#include <algorithm>
#include <cmath>

#include "constants.h"
#include "piece3dobject.h"
#include "resources.h"
#include "robot.h"
#include "ship.h"
#include "utils.h"

extern int detaillevel;

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


Robot::Robot(std::istream& in)
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
  in >> electronics_state >> chassis_state;
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
  float z = 0;
  switch (traction) {
  case 0:
    z += 1.0;
    break;
  case 1:
    z += 0.35;
    break;
  case 2:
    z += 0.25;
    break;
  }

  if (hasCannons()) {
    if (piece == 0) return z;
    z += 0.5;
  }
  if (hasMissiles()) {
    if (piece == 1) return z;
    z += 0.35;
  }
  // PHASER:
  if (hasPhasers()) {
    if (piece == 2) return z;
    z += 0.5;
  }
  // NUCLEAR:
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
    if (chassis_state >= 0) {
      if (chassis_state < 32) {
        bipod_v = chassis_state * 0.00390625;
      } else {
        bipod_v = (64 - chassis_state) * 0.00390625;
      }
    } else {
      if (chassis_state >- 31) {
        bipod_v = chassis_state * 0.00390625;
      } else {
        bipod_v = (-63 - chassis_state) * 0.00390625;
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
      //			piece_tile[owner][0]->draw(r[owner],g[owner],b[owner]);
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
      //			piece_tile[owner][0]->drawShadow(angle,lightposv,0,0,0,0.5);
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
    z+=(cos(chassis_state/15.0)*0.1)-0.1;
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
      glRotatef(electronics_state,0,0,1);
      if (detaillevel >= 3)
        Resources::pieceTiles[owner][7].draw(colors[owner]);
      else
        Resources::pieceTiles[owner][7].draw_notexture(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle+electronics_state,0,0,1);
      Resources::pieceTiles[owner][7].drawShadow(angle+electronics_state, lightposv, Color(0, 0, 0, 0.5));
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
                           const std::vector<Robot*> robots[2], bool complete, Ship* ship)
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

  for (int i = 0; i < 2; i++) {
    for(Robot* rt: robots[i]) {
      if (rt->pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
        if (rt != this) {
          m2[12] = rt->pos.x;
          m2[13] = rt->pos.y;
          m2[14] = rt->pos.z;
          if (cmc.collision_simple(m1, rt->cmc, m2)) return true;
        }
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
             << robot.electronics_state << ' ' << robot.chassis_state << '\n';
}


void Robot::copyDesign(const Robot& robot)
{
  traction = robot.traction;
  std::copy(std::begin(robot.pieces), std::end(robot.pieces), std::begin(pieces));
}
