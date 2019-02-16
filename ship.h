#ifndef SHIP_H
#define SHIP_H

#include <string>
#include <vector>

#include "resources.h"


class Building;
class Robot;


class Ship: public Shadow3DObject
{
public:
  enum OPS {NONE = -1,
            LEFT,
            RIGHT,
            FORWARD,
            BACKWARD,
            UP};
  Ship(const std::string& model, const std::string& texturesDir);
  bool checkCollision(const std::vector<Building>& buildings, const std::vector<Robot*> robots[2]);

  Vector pos;
  bool landed;
  int op, op2, op3;
  int timemoving;
};


# endif // SHIP_H
