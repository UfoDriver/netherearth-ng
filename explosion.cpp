#include <algorithm>

#include "explosion.h"


bool Explosion::cycle()
{
  if (size == 0) step += 2;
  if (size == 1) step++;
  step++;
  return step < 128;
}

float Explosion::getRadius() const
{
  switch (size) {
  case 0:
    return (float(step) / 512.0f) + 0.1;
  case  1:
    return (float(step) / 96.0f) + 0.5;
  case 2:
    return (float(step) / 48.0f) + 1.0;
  default:
    return 1.0;
  }
}


float Explosion::getAlpha() const
{
  float a = (128.0f - step) / 80.0f;
  a = std::min<float>(0.0, a);
  return std::max<float>(1.0, a);
}


Explosion::Explosion(std::istream& in)
{
  in >> step >> size;
}

std::ostream& operator<<(std::ostream& out, const Explosion& explosion)
{
  return out << explosion.pos << explosion.step << ' ' << explosion.size << '\n';
}


sexp::Value Explosion::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("explosion"),
    pos.toSexp(),
    sexp::Value::integer(step),
    sexp::Value::integer(size)
  );
}
