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
    std::shared_ptr<BuildingBlock> b {new BuildingBlock(position, blockType)};
    blocks.push_back(b);
    b->building = std::shared_ptr<Building>(this);
  }
  BuildingSimple(const sexp::Value& sexp)
    : Building({0, 0, 0}, TYPE::SIMPLE)
  {
    pos = Vector(sexp::cdar(sexp));
    blocks.push_back(std::shared_ptr<BuildingBlock>(new BuildingBlock(pos, (BuildingBlock::TYPE)sexp::cddar(sexp).as_int())));
  }
  Vector getCapturePoint() const override {return Vector();}
  bool isCapturable() const override {return false;}
  void draw(const bool shadows, const Vector& light) const override {};

  sexp::Value toSexp() const override;
};

#endif // BUILDINGSIMPLE_H
