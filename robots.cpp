#include <algorithm>

#include "robot.h"
#include "robots.h"


int Robots::getRobotCount(unsigned short int owner)
{
  return std::count_if(cbegin(), cend(),
                       [owner](auto r) {
                         return r->getOwner() == owner;
                       });
}


int Robots::findIndex(std::shared_ptr<Robot> robot)
{
  auto pos = std::find(cbegin(), cend(), robot);
  if (pos != cend()) {
    return std::distance(cbegin(), pos);
  } else {
    return -1;
  }
}


void Robots::findAndDestroy(std::shared_ptr<Robot> robot)
{
  erase(std::remove(begin(), end(), robot), end());
}


PlayerOnly::iterator::iterator(unsigned short player,
                               std::vector<std::shared_ptr<Robot>>::iterator begin_,
                               std::vector<std::shared_ptr<Robot>>::iterator end_)
  : player{player}, internal {begin_}, end_ {end_}
{
  do {
    if ((*internal)->getOwner() == player) {
      return;
    }
    ++internal;
  } while (internal != end_);
}


PlayerOnly::iterator& PlayerOnly::iterator::operator++() {
  do {
    ++internal;
  } while (internal != end_ and (*internal)->getOwner() != player);
  return *this;
}

bool PlayerOnly::iterator::operator!=(const iterator& other)
{
  return internal != other.internal;
}

Robot* PlayerOnly::iterator::operator*()
{
  return (*internal).get();
}
