#ifndef BUILDINGSIMPLE_H
#define BUILDINGSIMPLE_H

#include <sexp/value.hpp>

#include "building.h"
#include "buildingblock.h"
#include "vector.h"


class BuildingSimple : public Building
{
public:
  BuildingSimple(const Vector& position, BuildingBlock::TYPE blockType)
    : Building(position, TYPE::SIMPLE)
  {
    blocks.emplace_back(position, blockType);
  }
  Vector getCapturePoint() const override
  {
    return Vector();
  }
  bool isCapturable() const override
  {
    return false;
  }

  sexp::Value toSexp() const override;
};

#endif // BUILDINGSIMPLE_H
