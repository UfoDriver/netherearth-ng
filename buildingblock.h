#ifndef BUILDINGBLOCK_H
#define BUILDINGBLOCK_H

#include <iostream>
#include <memory>
#include <vector>

#include "cmc.h"
#include "vector.h"


class BuildingBlock {
public:
  enum class TYPE {FENCE,
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
                   WARBASE};
  BuildingBlock(Vector position, TYPE type, int owner=0, int status=0):
    pos{position}, type{type}, owner{owner}, status{status}
  {}
  explicit BuildingBlock(std::istream& in);

  void draw(const bool shadows, const int detaillevel, const Vector& light) const;
  static const std::vector<std::unique_ptr<BuildingBlock>> readMapFile(std::istream& inFile);

  CMC getCMC() const;
  CMC getExtraCMC() const;
  bool collidesWith(const Vector& position, const CMC& cmc) const;
  Vector getCapturePoint();
  bool isCapturable();

  Vector pos;
  TYPE type;
  int owner;
  int status;
};

std::ostream& operator<<(std::ostream& out, const BuildingBlock& bulding);

#endif // BUILDINGBLOCK_H
