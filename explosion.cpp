#include "explosion.h"


Explosion::Explosion(std::istream& in)
{
  in >> step >> size;
}

std::ostream& operator<<(std::ostream& out, const Explosion& explosion)
{
  return out << explosion.pos << explosion.step << ' ' << explosion.size << '\n';
}
