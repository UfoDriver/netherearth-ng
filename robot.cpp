#include <GL/gl.h>

#include "cmath"
#include "piece3dobject.h"
#include "robot.h"


extern int detaillevel;

int Robot::counter = 0;

Robot::Robot() : traction(-1), firetimer(0), strength(100),
                 electronics_state(0), chassis_state(0), id(Robot::counter++)
{
  pieces[0]=false;
  pieces[1]=false;
  pieces[2]=false;
  pieces[3]=false;
  pieces[4]=false;
};


bool Robot::valid()
{
  if (traction == -1) return false;
  return pieces[0] || pieces[1] || pieces[2] || pieces[3];
}


bool Robot::bulletHit(Bullet::BULLET_TYPE type)
{
  int npieces = 0;
  int damage;

  if (pieces[0]) npieces++;
  if (pieces[1]) npieces++;
  if (pieces[2]) npieces++;
  if (pieces[3]) npieces++;
  if (pieces[4]) npieces++;

  switch(type) {
  case Bullet::BULLET_CANNONS:
    damage = 24;
    if (traction == 0) damage -= 2;
    if (npieces == 1) damage -= 2;
    if (npieces == 2) damage -= 4;
    if (npieces == 3) damage -= 8;
    if (npieces == 4) damage -= 12;
    if (npieces == 5) damage -= 16;
    break;
  case Bullet::BULLET_MISSILES:
    damage = 36;
    if (traction == 0) damage -= 3;
    if (npieces == 1) damage -= 3;
    if (npieces == 2) damage -= 6;
    if (npieces == 3) damage -= 12;
    if (npieces == 4) damage -= 18;
    if (npieces == 5) damage -= 24;
    break;
  case Bullet::BULLET_PHASERS:
    damage = 48;
    if (traction == 0) damage -= 4;
    if (npieces == 1) damage -= 4;
    if (npieces == 2) damage -= 8;
    if (npieces == 3) damage -= 16;
    if (npieces == 4) damage -= 24;
    if (npieces == 5) damage -= 32;
    break;
  }

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

  // CANON:
  if (pieces[0]) {
    if (piece == 0) return z;
    z += 0.5;
  }
  // MISSILES:
  if (pieces[1]) {
    if (piece == 1) return z;
    z += 0.35;
  }
  // PHASER:
  if (pieces[2]) {
    if (piece == 2) return z;
    z += 0.5;
  }
  // NUCLEAR:
  if (pieces[3]) {
    if (piece == 3) return z;
    z += 0.8;
  }

  return z;
}


void Robot::draw(int owner, bool shadows, std::vector<Piece3DObject> piece_tiles[2], Vector lightposv)
{
  Color colors[2] = {{0.9, 0.9, 0.9}, {0.4, 0.4, 0.4}};
  float r[2] = {0.9, 0.4};
  float g[2] = {0.9, 0.4};
  float b[2] = {0.9, 0.4};
  float z = 0;
  float bipod_v = 0;
  Vector light;

  light=lightposv;
  light=light / light.z;

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
        piece_tiles[owner][8].draw(colors[owner]);
      else piece_tiles[owner][8].draw_notexture(colors[owner]);
      glPushMatrix();
      glTranslatef(bipod_v,0,0);
      if (detaillevel>=3) piece_tiles[owner][9].draw(colors[owner]);
      else piece_tiles[owner][9].draw_notexture(colors[owner]);
      glPopMatrix();
      glPushMatrix();
      glTranslatef(-bipod_v,0,0);
      if (detaillevel>=3) piece_tiles[owner][10].draw(colors[owner]);
      else piece_tiles[owner][10].draw_notexture(colors[owner]);
      glPopMatrix();
      //			piece_tile[owner][0]->draw(r[owner],g[owner],b[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      piece_tiles[owner][8].DrawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPushMatrix();
      glTranslatef(bipod_v,0,0);
      piece_tiles[owner][9].DrawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
      glPushMatrix();
      glTranslatef(-bipod_v,0,0);
      piece_tiles[owner][10].DrawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
      //			piece_tile[owner][0]->DrawShadow(angle,lightposv,0,0,0,0.5);
      glPopMatrix();
    } /* if */ 
    z+=1.0;
    break;
  case 1:
    if (!shadows) {
      glPushMatrix();
      glRotatef(angle,0,0,1);
      if (detaillevel>=3) piece_tiles[owner][1].draw(colors[owner]);
      else piece_tiles[owner][1].draw_notexture(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      piece_tiles[owner][1].DrawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
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
      if (detaillevel>=3) piece_tiles[owner][2].draw(colors[owner]);
      else piece_tiles[owner][2].draw_notexture(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      piece_tiles[owner][2].DrawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
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
      if (detaillevel>=3) piece_tiles[owner][3].draw(colors[owner]);
      else piece_tiles[owner][3].draw_notexture(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      piece_tiles[owner][3].DrawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    } /* if */
    z+=0.5;
  } /* if */
  if (pieces[1]) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (detaillevel>=3) piece_tiles[owner][4].draw(colors[owner]);
      else piece_tiles[owner][4].draw_notexture(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      piece_tiles[owner][4].DrawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    }
    z+=0.35;
  } /* if */
  if (pieces[2]) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (detaillevel>=3) piece_tiles[owner][5].draw(colors[owner]);
      else piece_tiles[owner][5].draw_notexture(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      piece_tiles[owner][5].DrawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    } /* if */
    z+=0.5;
  } /* if */
  if (pieces[3]) {
    if (!shadows) {
      glPushMatrix();
      glTranslatef(0,0,z);
      glRotatef(angle,0,0,1);
      if (detaillevel>=3) piece_tiles[owner][6].draw(colors[owner]);
      else piece_tiles[owner][6].draw_notexture(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle,0,0,1);
      piece_tiles[owner][6].DrawShadow(angle, lightposv, Color(0, 0, 0, 0.5));
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
      if (detaillevel >= 3) piece_tiles[owner][7].draw(colors[owner]);
      else piece_tiles[owner][7].draw_notexture(colors[owner]);
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(-z*light.x,-z*light.y,0.05);
      glRotatef(angle+electronics_state,0,0,1);
      piece_tiles[owner][7].DrawShadow(angle+electronics_state, lightposv, Color(0, 0, 0, 0.5));
      glPopMatrix();
    } /* if */
  } /* if */
  glPopMatrix();
}


void Robot::cost(int player, int* res, int resources[2][7])
{
  // int i;
  int tmp;
  int corr[5] = {5, 4, 3, 2, 1};
  int cost[5] = {2, 4, 4, 20, 3};

  res[0] = 0;
  res[1] = 0;
  res[2] = 0;
  res[3] = 0;
  res[4] = 0;
  res[5] = 0;
  res[6] = 0;

  tmp = 0;
  if (traction == 0) tmp = 3;
  if (traction == 1) tmp = 5;
  if (traction == 2) tmp = 10;
  if (resources[player][6] > tmp) {
    res[6] = tmp;
  } else {
    res[6] = resources[player][6];
    res[0] = tmp - resources[player][6];
  }

  for(int i = 0; i < 5; i++) {
    if (pieces[i]) {
      if (resources[player][corr[i]] > cost[i]) {
        res[corr[i]] = cost[i];
      } else {
        res[corr[i]] = resources[player][corr[i]];
        res[0] += cost[i] - resources[player][corr[i]];
      }
    }
  }
}

int Robot::cost()
{
  int tmp = 0;
  int cost[5] = {2, 4, 4, 20, 3};

  if (traction == 0) tmp = 3;
  if (traction == 1) tmp = 5;
  if (traction == 2) tmp = 10;

  for (int i = 0;i < 5; i++) {
    if (pieces[i]) tmp += cost[i];
  }

  return tmp;
}
