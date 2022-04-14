#include <GL/gl.h>
#include <algorithm>

#include "buildingwarbase.h"
#include "constants.h"
#include "resources.h"


BuildingWarbase::BuildingWarbase(const Vector &position) :
  Building(position, TYPE::WARBASE)
{
  // @TODO temporary position correction before maps are updated (if ever)
  pos = pos + Vector(1.5, 2, 0);

  // @TODO somehow out of the method it makes sigfault
  const std::vector<BuildingBlock> WARBASE_TEMPLATE
    {
     {{-1, -2, 0}, BuildingBlock::TYPE::WALL4},
     {{0, -2, 0}, BuildingBlock::TYPE::WALL5},
     {{-1.5, -1, 0}, BuildingBlock::TYPE::WALL4},
     {{-0.5, -1, 0}, BuildingBlock::TYPE::WALL1},
     {{0.5, -1, 0}, BuildingBlock::TYPE::WALL1},
     {{1.5, -1, 0}, BuildingBlock::TYPE::WALL2},
     {{-1, 0, 0}, BuildingBlock::TYPE::WALL4},
     {{0, 0, 0}, BuildingBlock::TYPE::WARBASE},
     {{1, 0, 0}, BuildingBlock::TYPE::WALL2},
     {{-1.5, 1, 0}, BuildingBlock::TYPE::WALL4},
     {{-0.5, 1, 0}, BuildingBlock::TYPE::WALL1},
     {{0.5, 1, 0}, BuildingBlock::TYPE::WALL1},
     {{1.5, 1, 0}, BuildingBlock::TYPE::WALL2},
     {{-1, 2, 0}, BuildingBlock::TYPE::WALL4},
     {{0, 2, 0}, BuildingBlock::TYPE::WALL5}};

  std::transform(WARBASE_TEMPLATE.cbegin(), WARBASE_TEMPLATE.cend(), std::back_inserter(blocks),
                 [this](const BuildingBlock& block) {
                   BuildingBlock b(block);
                   b.pos = b.pos + pos;
                   return b;
                 });
  flagTile = Resources::buildingTiles[6];
}


void BuildingWarbase::draw(const bool shadows, const Vector& light) const
{
  Building::draw(shadows, light);

  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);

  if (shadows) {
    glTranslatef(float(-light.x), float(-light.y) - 2, 0.05f);
    flagTile.drawShadow(Color(0, 0, 0, 0.5));
  } else {
    glTranslatef(0, -2, 1);
    flagTile.draw(getFlagColor());
  }

  glPopMatrix();
}


sexp::Value BuildingWarbase::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("warbase"),
    pos.toSexp(),
    sexp::Value::integer(owner),
    sexp::Value::integer(status)
  );
}
