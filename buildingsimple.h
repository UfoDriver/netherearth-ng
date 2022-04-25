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
  BuildingSimple(const Vector& position, BuildingBlock::TYPE blockType);
  BuildingSimple(const sexp::Value& sexp);

  Vector getCapturePoint() const override {return Vector();}
  bool isCapturable() const override {return false;}
  void draw(const bool shadows, const Vector& light) const override {};

  const std::vector<BuildingBlock> getTemplate() const override;

  sexp::Value toSexp() const override;

private :
  BuildingBlock::TYPE buildingBlockType;
};

#endif // BUILDINGSIMPLE_H
