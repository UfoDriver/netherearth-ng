#include "explosion.h"


bool Explosion::cycle()
{
  if (size == 0) step += 2;
  if (size == 1) step++;
  step++;
  return step < 128;
}


Explosion::Explosion(std::istream& in)
{
  in >> step >> size;
}

std::ostream& operator<<(std::ostream& out, const Explosion& explosion)
{
  return out << explosion.pos << explosion.step << ' ' << explosion.size << '\n';
}
