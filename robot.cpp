#include "robot.h"


Robot::Robot() : traction(-1), firetimer(0), strength(100),
                 electronics_state(0), chassis_state(0)
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


bool Robot::bulletHit(BULLET::BULLET_TYPE type)
{
  int npieces = 0;
  int damage;

  if (pieces[0]) npieces++;
  if (pieces[1]) npieces++;
  if (pieces[2]) npieces++;
  if (pieces[3]) npieces++;
  if (pieces[4]) npieces++;

  switch(type) {
  case BULLET::BULLET_CANNONS:
    damage = 24;
    if (traction == 0) damage -= 2;
    if (npieces == 1) damage -= 2;
    if (npieces == 2) damage -= 4;
    if (npieces == 3) damage -= 8;
    if (npieces == 4) damage -= 12;
    if (npieces == 5) damage -= 16;
    break;
  case BULLET::BULLET_MISSILES:
    damage = 36;
    if (traction == 0) damage -= 3;
    if (npieces == 1) damage -= 3;
    if (npieces == 2) damage -= 6;
    if (npieces == 3) damage -= 12;
    if (npieces == 4) damage -= 18;
    if (npieces == 5) damage -= 24;
    break;
  case BULLET::BULLET_PHASERS:
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
