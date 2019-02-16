#include "building.h"
#include "utils.h"


Building::Building(std::istream& in)
{
  int type_;
  in >> type_ >> owner >> status >> pos;
  type = Building::BUILDINGS_AND_WALLS(type_);
}


std::ostream& operator<<(std::ostream& out, const Building& building)
{
  return out << building.type << ' ' << building.owner << ' ' << building.status << '\n'
             << building.pos;
}


const std::vector<Building> Building::readMapFile(std::istream& inFile)
{
  std::vector<Building> acc;
  std::string buffer;
  float x, y;
  inFile >> buffer >> x >> y;

  if (buffer == "fence") {
    acc.emplace_back(Vector(x, y, 0), Building::B_FENCE);
  } else if (buffer == "wall1") {
    acc.emplace_back(Vector(x, y, 0), Building::B_WALL1);
  } else if (buffer == "wall2") {
    acc.emplace_back(Vector(x, y, 0), Building::B_WALL2);
  } else if (buffer == "wall3") {
    acc.emplace_back(Vector(x, y, 0), Building::B_WALL3);
  } else if (buffer == "wall4") {
    acc.emplace_back(Vector(x, y, 0), Building::B_WALL4);
  } else if (buffer == "wall6") {
    acc.emplace_back(Vector(x, y, 0), Building::B_WALL6);
  } else if (buffer == "factory") {
    Building::BUILDINGS_AND_WALLS obj[4] = {Building::B_WALL4,
                                            Building::B_WALL4,
                                            Building::B_WALL2,
                                            Building::B_WALL2};
    float xo[4] = {0, 0, 1, 1};
    float yo[4] = {0, 2, 0, 2};
    std::string buffer2;
    inFile >> buffer2;
    for (int i = 0; i < 4; i++) {
      acc.emplace_back(Vector(x + xo[i], y + yo[i], 0), obj[i], 0, 0);
    }

    Building b(Vector(x, y + 1, 0), Building::B_FACTORY_ELECTRONICS);
    if (buffer2 == "electronics") b.type = Building::B_FACTORY_ELECTRONICS;
    if (buffer2 == "nuclear") b.type = Building::B_FACTORY_NUCLEAR;
    if (buffer2 == "phasers") b.type = Building::B_FACTORY_PHASERS;
    if (buffer2 == "missiles") b.type = Building::B_FACTORY_MISSILES;
    if (buffer2 == "cannons") b.type = Building::B_FACTORY_CANNONS;
    if (buffer2 == "chassis") b.type = Building::B_FACTORY_CHASSIS;
    acc.push_back(b);
  } else if (buffer == "warbase") {
    Building::BUILDINGS_AND_WALLS obj[15] = {Building::B_WALL4,
                                             Building::B_WALL5,
                                             Building::B_WALL4,
                                             Building::B_WALL1,
                                             Building::B_WALL1,
                                             Building::B_WALL2,
                                             Building::B_WALL4,
                                             Building::B_WARBASE,
                                             Building::B_WALL2,
                                             Building::B_WALL4,
                                             Building::B_WALL1,
                                             Building::B_WALL1,
                                             Building::B_WALL2,
                                             Building::B_WALL4,
                                             Building::B_WALL5};
    float xo[15] = {0.5, 1.5,
                    0, 1, 2, 3,
                    0.5, 1.5, 2.5,
                    0, 1, 2, 3,
                    0.5, 1.5};
    float yo[15] = {0, 0,
                    1, 1, 1, 1,
                    2, 2, 2,
                    3, 3, 3, 3,
                    4, 4};
    int owner = 0;
    inFile >> owner;
    for(int i = 0; i < 15; i++) {
      acc.emplace_back(Vector(x + xo[i], y + yo[i], 0), obj[i], owner, 0);
    }
  }
  return acc;
}
