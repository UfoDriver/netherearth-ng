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

  for (const Building& building: buildings) {
    if (building.pos.y >= (viewp.y + MINY) &&
        building.pos.y <= (viewp.y + MAXY) &&
        building.pos.x >= (viewp.x + MINX) &&
        building.pos.x <= (viewp.x + MAXX)) {
      glPushMatrix();
      glTranslatef(float(building.pos.x), float(building.pos.y), float(building.pos.z));
      building.draw(shadows, detaillevel, light);
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
