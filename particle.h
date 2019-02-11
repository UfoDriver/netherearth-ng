#ifndef PARTICLE_H
#define PARTICLE_H

#include "color.h"
#include "vector.h"


class Particle {
public:
  Particle();
  Particle(Vector p, Vector speed1, Vector speed2, float sz1, float sz2, const Color& color,
           float a1, float a2, int lifetime);

  Vector pos, speed1, speed2;
  float size1, size2;
  Color color;
  float a1, a2;
  int lifetime, acttime;

  void draw();
  bool cycle();
};

#endif // PARTICLE_H
