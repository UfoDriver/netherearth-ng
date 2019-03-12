#ifndef LIGHT_H
#define LIGHT_H

#include "vector.h"


class Light
{
public:
  Light() = default;
  Light(float x, float y, float z, float type): pos {x, y, z, type} {}
  void set(float x, float y, float z, float type) {
    pos[0] = x;
    pos[1] = y;
    pos[2] = z;
    pos[3] = type;};
  Vector asVector() const { return Vector(pos[0], pos[1], pos[2]); }
  const float* raw() const { return pos; }

private:
  friend std::istream& operator>>(std::istream& in, Light& light);
  float pos[4] = {0, 0, 0, 0};
};

std::ostream& operator<<(std::ostream& out, const Light& light);
std::istream& operator>>(std::istream& in, Light& light);


#endif // LIGHT_H
