#include <iomanip>
#include <iostream>
#include <vector>

#include "light.h"
#include "sexp/value.hpp"


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


sexp::Value Light::toSexp() const
{
  return sexp::Value::list(sexp::Value::symbol("light"),
                           sexp::Value::real(pos[0]),
                           sexp::Value::real(pos[1]),
                           sexp::Value::real(pos[2]),
                           sexp::Value::real(pos[3]));
}
