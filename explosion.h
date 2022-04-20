#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <sexp/value.hpp>

#include "drawable.h"
#include "vector.h"


class Explosion: public Drawable
{
public:
  Explosion() : step{0}, size {0} {};
  Explosion(Vector p, int sz) : pos{p}, step{0}, size {sz} {}
  Explosion(const sexp::Value& value);

  float getRadius() const;
  float getAlpha() const;

  bool cycle();
  void draw(const Vector& lightposv, bool shadows) const;

  sexp::Value toSexp() const;

  Vector pos;
  int step;
  int size;
};

#endif // EXPLOSION_H
