#ifndef MAP_H
#define MAP_H

#include <vector>

#include "building.h"
#include "bullet.h"
#include "camera.h"
#include "explosion.h"
#include "particle.h"
#include "vector.h"


class Map
{
public:
  Map(): Width(0), Height(0) {}

  std::vector<int> map;

  void resize(int const width, int const height);
  void draw(const Vector& viewp, const bool shadows, const Vector& light, const Camera& camera);

  int width() const { return Width; }
  int height() const { return Height; }
  bool loadMap(const std::string& filename);
  float maxZ(float x[2], float y[2]);
  int terrain(float x, float y);
  int worseTerrain(float x[2], float y[2]);
  std::vector<Building> buildings;
  std::vector<Bullet> bullets;
  std::vector<Explosion> explosions;
  std::vector<Particle> particles;
private:
  int Width;
  int Height;
};



#endif // MAP_H
