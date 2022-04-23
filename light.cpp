#include <sexp/util.hpp>
#include <sexp/value.hpp>

#include "light.h"


sexp::Value Light::toSexp() const
{
  return sexp::Value::list(sexp::Value::symbol("light"),
                           sexp::Value::real(pos[0]),
                           sexp::Value::real(pos[1]),
                           sexp::Value::real(pos[2]),
                           sexp::Value::real(pos[3]));
}


bool Light::fromSexp(const sexp::Value& value)
{
  pos[0] = sexp::cdar(value).as_float();
  pos[1] = sexp::cddar(value).as_float();
  pos[2] = sexp::cdddar(value).as_float();
  pos[3] = sexp::cdddar(value.get_cdr()).as_float();

  return true;
}
