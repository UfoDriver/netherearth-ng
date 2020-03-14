#include <GL/gl.h>

#include "buildingblock.h"
#include "constants.h"
#include "resources.h"


BuildingBlock::BuildingBlock(Vector position, TYPE type) :
    pos{position}, type{type}
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


BuildingBlock::BuildingBlock(std::istream& in)
{
  int type_;
  int owner, status;
  in >> type_ >> owner >> status >> pos;
  type = BuildingBlock::TYPE(type_);
}


std::ostream& operator<<(std::ostream& out, const BuildingBlock& building)
{
  return out << int(building.type) << ' ' << '?' << ' ' << '?' << '\n'
             << building.pos;
}


const std::vector<std::unique_ptr<BuildingBlock>> BuildingBlock::readMapFile(std::istream& inFile)
{
  std::vector<std::unique_ptr<BuildingBlock>> acc;
  std::string buffer;
  float x, y;
  inFile >> buffer >> x >> y;

  if (buffer == "fence") {
    acc.emplace_back(new BuildingBlock(Vector(x, y, 0), BuildingBlock::TYPE::FENCE));
  } else if (buffer == "wall1") {
    acc.emplace_back(new BuildingBlock(Vector(x, y, 0), BuildingBlock::TYPE::WALL1));
  } else if (buffer == "wall2") {
    acc.emplace_back(new BuildingBlock(Vector(x, y, 0), BuildingBlock::TYPE::WALL2));
  } else if (buffer == "wall3") {
    acc.emplace_back(new BuildingBlock(Vector(x, y, 0), BuildingBlock::TYPE::WALL3));
  } else if (buffer == "wall4") {
    acc.emplace_back(new BuildingBlock(Vector(x, y, 0), BuildingBlock::TYPE::WALL4));
  } else if (buffer == "wall6") {
    acc.emplace_back(new BuildingBlock(Vector(x, y, 0), BuildingBlock::TYPE::WALL6));
  } else if (buffer == "factory") {
    BuildingBlock::TYPE obj[4] = {BuildingBlock::TYPE::WALL4,
                                  BuildingBlock::TYPE::WALL4,
                                  BuildingBlock::TYPE::WALL2,
                                  BuildingBlock::TYPE::WALL2};
    float xo[4] = {0, 0, 1, 1};
    float yo[4] = {0, 2, 0, 2};
    std::string buffer2;
    inFile >> buffer2;
    for (int i = 0; i < 4; i++) {
      acc.emplace_back(new BuildingBlock(Vector(x + xo[i], y + yo[i], 0), obj[i]));
    }

    BuildingBlock* b = new BuildingBlock(Vector(x, y + 1, 0), BuildingBlock::TYPE::FACTORY_ELECTRONICS);
    if (buffer2 == "electronics") b->type = BuildingBlock::TYPE::FACTORY_ELECTRONICS;
    if (buffer2 == "nuclear") b->type = BuildingBlock::TYPE::FACTORY_NUCLEAR;
    if (buffer2 == "phasers") b->type = BuildingBlock::TYPE::FACTORY_PHASERS;
    if (buffer2 == "missiles") b->type = BuildingBlock::TYPE::FACTORY_MISSILES;
    if (buffer2 == "cannons") b->type = BuildingBlock::TYPE::FACTORY_CANNONS;
    if (buffer2 == "chassis") b->type = BuildingBlock::TYPE::FACTORY_CHASSIS;
    acc.emplace_back(b);
  } else if (buffer == "warbase") {
    BuildingBlock::TYPE obj[15] = {BuildingBlock::TYPE::WALL4,
                                   BuildingBlock::TYPE::WALL5,
                                   BuildingBlock::TYPE::WALL4,
                                   BuildingBlock::TYPE::WALL1,
                                   BuildingBlock::TYPE::WALL1,
                                   BuildingBlock::TYPE::WALL2,
                                   BuildingBlock::TYPE::WALL4,
                                   BuildingBlock::TYPE::WARBASE,
                                   BuildingBlock::TYPE::WALL2,
                                   BuildingBlock::TYPE::WALL4,
                                   BuildingBlock::TYPE::WALL1,
                                   BuildingBlock::TYPE::WALL1,
                                   BuildingBlock::TYPE::WALL2,
                                   BuildingBlock::TYPE::WALL4,
                                   BuildingBlock::TYPE::WALL5};
    float xo[15] = {0.5, 1.5,
                    0, 1, 2, 3,
                    0.5, 1.5, 2.5,
                    0, 1, 2, 3,
                    0.5, 1.5};
    float yo[15] = {0, 0,
                    1, 1, 1, 1,
                    2, 2, 2,
                    3, 3, 3, 3,
                    4, 4};
    int owner = 0;
    inFile >> owner;
    for(int i = 0; i < 15; i++) {
      acc.emplace_back(new BuildingBlock(Vector(x + xo[i], y + yo[i], 0), obj[i]));
    }
  }
  return acc;
}


void BuildingBlock::draw(const bool shadows, const Vector& light) const
{
  if (!shadows) {
    tile.draw(Color(0.2f, 0.2f, 0.2f));
  } else {
    glTranslatef(0, 0, 0.05f);
    tile.drawShadow(Color(0, 0, 0, 0.5));
  }
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
  return tile.cmc;
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
