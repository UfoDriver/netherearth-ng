#include "buildingsimple.h"


sexp::Value BuildingSimple::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("block"),
    pos.toSexp(),
    sexp::Value::integer((int)type)
  );
}


const std::vector<BuildingBlock> BuildingSimple::getTemplate() const
{
  return std::vector<BuildingBlock> {
    {{0, 0, 0}, buildingBlockType},
  };
}
