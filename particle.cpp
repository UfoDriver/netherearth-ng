#include "particle.h"


Particle::Particle():
  size1(0), size2(0), r(0), g(0), b(0), a1(0), a2(0),
  lifetime(0), acttime(0)
{
}


Particle::Particle(Vector p, Vector spd1, Vector spd2, float sz1, float sz2, float rp, float gp,
                   float bp, float a1p, float a2p, int lt):
  pos(p), speed1(spd1), speed2(spd2), size1(sz1), size2(sz2), r(rp), g(gp), b(bp), a1(a1p), a2(a2p),
  lifetime(lt), acttime(0)
{
}
