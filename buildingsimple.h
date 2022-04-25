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
    : Building(position, TYPE::SIMPLE), buildingBlockType {blockType}
  {
    blocks.emplace_back(new BuildingBlock(position, blockType));
  }
  BuildingSimple(const sexp::Value& sexp)
    : Building({0, 0, 0}, TYPE::SIMPLE)
  {
    pos = Vector(sexp::cdar(sexp));
    buildingBlockType = (BuildingBlock::TYPE)sexp::cddar(sexp).as_int();
    blocks.emplace_back(new BuildingBlock(pos, buildingBlockType, std::shared_ptr<Building>(this)));
  }
  Vector getCapturePoint() const override {return Vector();}
  bool isCapturable() const override {return false;}
  void draw(const bool shadows, const Vector& light) const override {};

  const std::vector<BuildingBlock> getTemplate() const override;

  sexp::Value toSexp() const override;

private :
  BuildingBlock::TYPE buildingBlockType;
};

#endif // BUILDINGSIMPLE_H
