#ifndef BUILDINGWARBASE_H
#define BUILDINGWARBASE_H

#include <sexp/value.hpp>

#include "building.h"
#include "vector.h"


class BuildingWarbase : public Building
{
public:
  explicit BuildingWarbase(const Vector& position);

  void draw(const bool shadows, const Vector& light) const override;

  Vector getCapturePoint() const override
  {
    return pos + Vector(2, 0, 0);
  }
  bool isCapturable() const override
  {
    return true;
  }
  sexp::Value toSexp() const override;

private:
  Shadow3DObject flagTile;
};

#endif // BUILDINGWARBASE_H
