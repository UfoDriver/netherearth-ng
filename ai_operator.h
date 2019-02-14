#ifndef AI_OPERATOR_H
#define AI_OPERATOR_H

#include <iostream>

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

std::ostream& operator<<(std::ostream& out, const AIOperator& op);

#endif // AI_OPERATOR_H
