#ifndef BUILDING_H
#define BUILDING_H

#include <unordered_map>
#include <vector>
#include <sexp/value.hpp>

#include "buildingblock.h"
#include "vector.h"


class Building
{
public:
  enum class TYPE {
    SIMPLE,
    FACTORY,
    WARBASE
  };
  enum class SUBTYPE {
    ELECTRONICS,
    NUCLEAR,
    PHASERS,
    MISSILES,
    CANNONS,
    CHASSIS,
    UNKNOWN
  };

  Building(Vector position, TYPE type, int owner = 0, int status = 0)
    : pos{position}, type{type}, owner{owner}, status{status} {}
  virtual ~Building() {}

  static Building* getFromSexp(const sexp::Value& sexp);

  virtual Vector getCapturePoint() const = 0;
  virtual bool isCapturable() const = 0;
  virtual void draw(const bool shadows, const Vector& light) const = 0;
  /// @HERE REMOVE
  // virtual bool collisionCheck(const CMC& other, float* m2) const;

  virtual sexp::Value toSexp() const = 0;
  static Building* fromSexp(const sexp::Value&);

  Vector pos;
protected:
  Color getFlagColor() const;

// private:
public:
  TYPE type;
  int owner;
  int status;
  Building::SUBTYPE subtype = SUBTYPE::UNKNOWN;
  std::vector<std::shared_ptr<BuildingBlock>> blocks;

  const static std::unordered_map<std::string, BuildingBlock::TYPE> SIMPLE_BUILDING_MAP;
  const static std::unordered_map<std::string, Building::SUBTYPE> FACTORIES_MAP;
};


#endif // BUILDING_H
