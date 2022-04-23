#include <GL/gl.h>
#include <algorithm>
#include <unordered_map>
#include <sexp/util.hpp>

#include "building.h"
#include "buildingsimple.h"
#include "buildingfactory.h"
#include "buildingwarbase.h"
#include "constants.h"
#include "sexp/value.hpp"


std::unordered_map<std::string, BuildingBlock::TYPE> Building::simple_buildings_map =
    {
     {"fence", BuildingBlock::TYPE::FENCE},
     {"wall1", BuildingBlock::TYPE::WALL1},
     {"wall2", BuildingBlock::TYPE::WALL2},
     {"wall3", BuildingBlock::TYPE::WALL3},
     {"wall4", BuildingBlock::TYPE::WALL4},
     {"wall6", BuildingBlock::TYPE::WALL6}};

std::unordered_map<std::string, Building::SUBTYPE> Building::factories_map =
  {
    {"electronics", Building::SUBTYPE::ELECTRONICS},
    {"nuclear", Building::SUBTYPE::NUCLEAR},
    {"phasers", Building::SUBTYPE::PHASERS},
    {"missiles", Building::SUBTYPE::MISSILES},
    {"cannons", Building::SUBTYPE::CANNONS},
    {"chassis", Building::SUBTYPE::CHASSIS}
  };


Color Building::getFlagColor() const
{
  switch (owner) {
  case 1:
    return PLAYER1_COLOR;
  case 2:
    return PLAYER2_COLOR;
  default:
    return Color(0.5, 0.5, 0.5);
  }
}

Building* Building::getFromMapFile(std::istream& inFile)
{
  Building* building = nullptr;

  std::string buffer;
  Vector position{0, 0, 0};
  inFile >> buffer >> position.x >> position.y;

  if (buffer == "factory") {
    std::string buffer2;
    inFile >> buffer2;
    building = new BuildingFactory(position, factories_map.at(buffer2));
  } else if (buffer == "warbase") {
    building = new BuildingWarbase(position);
    inFile >> building->owner;
  } else if (buffer != "") {
    building = new BuildingSimple(position, simple_buildings_map.at(buffer));
  }

  return building;
}


Building* Building::getFromSexp(const sexp::Value& sexp)
{
  Building* building = nullptr;

  Vector position{0, 0, 0};
  std::string buildingType = sexp::car(sexp).as_string();
  position.x = sexp::cdar(sexp).as_float();
  position.y = sexp::cddar(sexp).as_float();

  if (buildingType == "factory") {
    building = new BuildingFactory(position, factories_map.at(sexp::cdddar(sexp).as_string()));
  } else if (buildingType == "warbase") {
    building = new BuildingWarbase(position);
    building->owner = sexp::cdddar(sexp).as_int();
  } else if (buildingType != "") {
    building = new BuildingSimple(position, simple_buildings_map.at(buildingType));
  }

  return building;
}


Building* Building::fromSexp(const sexp::Value& sexp)
{
  std::string buildingType = sexp::car(sexp).as_string();

  if (buildingType == "factory") {
    return new BuildingFactory(sexp);
  } else if (buildingType == "warbase") {
    return new BuildingWarbase(sexp);
  } else if (buildingType != "") {
    return new BuildingSimple(sexp);
  }

  std::cerr << "Cannot guess building type" << buildingType << std::endl;
  return nullptr;
}
