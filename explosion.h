#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "vector.h"

#include <sexp/value.hpp>


class Explosion {
public:
  Explosion() : step{0}, size {0} {};
  Explosion(Vector p, int sz) : pos{p}, step{0}, size {sz} {}
  Explosion(const sexp::Value& value) { fromSexp(value); };
  explicit Explosion(std::istream& in);
  bool cycle();
  float getRadius() const;
  float getAlpha() const;

  sexp::Value toSexp() const;
  bool fromSexp(const sexp::Value&);

  Vector pos;
  int step;
  int size;
};

std::ostream& operator<<(std::ostream& out, const Explosion& explosion);

#endif // EXPLOSION_H
