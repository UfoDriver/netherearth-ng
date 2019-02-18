#ifndef CAMERA_H
#define CAMERA_H

#include "vector.h"

class Camera: public Vector
{
public:
  Camera(float x, float y, float z, float zoom): Vector(x, y, z), zoom(zoom) {}
  float zoom;
};

#endif // CAMERA_H
