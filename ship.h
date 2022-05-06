#ifndef SHIP_H
#define SHIP_H

#include <memory>
#include <string>
#include <vector>

#include <sexp/value.hpp>

#include "resources.h"


class BuildingBlock;
class Config;
class Map;
class NETHER;
class Robot;


class Ship: public Shadow3DObject
{
public:
  enum OPS {
    NONE = -1,
    LEFT,
    RIGHT,
    FORWARD,
    BACKWARD,
    UP
  };
  Ship(const std::string& model, const std::string& texturesDir, NETHER* nether);
  bool checkCollision(const std::vector<std::shared_ptr<BuildingBlock>>& buildingBlocks, const std::vector<std::shared_ptr<Robot>>& robots);
  void draw(const bool shadows, const Vector& light, const Map& map, const std::shared_ptr<Robot> controlled);
  bool landedHere(const Vector& position) const;
  void cycle(const Config& config, unsigned char* keyboard);

  Vector pos {4.0, 2.0, 3.0};
  bool landed {false};
  int op {OPS::NONE}, op2 {OPS::NONE}, op3 {OPS::NONE};
  int timemoving {0};
  NETHER* nether;

  sexp::Value toSexp() const;
  bool fromSexp(const sexp::Value&);
};

# endif // SHIP_H
