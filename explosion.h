#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "vector.h"


class Explosion {
public:
  Explosion(): step(0), size(0) {};
  Explosion(Vector p, int sz): pos(p), step(0), size(sz) {};
  explicit Explosion(std::istream& in);

  Vector pos;
  int step;
  int size;
};

std::ostream& operator<<(std::ostream& out, const Explosion& explosion);

#endif // EXPLOSION_H
