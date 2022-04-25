#ifndef BUILDINGBLOCK_H
#define BUILDINGBLOCK_H

#include <iostream>
#include <memory>
#include <vector>

#include "cmc.h"
#include "shadow3dobject.h"
#include "vector.h"


class Building;


class BuildingBlock {
public:
  enum class TYPE {
    FENCE,
    WALL1,
    WALL2,
    WALL3,
    WALL4,
    WALL5,
    WALL6,
    FACTORY_ELECTRONICS,
    FACTORY_NUCLEAR,
    FACTORY_PHASERS,
    FACTORY_MISSILES,
    FACTORY_CANNONS,
    FACTORY_CHASSIS,
    WARBASE
  };

  BuildingBlock(Vector position, TYPE type, std::shared_ptr<Building> building=nullptr);

  void draw(const bool shadows, const Vector& light) const;

  bool collisionCheck(const CMC& other, float* m2) const;
  CMC getCMC() const;

  Vector pos;
  TYPE type;

  std::shared_ptr<Building> building;

private:
  bool isMainBuildingBlock() const;
  Shadow3DObject tile;
};

#endif // BUILDINGBLOCK_H
