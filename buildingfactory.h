#ifndef BUILDINGFACTORY_H
#define BUILDINGFACTORY_H

#include "building.h"
#include "piece3dobject.h"
#include "vector.h"


class BuildingFactory : public Building
{
public:
  explicit BuildingFactory(const Vector& position, SUBTYPE subtype);

  void draw(const bool shadows, const Vector& light) const override;

  Vector getCapturePoint() const override
  {
      return pos + Vector(1, 0, 0);
  }
  bool isCapturable() const override
  {
    return true;
  }
  bool collisionCheck(const CMC& other, float* m2) const override;

private:
  Shadow3DObject flagTile;
  Piece3DObject typeTile;
};

#endif // BUILDINGFACTORY_H
