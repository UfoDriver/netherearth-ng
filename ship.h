#ifndef SHIP_H
#define SHIP_H

#include <string>
#include <vector>

#include "resources.h"


class Building;
class Map;
class NETHER;
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
  Ship(const std::string& model, const std::string& texturesDir, NETHER* nether);
  bool checkCollision(const std::vector<Building>& buildings, const std::vector<Robot*>& robots);
  void draw(const bool shadows, const Vector& light, const Map& map, const Robot* controlled);
  bool landedHere(const Vector& position) const;
  void cycle(unsigned char* keyboard);

  Vector pos {4.0, 2.0, 3.0};
  bool landed {false};
  int op {OPS::NONE}, op2 {OPS::NONE}, op3 {OPS::NONE};
  int timemoving {0};
  NETHER* nether;
};


std::ostream& operator<<(std::ostream& out, const Ship& ship);
std::istream& operator>>(std::istream& in, Ship& ship);


# endif // SHIP_H
