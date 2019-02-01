#ifndef AI_OPERATOR_H
#define AI_OPERATOR_H

#include "vector.h"


class AIOperator {
public:
  Vector newpos;
  int first_robotop;
  int cost;
  int previous;
  bool deadend;
  bool used;
};

#endif // AI_OPERATOR_H
