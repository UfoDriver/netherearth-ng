#include "building.h"
#include "constants.h"
#include "robot.h"
#include "ship.h"


Ship::Ship(const std::string& model, const std::string& texturesDir) :
  Shadow3DObject(model, texturesDir),
  pos(4.0, 2.0, 3.0),
  landed(false),
  op(OPS::NONE),
  op2(OPS::NONE),
  op3(OPS::NONE),
  timemoving(0)
{
  normalize(0.5f);
  makepositive();
};


bool Ship::checkCollision(const std::vector<Building>& buildings,
                          const std::vector<Robot*> robots[2])
{
  float m1[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  pos.x, pos.y, pos.z, 1};
  float m2[16] = {1, 0, 0, 0,
                  0, 1, 0, 0,
                  0, 0, 1, 0,
                  0, 0, 0, 1};

  /* Collision with buildings: */
  for (const Building& b: buildings) {
    if (((b.pos.x - pos.x) * (b.pos.x - pos.x) +
         (b.pos.y - pos.y) * (b.pos.y - pos.y) +
         (b.pos.z - pos.z) * (b.pos.z - pos.z)) < COLISION_TEST_THRESHOLD) {

      m2[12] = b.pos.x;
      m2[13] = b.pos.y;
      m2[14] = b.pos.z;

      switch(b.type) {
      case Building::TYPE::FENCE:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[5].cmc), m2)) return true;
        break;
      case Building::TYPE::WALL1:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[0].cmc), m2)) return true;
        break;
      case Building::TYPE::WALL2:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[1].cmc), m2)) return true;
        break;
      case Building::TYPE::WALL3:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[2].cmc), m2)) return true;
        break;
      case Building::TYPE::WALL4:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[3].cmc), m2)) return true;
        break;
      case Building::TYPE::WALL5:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[4].cmc), m2)) return true;
        break;
      case Building::TYPE::WALL6:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[7].cmc), m2)) return true;
        break;
      case Building::TYPE::WARBASE:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[8].cmc), m2)) return true;
        break;
      case Building::TYPE::FACTORY_ELECTRONICS:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[4].cmc), m2)) return true;
        m2[12] = b.pos.x + 0.5;
        m2[13] = b.pos.y + 0.5;
        m2[14] = b.pos.z + 1;
        if (cmc.collision_simple(m1, &(Resources::pieceTiles[0][7].cmc), m2)) return true;
        break;
      case Building::TYPE::FACTORY_NUCLEAR:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[4].cmc), m2)) return true;
        m2[12] = b.pos.x + 0.5;
        m2[13] = b.pos.y + 0.5;
        m2[14] = b.pos.z + 1;
        if (cmc.collision_simple(m1, &(Resources::pieceTiles[0][6].cmc), m2)) return true;
        break;
      case Building::TYPE::FACTORY_PHASERS:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[4].cmc), m2)) return true;
        m2[12] = b.pos.x + 0.5;
        m2[13] = b.pos.y + 0.5;
        m2[14] = b.pos.z + 1;
        if (cmc.collision_simple(m1, &(Resources::pieceTiles[0][5].cmc), m2)) return true;
        break;
      case Building::TYPE::FACTORY_MISSILES:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[4].cmc), m2)) return true;
        m2[12] = b.pos.x + 0.5;
        m2[13] = b.pos.y + 0.5;
        m2[14] = b.pos.z + 1;
        if (cmc.collision_simple(m1, &(Resources::pieceTiles[0][4].cmc), m2)) return true;
        break;
      case Building::TYPE::FACTORY_CANNONS:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[4].cmc), m2)) return true;
        m2[12] = b.pos.x + 0.5;
        m2[13] = b.pos.y + 0.5;
        m2[14] = b.pos.z + 1;
        if (cmc.collision_simple(m1, &(Resources::pieceTiles[0][3].cmc), m2)) return true;
        break;
      case Building::TYPE::FACTORY_CHASSIS:
        if (cmc.collision_simple(m1, &(Resources::buildingTiles[4].cmc), m2)) return true;
        m2[12] = b.pos.x + 0.5;
        m2[13] = b.pos.y + 0.5;
        m2[14] = b.pos.z + 1;
        if (cmc.collision_simple(m1, &(Resources::pieceTiles[0][1].cmc), m2)) return true;
        break;
      }
    }
  }

  /* Collision with the robots: */
  for(int i = 0; i < 2; i++) {
    for (Robot* r: robots[i]) {
      if (((r->pos.x - pos.x) * (r->pos.x - pos.x) +
           (r->pos.y - pos.y) * (r->pos.y - pos.y)) < COLISION_TEST_THRESHOLD) {
          m2[12] = r->pos.x;
          m2[13] = r->pos.y;
          m2[14] = r->pos.z;
          if (cmc.collision_simple(m1, &(r->cmc), m2)) return true;
        }
      }
  }

  return false;
}
