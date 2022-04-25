#include "buildingsimple.h"


BuildingSimple::BuildingSimple(const Vector& position, BuildingBlock::TYPE blockType)
  : Building(position, TYPE::SIMPLE), buildingBlockType {blockType}
{
  blocks.emplace_back(new BuildingBlock(position, blockType));
}


BuildingSimple::BuildingSimple(const sexp::Value& sexp)
  : Building({0, 0, 0}, TYPE::SIMPLE)
{
  pos = Vector(sexp::cdar(sexp));
  buildingBlockType = (BuildingBlock::TYPE)sexp::cddar(sexp).as_int();
  blocks.emplace_back(new BuildingBlock(pos, buildingBlockType, std::shared_ptr<Building>(this)));
}


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
