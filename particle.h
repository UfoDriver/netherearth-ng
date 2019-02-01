#ifndef PARTICLE_H
#define PARTICLE_H

#include "vector.h"


class Particle {
public:
  Particle();
  Particle(Vector p, Vector speed1, Vector speed2, float sz1, float sz2, float r, float g, float b,
           float a1, float a2, int lifetime);

  Vector pos, speed1, speed2;
  float size1, size2;
  float r, g, b;
  float a1, a2;
  int lifetime, acttime;

  void draw();
  bool cycle();
};

#endif // PARTICLE_H
