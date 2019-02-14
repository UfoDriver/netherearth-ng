#include "ai_operator.h"


std::ostream& operator<<(std::ostream& out, const AIOperator& op)
{
  const char* map[] = {"NONE", "FORWARD", "LEFT", "RIGHT", "CANNONS", "MISSILES", "PHASERS", "NUCLEAR"};

  return out << "New position (" << op.newpos.x << '/' << op.newpos.y << ") "
             << map[op.first_robotop + 1]
             << " for " << op.cost;
}
