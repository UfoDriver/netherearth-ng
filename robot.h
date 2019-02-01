#ifndef ROBOT_H
#define ROBOT_H

#include "bullet.h"
#include "cmc.h"
#include "vector.h"


class Robot {
public:
  Robot();
  bool valid();
  float piecez(int piece);
  bool bulletHit(Bullet::BULLET_TYPE type);

  int traction;
  bool pieces[5];

  int program;
  int program_parameter;
  Vector program_goal;

  int op;
  bool shipover;
  int firetimer;
  int strength;

  Vector pos;
  int angle;
  CMC cmc;

  /* Animation variables: */
  int electronics_state;
  int chassis_state;
};

#endif // ROBOT_H
