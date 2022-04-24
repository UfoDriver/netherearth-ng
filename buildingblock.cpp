#include <GL/gl.h>
#include <memory>

#include "building.h"
#include "buildingblock.h"
#include "buildingfactory.h"
#include "constants.h"
#include "resources.h"


BuildingBlock::BuildingBlock(Vector position, TYPE type, std::shared_ptr<Building> building)
  : pos{position}, type{type}, building{building}
{
  switch (type) {
  case TYPE::FENCE:
    tile = Resources::buildingTiles[5];
    break;
  case TYPE::WALL1:
    tile = Resources::buildingTiles[0];
    break;
  case TYPE::WALL2:
    tile = Resources::buildingTiles[1];
    break;
  case TYPE::WALL3:
    tile = Resources::buildingTiles[2];
    break;
  case TYPE::WALL4:
    tile = Resources::buildingTiles[3];
    break;
  case TYPE::WALL5:
  case TYPE::FACTORY_CANNONS:
  case TYPE::FACTORY_CHASSIS:
  case TYPE::FACTORY_ELECTRONICS:
  case TYPE::FACTORY_MISSILES:
  case TYPE::FACTORY_NUCLEAR:
  case TYPE::FACTORY_PHASERS:
    tile = Resources::buildingTiles[4];
    break;
  case TYPE::WALL6:
    tile = Resources::buildingTiles[7];
    break;
  case TYPE::WARBASE:
    tile = Resources::buildingTiles[8];
    break;
  }
}


void BuildingBlock::draw(const bool shadows, const Vector& light) const
{
  glPushMatrix();
  glTranslatef(float(pos.x), float(pos.y), float(pos.z));

  if (!shadows) {
    tile.draw(Color(0.2f, 0.2f, 0.2f));
  } else {
    glTranslatef(0, 0, 0.05f);
    tile.drawShadow(Color(0, 0, 0, 0.5));
  }

  if (isMainBuildingBlock()) {
    building->draw(shadows, light);
    // getCMC().draw(Color(255, 0, 0));
  } else {
    // getCMC().draw(Color(0, 255, 0));
  }
  glPopMatrix();
}


bool BuildingBlock::collidesWith(const Vector& position, const CMC& cmc) const
{
  float m1[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  pos.x, pos.y, pos.z, 1};
  float m2[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  position.x, position.y, position.z, 1};


  if (getCMC().collision_simple(m1, cmc, m2)) return true;

  if (type >= TYPE::FACTORY_ELECTRONICS && type <= TYPE::FACTORY_CHASSIS) {
    m1[12] = pos.x + 0.5;
    m1[13] = pos.y + 0.5;
    m1[14] = pos.z + 1;
    return getExtraCMC().collision_simple(m1, cmc, m2);
  }

  return false;
}


CMC BuildingBlock::getCMC() const
{
  if (isMainBuildingBlock() && building->subtype != Building::SUBTYPE::UNKNOWN) {
    CMC ret(tile.cmc);
    CMC typeTileCNC {(dynamic_cast<BuildingFactory*>(building.get())->typeTile.cmc)};
    // @TODO: cnc expand does not work or I don't know how to use it right
    ret.z[1] += typeTileCNC.z[1] - typeTileCNC.z[0];
    return ret;
  } else {
    return tile.cmc;
  }
}


CMC BuildingBlock::getExtraCMC() const
{
  switch (type) {
  case TYPE::FACTORY_ELECTRONICS:
    return Resources::pieceTiles[0][7].cmc;
    break;
  case TYPE::FACTORY_NUCLEAR:
    return Resources::pieceTiles[0][6].cmc;
    break;
  case TYPE::FACTORY_PHASERS:
    return Resources::pieceTiles[0][5].cmc;
    break;
  case TYPE::FACTORY_MISSILES:
    return Resources::pieceTiles[0][4].cmc;
    break;
  case TYPE::FACTORY_CANNONS:
    return Resources::pieceTiles[0][3].cmc;
    break;
  case TYPE::FACTORY_CHASSIS:
    return Resources::pieceTiles[0][1].cmc;
    break;
  default:
    return CMC();
  }
}


bool BuildingBlock::collisionCheck(const CMC& other, float* m2) const
{
  float m1[16] = {
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    pos.x, pos.y, pos.z, 1
  };

  return getCMC().collision_simple(m1, other, m2);
}


bool BuildingBlock::isMainBuildingBlock() const
{
  return building && pos == building->pos;
}
