#ifndef BUILDING_H
#define BUILDING_H

#include "vector.h"


class Building {
public:
  enum BUILDINGS_AND_WALLS {B_FENCE,
                            B_WALL1,
                            B_WALL2,
                            B_WALL3,
                            B_WALL4,
                            B_WALL5,
                            B_WALL6,
                            B_FACTORY_ELECTRONICS,
                            B_FACTORY_NUCLEAR,
                            B_FACTORY_PHASERS,
                            B_FACTORY_MISSILES,
                            B_FACTORY_CANNONS,
                            B_FACTORY_CHASSIS,
                            B_WARBASE};
  Building(Vector position, BUILDINGS_AND_WALLS type, int owner=0, int status=0):
    pos(position), type(type), owner(owner), status(status)
  {}
  explicit Building(std::istream& in);

  Vector pos;
  BUILDINGS_AND_WALLS type;
  int owner;
  int status;
};

#endif // BUILDING_H
