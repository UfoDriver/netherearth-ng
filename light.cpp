#include <iomanip>
#include <iostream>

#include "light.h"


std::ostream& operator<<(std::ostream& out, const Light& light)
{
  return out << std::setw(8) << light.raw()[0] << ' '
             << std::setw(8) << light.raw()[1] << ' '
             << std::setw(8) << light.raw()[2] << ' '
             << std::setw(8) << light.raw()[3] << '\n'
             << light.asVector();
}


std::istream& operator>>(std::istream& in, Light& light)
{
  Vector v;
  return in >> light.pos[0] >> light.pos[1] >> light.pos[2] >> light.pos[3] >> v;
}
