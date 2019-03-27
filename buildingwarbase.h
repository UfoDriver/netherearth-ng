#ifndef BUILDINGWARBASE_H
#define BUILDINGWARBASE_H

#include "building.h"
#include "vector.h"


class BuildingWarbase : public Building
{
public:
  explicit BuildingWarbase(const Vector& position);

  void draw(const bool shadows, const int detaillevel, const Vector& light) const override;

  Vector getCapturePoint() const override
  {
    return pos + Vector(2, 0, 0);
  }
  bool isCapturable() const override
  {
    return true;
  }

private:
  Shadow3DObject flagTile;
};

#endif // BUILDINGWARBASE_H
