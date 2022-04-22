#ifndef BUILDINGSIMPLE_H
#define BUILDINGSIMPLE_H

#include <sexp/util.hpp>
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
  BuildingSimple(const sexp::Value& sexp)
    : Building({0, 0, 0}, TYPE::SIMPLE)
  {
    pos = Vector(sexp::cdar(sexp));
    blocks.emplace_back(pos, BuildingBlock::TYPE::WALL1);
  }
  Vector getCapturePoint() const override {return Vector();}
  bool isCapturable() const override {return false;}

  sexp::Value toSexp() const override;
};

#endif // BUILDINGSIMPLE_H
