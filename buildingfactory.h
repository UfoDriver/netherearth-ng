#ifndef BUILDINGFACTORY_H
#define BUILDINGFACTORY_H

#include <sexp/value.hpp>

#include "building.h"
#include "piece3dobject.h"
#include "vector.h"


class BuildingFactory : public Building
{
public:
  explicit BuildingFactory(const Vector& position, SUBTYPE subtype);
  BuildingFactory(const sexp::Value& value)
    : BuildingFactory(Vector(sexp::cddr(value)), (SUBTYPE)sexp::cdr(value).as_int())
  {
    owner = sexp::cdddr(value).as_int();
    status = sexp::cdddr(value.get_cdr()).as_int();
  };

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
  sexp::Value toSexp() const override;


private:
  Shadow3DObject flagTile;
  Piece3DObject typeTile;
};

#endif // BUILDINGFACTORY_H