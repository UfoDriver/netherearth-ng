#ifndef BUILDINGBLOCK_H
#define BUILDINGBLOCK_H

#include <iostream>
#include <memory>
#include <vector>

#include "cmc.h"
#include "shadow3dobject.h"
#include "vector.h"


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

  BuildingBlock(Vector position, TYPE type);

  void draw(const bool shadows, const Vector& light) const;
  static const std::vector<std::shared_ptr<BuildingBlock>> readMapFile(std::istream& inFile);

  bool collisionCheck(const CMC& other, float* m2) const;
  CMC getCMC() const;
  CMC getExtraCMC() const;
  bool collidesWith(const Vector& position, const CMC& cmc) const;

  Vector pos;
  TYPE type;

private:
  Shadow3DObject tile;
};

#endif // BUILDINGBLOCK_H
