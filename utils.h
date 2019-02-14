#include <algorithm>
#include <iostream>

#include "building.h"
#include "bullet.h"
#include "menu.h"
#include "robot.h"
#include "statusbutton.h"


template <class T, class V>
int find_index(const T& container, const V& value)
{
  // @TODO: comparing addresses is wrong, should be fixed
  auto pos = std::find(container.cbegin(), container.cend(), value);
  if (pos != container.cend()) {
    return std::distance(container.cbegin(), pos);
  } else {
    return -1;
  }
}

template <class T>
inline std::istream& operator>>(std::istream &str, T &v) {
  unsigned int type = 0;
  if (str >> type)
    v = static_cast<T>(type);
  return str;
}
