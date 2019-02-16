#ifndef SHIP_H
#define SHIP_H

#include <string>

#include "shadow3dobject.h"


class Ship: public Shadow3DObject
{
public:
  enum OPS {NONE = -1,
            LEFT,
            RIGHT,
            FORWARD,
            BACKWARD,
            UP};
  Ship(const std::string& model, const std::string& texDir) :
    Shadow3DObject(model, texDir),
    pos(4.0, 2.0, 3.0),
    landed(false),
    op(OPS::NONE),
    op2(OPS::NONE),
    op3(OPS::NONE),
    timemoving(0)
  {};

  Vector pos;
  bool landed;
  int op, op2, op3;
  int timemoving;
};


# endif // SHIP_H
