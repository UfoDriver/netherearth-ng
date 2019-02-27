#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

class Camera: public Vector
{
public:
  Camera(float x, float y, float z, float zoom): Vector(x, y, z), zoom(zoom), viewport(0, 0, 0) {}
  void cycle(unsigned char* keyboard);
  float zoom;
  Vector viewport;
  void drawViewport();

  void updateViewportForShip(const Vector& shipPosition, int mapWidth, int mapHeight);
  bool canSee(const Vector& position) const;

private:
  float minx = -8;
  float miny = -8;
  float maxx = 8;
  float maxy = 8;
};

#endif // CAMERA_H
