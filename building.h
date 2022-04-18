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
  enum class TYPE {SIMPLE, FACTORY, WARBASE};
  enum class SUBTYPE {ELECTRONICS,
                      NUCLEAR,
                      PHASERS,
                      MISSILES,
                      CANNONS,
                      CHASSIS,
                      UNKNOWN};

  static std::unordered_map<std::string, BuildingBlock::TYPE> simple_buildings_map;
  static std::unordered_map<std::string, Building::SUBTYPE> factories_map;

  Building(Vector position, TYPE type, int owner = 0, int status = 0)
    : pos{position}, type{type}, owner{owner}, status{status} {}
  virtual ~Building() {}

  static Building* getFromMapFile(std::istream& inFile);
  static Building* getFromSexp(const sexp::Value& sexp);


  virtual Vector getCapturePoint() const = 0;
  virtual bool isCapturable() const = 0;
  virtual void draw(const bool shadows, const Vector& light) const;
  virtual bool collisionCheck(const CMC& other, float* m2) const;

  virtual sexp::Value toSexp() const;
  static Building* fromSexp(const sexp::Value&);

  Vector pos;
protected:
  Color getFlagColor() const;

// private:
public:
  TYPE type;
  Building::SUBTYPE subtype = SUBTYPE::UNKNOWN;
  int owner;
  int status;
  std::vector<BuildingBlock> blocks;
};


#endif // BUILDING_H
