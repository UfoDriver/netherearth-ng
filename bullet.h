#ifndef BULLET_H
#define BULLET_H

#include "cmc.h"
#include "vector.h"

class Robot;


class BULLET {
public:
  enum BULLET_TYPE {BULLET_CANNONS, BULLET_MISSILES, BULLET_PHASERS};
  BULLET(void);
  BULLET(BULLET_TYPE type, Vector position, int angle, Robot *robot);

  BULLET_TYPE type;
  int step;

  Vector pos;
  int angle;
  Robot *owner;	/* The robot who fired this bullet */

  CMC cmc;
};

#endif // BULLET_H
