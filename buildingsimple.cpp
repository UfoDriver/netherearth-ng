#include "buildingsimple.h"


sexp::Value BuildingSimple::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("block"),
    pos.toSexp()
  );
}
