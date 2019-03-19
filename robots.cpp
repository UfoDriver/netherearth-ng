#include <algorithm>

#include "robots.h"


int Robots::getRobotCount(unsigned short int owner)
{
  return std::count_if(cbegin(), cend(),
                       [owner](auto r) {
                         return r->getOwner() == owner;
                       });
}
