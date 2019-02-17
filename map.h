#ifndef MAP_H
#define MAP_H

#include <vector>
#include <GL/gl.h>

#include "vector.h"

class Building;


class Map
{
public:
  Map(): Width(0), Height(0) {}

  std::vector<int> map;

  void resize(int width, int height);
  void draw(const Vector& viewp, bool shadows);

  int width() const { return Width; }
  int height() const { return Height; }
  bool loadMap(const std::string& filename, std::vector<Building>& buildings);
  float maxZ(float x[2], float y[2]);
  int terrain(float x, float y);
  int worseTerrain(float x[2], float y[2]);
private:
  int Width;
  int Height;
};



#endif // MAP_H
