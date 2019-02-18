#include <algorithm>

#include "map.h"
#include "nether.h"
#include "resources.h"
#include "utils.h"

extern int detaillevel;
extern float MINY, MAXY, MINX, MAXX;


void Map::resize(const int width, const int height)
{
  Width = width;
  Height = height;
  map.clear();
  map.reserve(width * height);
}


void Map::draw(const Vector& viewp, const bool shadows, const Vector& light)
{
  glMatrixMode(GL_MODELVIEW);

  if (!shadows) {
    glPushMatrix();
    for (int j = 0; j < Height; j++) {
      if (j >= (viewp.y + MINY) &&
          j <= (viewp.y + MAXY)) {
        glPushMatrix();
        for (int i = 0; i < Width; i++) {
          if (i >= (viewp.x + MINX) &&
              i <= (viewp.x + MAXX)) {
            int o = map[i + j * Width];
            if (o == 0) {
              int m[8] = {13, 15, 17, 19, 7, 23, 21, 25};
              if (((i * 3 + j * 7) % m[(i + j) % 8]) == 0) o = 10;
              if (((i * 3 + j * 7) % m[(i + j + 1) % 8]) == 0 ) o = 11;
            } else {
            }
            if (detaillevel >= 1)
              Resources::tiles[o].draw(Resources::tiles[0].color);
            else
              Resources::tiles[o].draw_notexture(Resources::tiles[0].color);
          }
          glTranslatef(1.0, 0.0, 0.0);
        }
        glPopMatrix();
      }
      glTranslatef(0.0, 1.0, 0.0);
    }
    glPopMatrix();
  }

  Vector lightposv = light;
  for (const Building& b: buildings) {
    if (b.pos.y >= (viewp.y + MINY) &&
        b.pos.y <= (viewp.y + MAXY) &&
        b.pos.x >= (viewp.x + MINX) &&
        b.pos.x <= (viewp.x + MAXX)) {
      glPushMatrix();
      glTranslatef(float(b.pos.x), float(b.pos.y), float(b.pos.z));
      // @TODO: giant switch should be replaced by inheritance
      switch(b.type) {
      case Building::TYPE::FENCE:
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
      case Building::TYPE::WALL1:
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
      case Building::TYPE::WALL2:
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
      case Building::TYPE::WALL3:if (!shadows) {
          if (detaillevel >= 2)
            Resources::buildingTiles[2].draw(Color(0.3f, 0.3f, 0.3f));
          else
            Resources::buildingTiles[2].draw_notexture(Color(0.5, 0.5, 0.5));
        } else {
          glTranslatef(0, 0, 0.05f);
          Resources::buildingTiles[2].DrawShadow(Color(0, 0, 0, 0.5));
        }
        break;
      case Building::TYPE::WALL4:
        if (!shadows) {
          if (detaillevel >= 2)
            Resources::buildingTiles[3].draw(Color(0.5, 0.5, 0.5));
          else Resources::buildingTiles[3].draw_notexture(Color(0.5, 0.5, 0.5));
        } else {
          glTranslatef(0, 0, 0.05f);
          Resources::buildingTiles[3].DrawShadow(Color(0, 0, 0, 0.5));
        }
        break;
      case Building::TYPE::WALL5:
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
      case Building::TYPE::WALL6:
        if (!shadows) {
          if (detaillevel >= 2)
            Resources::buildingTiles[7].draw(Color(0.3f, 0.3f, 0.3f));
          else Resources::buildingTiles[7].draw_notexture(Color(0.5, 0.5, 0.5));
        } else {
          glTranslatef(0, 0, 0.05f);
          Resources::buildingTiles[7].DrawShadow(Color(0, 0, 0, 0.5));
        }
        break;
      case Building::TYPE::FACTORY_ELECTRONICS:
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
          Resources::pieceTiles[0][7].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
          glPopMatrix();
        }
        if (b.owner == 1) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        if (b.owner == 2) {
          if (!shadows) {
            glTranslatef(0,-1,1);
            Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
          } else {
            glTranslatef(float(-light.x),float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        break;
      case Building::TYPE::FACTORY_NUCLEAR:
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
          Resources::pieceTiles[0][6].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
          glPopMatrix();
        }

        if (b.owner == 1) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        if (b.owner == 2) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        break;
      case Building::TYPE::FACTORY_PHASERS:
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
          Resources::pieceTiles[0][5].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
          glPopMatrix();
        }
        if (b.owner == 1) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        if (b.owner == 2) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        break;
      case Building::TYPE::FACTORY_MISSILES:
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
          Resources::pieceTiles[0][4].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
          glPopMatrix();
        }
        if (b.owner == 1) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        if (b.owner == 2) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        break;
      case Building::TYPE::FACTORY_CANNONS:
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
          Resources::pieceTiles[0][3].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
          glPopMatrix();
        }

        if (b.owner == 1) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        if (b.owner == 2) {
          if (!shadows) {
            glTranslatef(0,-1,1);
            Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        break;
      case Building::TYPE::FACTORY_CHASSIS:
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
          Resources::pieceTiles[0][1].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
          glPopMatrix();
        }
        if (b.owner == 1) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
          } else {
            glTranslatef(float(-light.x), float(-light.y)-1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        if (b.owner == 2) {
          if (!shadows) {
            glTranslatef(0, -1, 1);
            Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 1, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        break;

      case Building::TYPE::WARBASE:if (!shadows) {
          if (detaillevel >= 2)
            Resources::buildingTiles[8].draw(Color(0.5, 0.5, 0.5));
          else
            Resources::buildingTiles[8].draw_notexture(Color(0.5, 0.5, 0.5));
        } else {
          glTranslatef(0, 0, 0.05f);
          Resources::buildingTiles[8].DrawShadow(Color(0, 0, 0, 0.5));
        }
        if (b.owner == 1) {
          if (!shadows) {
            glTranslatef(0, -2, 1);
            Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 2, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        if (b.owner == 2) {
          if (!shadows) {
            glTranslatef(0, -2, 1);
            Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
          } else {
            glTranslatef(float(-light.x), float(-light.y) - 2, 0.05f);
            Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
          }
        }
        break;
      }
      glPopMatrix();
    }
  }
}


float Map::maxZ(float x[2], float y[2])
{
  float z = 0;
  int o;

  for (int i = int(x[0]); float(i) < x[1]; i++) {
    for (int j = int(y[0]); float(j) < y[1]; j++) {
      if (i >= 0 && i < Width &&
          j >= 0 && j < Height) {
        o = map[i + j * Width];
        z = std::max(Resources::tiles[o].cmc.z[0], z);
        z = std::max(Resources::tiles[o].cmc.z[1], z);
      }
    }
  }
  return z;
}


int Map::terrain(float x, float y)
{
  switch (map[int(x) + int(y) * Width]) {
  case 0:
    return T_GRASS;
    break;
  case 1:
  case 2:
    return T_SAND;
    break;
  case 3:
    return T_MOUNTAINS;
    break;
  default:
    return T_HOLE;
    break;
  }
}


int Map::worseTerrain(float x[2], float y[2])
{
  int t = terrain(x[0] + 0.001f, y[0] + 0.001f);
  int t2 = terrain(x[1] - 0.001f, y[0] + 0.001f);
  if (t2 == T_HOLE || t == T_HOLE) return T_HOLE;
  if ((t2 == T_MOUNTAINS && (t == T_SAND || t == T_GRASS)) || (t2 == T_SAND && t == T_GRASS))
    t = t2;
  t2 = terrain(x[0] + 0.001f, y[1] - 0.001f);
  if (t2==T_HOLE) return T_HOLE;
  if ((t2 == T_MOUNTAINS && (t == T_SAND || t == T_GRASS)) || (t2 == T_SAND && t == T_GRASS))
    t = t2;
  t2 = terrain(x[1] - 0.001f, y[1] - 0.001f);
  if (t2 == T_HOLE) return T_HOLE;
  if ((t2 == T_MOUNTAINS && (t == T_SAND || t == T_GRASS)) ||
      (t2 == T_SAND && t == T_GRASS)) t = t2;
  return t;
}


bool Map::loadMap(const std::string& filename)
{
  std::ifstream iFile(filename);
  iFile >> Width >> Height;
  map.resize(Width, Height);

  const std::vector<std::string> tiles = {"G", "S", "S2", "M", "H1",
                                          "H2", "H3", "H4", "H5", "H6",
                                          "GG", "SS", "MM", "?"};
  std::string tilestr;
  for (int i = 0; i < Width * Height; i++) {
    iFile >> tilestr;
    int tile = find_index(tiles, tilestr);
    if (tile == 10) tile = 0;
    if (tile == 11) tile = 1;
    if (tile == 12) tile = 3;

    map.push_back(tile);
  }

  while (!iFile.eof()) {
    const std::vector<Building>& newBuildings {Building::readMapFile(iFile)};
    std::copy(newBuildings.cbegin(), newBuildings.cend(), std::back_inserter(buildings));
  }
  return true;
}
