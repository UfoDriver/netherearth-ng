#include <GL/gl.h>

#include "building.h"
#include "constants.h"
#include "resources.h"
#include "utils.h"


Building::Building(std::istream& in)
{
  int type_;
  in >> type_ >> owner >> status >> pos;
  type = Building::TYPE(type_);
}


std::ostream& operator<<(std::ostream& out, const Building& building)
{
  return out << int(building.type) << ' ' << building.owner << ' ' << building.status << '\n'
             << building.pos;
}


const std::vector<Building> Building::readMapFile(std::istream& inFile)
{
  std::vector<Building> acc;
  std::string buffer;
  float x, y;
  inFile >> buffer >> x >> y;

  if (buffer == "fence") {
    acc.emplace_back(Vector(x, y, 0), Building::TYPE::FENCE);
  } else if (buffer == "wall1") {
    acc.emplace_back(Vector(x, y, 0), Building::TYPE::WALL1);
  } else if (buffer == "wall2") {
    acc.emplace_back(Vector(x, y, 0), Building::TYPE::WALL2);
  } else if (buffer == "wall3") {
    acc.emplace_back(Vector(x, y, 0), Building::TYPE::WALL3);
  } else if (buffer == "wall4") {
    acc.emplace_back(Vector(x, y, 0), Building::TYPE::WALL4);
  } else if (buffer == "wall6") {
    acc.emplace_back(Vector(x, y, 0), Building::TYPE::WALL6);
  } else if (buffer == "factory") {
    Building::TYPE obj[4] = {Building::TYPE::WALL4,
                             Building::TYPE::WALL4,
                             Building::TYPE::WALL2,
                             Building::TYPE::WALL2};
    float xo[4] = {0, 0, 1, 1};
    float yo[4] = {0, 2, 0, 2};
    std::string buffer2;
    inFile >> buffer2;
    for (int i = 0; i < 4; i++) {
      acc.emplace_back(Vector(x + xo[i], y + yo[i], 0), obj[i], 0, 0);
    }

    Building b(Vector(x, y + 1, 0), Building::TYPE::FACTORY_ELECTRONICS);
    if (buffer2 == "electronics") b.type = Building::TYPE::FACTORY_ELECTRONICS;
    if (buffer2 == "nuclear") b.type = Building::TYPE::FACTORY_NUCLEAR;
    if (buffer2 == "phasers") b.type = Building::TYPE::FACTORY_PHASERS;
    if (buffer2 == "missiles") b.type = Building::TYPE::FACTORY_MISSILES;
    if (buffer2 == "cannons") b.type = Building::TYPE::FACTORY_CANNONS;
    if (buffer2 == "chassis") b.type = Building::TYPE::FACTORY_CHASSIS;
    acc.push_back(b);
  } else if (buffer == "warbase") {
    Building::TYPE obj[15] = {Building::TYPE::WALL4,
                              Building::TYPE::WALL5,
                              Building::TYPE::WALL4,
                              Building::TYPE::WALL1,
                              Building::TYPE::WALL1,
                              Building::TYPE::WALL2,
                              Building::TYPE::WALL4,
                              Building::TYPE::WARBASE,
                              Building::TYPE::WALL2,
                              Building::TYPE::WALL4,
                              Building::TYPE::WALL1,
                              Building::TYPE::WALL1,
                              Building::TYPE::WALL2,
                              Building::TYPE::WALL4,
                              Building::TYPE::WALL5};
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
      acc.emplace_back(Vector(x + xo[i], y + yo[i], 0), obj[i], owner, 0);
    }
  }
  return acc;
}


void Building::draw(const bool shadows, const int detaillevel, const Vector& light) const
{
  // @TODO: giant switch should be replaced by inheritance
  switch(type) {
  case TYPE::FENCE:
  if (!shadows) {
    if (detaillevel >= 2)
      Resources::buildingTiles[5].draw(Color(0.2f, 0.2f, 0.2f));
    else
      Resources::buildingTiles[5].draw_notexture(Color(0.2f, 0.2f, 0.2f));
  } else {
    glTranslatef(0, 0, 0.05f);
    Resources::buildingTiles[5].DrawShadow(Color(0, 0, 0, 0.5));
  }
  break;
  case TYPE::WALL1:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[0].draw(Color(0.5, 0.5, 0.5));
      else
        Resources::buildingTiles[0].draw_notexture(Color(0.5, 0.5, 0.5));
    } else {
      glTranslatef(0,0,0.05f);
      Resources::buildingTiles[0].DrawShadow(Color(0, 0, 0, 0.5));
    }
    break;
  case TYPE::WALL2:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[1].draw(Color(0.5, 0.5, 0.5));
      else
        Resources::buildingTiles[1].draw_notexture(Color(0.5, 0.5, 0.5));
    } else {
      glTranslatef(0,0,0.05f);
      Resources::buildingTiles[1].DrawShadow(Color(0, 0, 0, 0.5));
    }
    break;
  case TYPE::WALL3:if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[2].draw(Color(0.3f, 0.3f, 0.3f));
      else
        Resources::buildingTiles[2].draw_notexture(Color(0.5, 0.5, 0.5));
    } else {
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[2].DrawShadow(Color(0, 0, 0, 0.5));
    }
    break;
  case TYPE::WALL4:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[3].draw(Color(0.5, 0.5, 0.5));
      else Resources::buildingTiles[3].draw_notexture(Color(0.5, 0.5, 0.5));
    } else {
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[3].DrawShadow(Color(0, 0, 0, 0.5));
    }
    break;
  case TYPE::WALL5:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
      else
        Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
    } else {
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
    }
    break;
  case TYPE::WALL6:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[7].draw(Color(0.3f, 0.3f, 0.3f));
      else Resources::buildingTiles[7].draw_notexture(Color(0.5, 0.5, 0.5));
    } else {
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[7].DrawShadow(Color(0, 0, 0, 0.5));
    }
    break;
  case TYPE::FACTORY_ELECTRONICS:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
      else
        Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
      glPushMatrix();
      glTranslatef(0.5, 0.5, 1);
      Resources::pieceTiles[0][7].draw_notexture(Color(0.8f, 0.8f, 0.8f));
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(0,0,0.05f);
      Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
      glTranslatef(float(-light.x), float(-light.y), 0);
      Resources::pieceTiles[0][7].drawShadow(0, light, Color(0, 0, 0, 0.5));
      glPopMatrix();
    }
    if (owner == 1) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER1_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    if (owner == 2) {
      if (!shadows) {
        glTranslatef(0,-1,1);
        Resources::buildingTiles[6].draw(PLAYER2_COLOR);
      } else {
        glTranslatef(float(-light.x),float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    break;
  case TYPE::FACTORY_NUCLEAR:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
      else
        Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
      glPushMatrix();
      glTranslatef(0.5, 0.5, 1);
      Resources::pieceTiles[0][6].draw_notexture(Color(0.8f, 0.8f, 0.8f));
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
      glTranslatef(float(-light.x), float(-light.y), 0);
      Resources::pieceTiles[0][6].drawShadow(0, light, Color(0, 0, 0, 0.5));
      glPopMatrix();
    }

    if (owner == 1) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER1_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    if (owner == 2) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER2_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    break;
  case TYPE::FACTORY_PHASERS:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
      else
        Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
      glPushMatrix();
      glTranslatef(0.5, 0.5, 1);
      Resources::pieceTiles[0][5].draw_notexture(Color(0.8f, 0.8f, 0.8f));
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
      glTranslatef(float(-light.x), float(-light.y), 0);
      Resources::pieceTiles[0][5].drawShadow(0, light, Color(0, 0, 0, 0.5));
      glPopMatrix();
    }
    if (owner == 1) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER1_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    if (owner == 2) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER2_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    break;
  case TYPE::FACTORY_MISSILES:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
      else
        Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
      glPushMatrix();
      glTranslatef(0.5, 0.5, 1);
      Resources::pieceTiles[0][4].draw_notexture(Color(0.8f, 0.8f, 0.8f));
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
      glTranslatef(float(-light.x), float(-light.y), 0);
      Resources::pieceTiles[0][4].drawShadow(0, light, Color(0, 0, 0, 0.5));
      glPopMatrix();
    }
    if (owner == 1) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER1_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    if (owner == 2) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER2_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    break;
  case TYPE::FACTORY_CANNONS:
    if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
      else
        Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
      glPushMatrix();
      glTranslatef(0.5, 0.5, 1);
      Resources::pieceTiles[0][3].draw_notexture(Color(0.8f, 0.8f, 0.8f));
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
      glTranslatef(float(-light.x), float(-light.y), 0);
      Resources::pieceTiles[0][3].drawShadow(0, light, Color(0, 0, 0, 0.5));
      glPopMatrix();
    }

    if (owner == 1) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER1_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    if (owner == 2) {
      if (!shadows) {
        glTranslatef(0,-1,1);
        Resources::buildingTiles[6].draw(PLAYER2_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    break;
  case TYPE::FACTORY_CHASSIS:
    if (!shadows) {
      if (detaillevel>=2)
        Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
      else Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
      glPushMatrix();
      glTranslatef(0.5, 0.5, 1);
      Resources::pieceTiles[0][1].draw_notexture(Color(0.8f, 0.8f, 0.8f));
      glPopMatrix();
    } else {
      glPushMatrix();
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
      glTranslatef(float(-light.x), float(-light.y), 0);
      Resources::pieceTiles[0][1].drawShadow(0, light, Color(0, 0, 0, 0.5));
      glPopMatrix();
    }
    if (owner == 1) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER1_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y)-1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    if (owner == 2) {
      if (!shadows) {
        glTranslatef(0, -1, 1);
        Resources::buildingTiles[6].draw(PLAYER2_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    break;

  case TYPE::WARBASE:if (!shadows) {
      if (detaillevel >= 2)
        Resources::buildingTiles[8].draw(Color(0.5, 0.5, 0.5));
      else
        Resources::buildingTiles[8].draw_notexture(Color(0.5, 0.5, 0.5));
    } else {
      glTranslatef(0, 0, 0.05f);
      Resources::buildingTiles[8].DrawShadow(Color(0, 0, 0, 0.5));
    }
    if (owner == 1) {
      if (!shadows) {
        glTranslatef(0, -2, 1);
        Resources::buildingTiles[6].draw(PLAYER1_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 2, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    if (owner == 2) {
      if (!shadows) {
        glTranslatef(0, -2, 1);
        Resources::buildingTiles[6].draw(PLAYER2_COLOR);
      } else {
        glTranslatef(float(-light.x), float(-light.y) - 2, 0.05f);
        Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
      }
    }
    break;
  }
}
