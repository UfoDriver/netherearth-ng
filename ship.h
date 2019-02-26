#ifndef SHIP_H
#define SHIP_H

#include <string>
#include <vector>

#include "resources.h"


class Building;
class Map;
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
  void draw(const bool shadows, const Vector& light, const Map& map, const Robot* controlled);
  bool landedHere(const Vector& position) const;

  Vector pos;
  bool landed;
  int op, op2, op3;
  int timemoving;
};


std::ostream& operator<<(std::ostream& out, const Ship& ship);
std::istream& operator>>(std::istream& in, Ship& ship);


# endif // SHIP_H
