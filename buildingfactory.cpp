#include <GL/freeglut_std.h>
#include <GL/gl.h>
#include <algorithm>

#include "constants.h"
#include "buildingfactory.h"
#include "resources.h"


BuildingFactory::BuildingFactory(const Vector& position, SUBTYPE subtype)
  : Building(position, TYPE::FACTORY)
{
  this->subtype = subtype;

  // @TODO somehow out of the method it makes sigfault
  const std::vector<BuildingBlock> FACTORY_TEMPLATE
    {
     {{0, -1, 0}, BuildingBlock::TYPE::WALL4},
     {{0, 1, 0}, BuildingBlock::TYPE::WALL4},
     {{0, 0, 0}, BuildingBlock::TYPE::WALL5},
     {{1, -1, 0}, BuildingBlock::TYPE::WALL2},
     {{1, 1, 0}, BuildingBlock::TYPE::WALL2}};

  std::transform(
    FACTORY_TEMPLATE.cbegin(), FACTORY_TEMPLATE.cend(), std::back_inserter(blocks),
    [this](const BuildingBlock& block) {
      std::shared_ptr<BuildingBlock> b {new BuildingBlock(block)};
      b->building = std::shared_ptr<Building>(this);
      b->pos = b->pos + pos;
      return b;
    }
  );

  switch (subtype) {
  case Building::SUBTYPE::ELECTRONICS:
    typeTile = Resources::pieceTiles[0][7];
    break;
  case Building::SUBTYPE::NUCLEAR:
    typeTile = Resources::pieceTiles[0][6];
    break;
  case Building::SUBTYPE::PHASERS:
    typeTile = Resources::pieceTiles[0][5];
    break;
  case Building::SUBTYPE::MISSILES:
    typeTile = Resources::pieceTiles[0][4];
    break;
  case Building::SUBTYPE::CANNONS:
    typeTile = Resources::pieceTiles[0][3];
    break;
  case Building::SUBTYPE::CHASSIS:
    typeTile = Resources::pieceTiles[0][1];
    break;
  case Building::SUBTYPE::UNKNOWN:
    break;
  }

  flagTile = Resources::buildingTiles[6];
  // TODO: temporary mutable for building factory tile
  typeTile.textured = false;
}


void BuildingFactory::draw(const bool shadows, const Vector& light) const
{
  glPushMatrix();

  if (!shadows) {
    glPushMatrix();
    glTranslatef(0.5, 0.5, 1);
    typeTile.draw(Color(0.8f, 0.8f, 0.8f));
    glPopMatrix();
  } else {
    glPushMatrix();
    glTranslatef(0, 0, 0.05f);
    glTranslatef(float(-light.x), float(-light.y), 0);
    typeTile.drawShadow(0, light, Color(0, 0, 0, 0.5));
    glPopMatrix();
  }

  if (owner) {
    if (shadows) {
      glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
      flagTile.drawShadow(Color(0, 0, 0, 0.5));
    } else {
      glTranslatef(0, -1, 1);
      flagTile.draw(getFlagColor());
    }
  }

  glPopMatrix();
}


sexp::Value BuildingFactory::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("factory"),
    sexp::Value::integer((int)subtype),
    pos.toSexp(),
    sexp::Value::integer(owner),
    sexp::Value::integer(status)
  );
}
