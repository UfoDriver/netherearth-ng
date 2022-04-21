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
  explicit Map(NETHER* nether) : width{0}, height{0}, nether{nether} {}

  std::vector<int> map;

  void resize(int const width, int const height);
  void draw(const Camera& camera, const Vector& light, const bool shadows);

  int getWidth() const { return width; }
  int getHeight() const { return height; }
  float getMaxZ(float x[2], float y[2]) const;
  float getMaxZ(const Vector& position) const;
  int getTerrain(float x, float y);

  bool loadMap(const std::string& filename);

  int worseTerrain(const Vector& position);
  void nuclearExplosionAt(Robot* robot, const Vector& position);

  std::vector<std::unique_ptr<Building>> buildings;
  std::vector<std::shared_ptr<BuildingBlock>> buidlingBlocks;

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
