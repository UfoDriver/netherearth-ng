#ifndef MAP_H
#define MAP_H

#include <memory>
#include <vector>
#include <sexp/value.hpp>

#include "building.h"
#include "bullet.h"
#include "camera.h"
#include "explosion.h"
#include "particle.h"
#include "robots.h"
#include "vector.h"

class NETHER;
class Robot;

/*
  Map has terrain, building blocks and buildings. It has dimension and know some terrain details.
  It reacts on big explosions (nuclear) that change the map.
*/
class Map
{
public:
  enum TERRAIN {
    T_GRASS,
    T_SAND,
    T_MOUNTAINS,
    T_HOLE,
    T_BUILDING,
    T_SHIP,
    T_ROBOT,
    T_EROBOT,
    T_OUT
  };

  explicit Map(NETHER* nether) : width{0}, height{0}, nether{nether} {}

  std::vector<int> map;

  void resize(int const width, int const height);
  void draw(const Camera& camera, const Vector& light, const bool shadows);

  int getWidth() const { return width; }
  int getHeight() const { return height; }
  float getMaxZ(float x[2], float y[2]) const;
  float getMaxZ(const Vector& position) const;
  TERRAIN getTerrain(float x, float y);

  bool loadMap(const std::string& filename);

  int worseTerrain(const Vector& position);
  void nuclearExplosionAt(const Vector& position);

  std::vector<std::shared_ptr<Building>> buildings;
  std::vector<std::shared_ptr<BuildingBlock>> buildingBlocks;

  bool cycle();

  sexp::Value toSexp() const;
  bool fromSexp(const sexp::Value&);

private:
  void processMapSectionSexp(const sexp::Value&);

  int width;
  int height;
  NETHER* nether;
};


#endif // MAP_H
