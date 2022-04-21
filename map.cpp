#include <algorithm>
#include <cmath>
#include <numeric>
#include <unordered_map>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <sexp/parser.hpp>
#include <sexp/util.hpp>
#include <sexp/value.hpp>

#include "bulletcannon.h"
#include "bulletmissile.h"
#include "bulletphaser.h"
#include "map.h"
#include "nether.h"
#include "resources.h"

extern int up_key, down_key, left_key, right_key, fire_key, pause_key;


void Map::resize(const int newWidth, const int newHeight)
{
  width = newWidth;
  height = newHeight;
  buildings.clear();
  map.clear();
  map.resize(width * height, 0);
}


void Map::draw(const Camera& camera, const Vector& light, const bool shadows)
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  camera.lookAt();

  if (!shadows) {
    glPushMatrix();
    for (int j = 0; j < height; j++) {
      glPushMatrix();
      for (int i = 0; i < width; i++) {
        if (camera.canSee(Vector(i, j, 0.0))) {
          int o = map[i + j * width];
          if (o == 0) {
            int m[8] = {13, 15, 17, 19, 7, 23, 21, 25};
            if (((i * 3 + j * 7) % m[(i + j) % 8]) == 0) o = 10;
            if (((i * 3 + j * 7) % m[(i + j + 1) % 8]) == 0 ) o = 11;
          } else {
          }
          Resources::tiles[o].draw(Resources::tiles[0].color);
        }
        glTranslatef(1.0, 0.0, 0.0);
      }
      glPopMatrix();
      glTranslatef(0.0, 1.0, 0.0);
    }
    glPopMatrix();
  }

  for (const auto& building: buildings) {
    // Probably it's faster to open widen camera radius a bit and use building position
    if (std::any_of(building->blocks.cbegin(), building->blocks.cend(),
                    [camera](const auto& bb) { return camera.canSee(bb.pos);})) {
      building->draw(shadows, light);
    }
  }
}


float Map::getMaxZ(float x[2], float y[2]) const
{
  float z = 0;

  for (int i = int(x[0]); float(i) < x[1]; i++) {
    for (int j = int(y[0]); float(j) < y[1]; j++) {
      if (i >= 0 && i < width &&
          j >= 0 && j < height) {
        int o = map[i + j * width];
        z = std::max(Resources::tiles[o].cmc.z[0], z);
        z = std::max(Resources::tiles[o].cmc.z[1], z);
      }
    }
  }
  return z;
}


float Map::getMaxZ(const Vector& position) const
{
  float z = 0;
  float x[2], y[2];
  x[0] = position.x - 0.5;
  x[1] = position.x + 0.5;
  y[0] = position.y - 0.5;
  y[1] = position.y + 0.5;

  for (int i = int(x[0]); float(i) < x[1]; i++) {
    for (int j = int(y[0]); float(j) < y[1]; j++) {
      if (i >= 0 && i < width &&
          j >= 0 && j < height) {
        int o = map[i + j * width];
        z = std::max(Resources::tiles[o].cmc.z[0], z);
        z = std::max(Resources::tiles[o].cmc.z[1], z);
      }
    }
  }
  return z;
}


int Map::getTerrain(float x, float y)
{
  switch (map[int(x) + int(y) * width]) {
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


int Map::worseTerrain(const Vector& pos)
{
  float x[2], y[2];
  x[0] = pos.x - 0.5;
  x[1] = pos.x + 0.5;
  y[0] = pos.y - 0.5;
  y[1] = pos.y + 0.5;

  int t = getTerrain(x[0] + 0.001f, y[0] + 0.001f);
  int t2 = getTerrain(x[1] - 0.001f, y[0] + 0.001f);
  if (t2 == T_HOLE || t == T_HOLE) return T_HOLE;
  if ((t2 == T_MOUNTAINS && (t == T_SAND || t == T_GRASS)) || (t2 == T_SAND && t == T_GRASS))
    t = t2;
  t2 = getTerrain(x[0] + 0.001f, y[1] - 0.001f);
  if (t2==T_HOLE) return T_HOLE;
  if ((t2 == T_MOUNTAINS && (t == T_SAND || t == T_GRASS)) || (t2 == T_SAND && t == T_GRASS))
    t = t2;
  t2 = getTerrain(x[1] - 0.001f, y[1] - 0.001f);
  if (t2 == T_HOLE) return T_HOLE;
  if ((t2 == T_MOUNTAINS && (t == T_SAND || t == T_GRASS)) ||
      (t2 == T_SAND && t == T_GRASS)) t = t2;
  return t;
}


bool Map::loadMap(const std::string& filename)
{
  std::cout << "Loading map " << filename << std::endl;
  std::ifstream sexpFile(filename);
  sexp::Value value = sexp::Parser::from_stream(sexpFile, false);
  if (sexp::car(value).as_string() == "map") {
    for (const sexp::Value& section: sexp::ListAdapter(sexp::cdr(value))) {
      processMapSectionSexp(section);
    }
  } else {
    std::cout << "Cannot load map " << filename + ".sexp" << std::endl;
  }

  return true;
}


bool Map::cycle()
{
  // Keyboard/zoom
  // ship movement
  // robot menu process/redrawing
  // viewport calculation
  // Status redraw
  // Ship on the factory/robot handling
  // Robots cycling
  // Building cycling (capturing)
  // Bullets cycling
  // Nuclear explosion stepping/removal
  // Menu cycling
  // Explosions cycling
  // Particles cycling
  // Game goals checking

  for (auto& b: buildings) {
    if (b->isCapturable()) {
      int robot = nether->ai.robotHere(b->getCapturePoint());

      if (robot == 0) {
        b->status = 0;
      } else {
        if (robot == T_ROBOT) b->status++;
        if (robot == T_EROBOT) b->status--;

        if (b->status >= CAPTURE_TIME) {
          b->owner = 1;
          b->status = 0;
          nether->requestStatsRecomputing();
        }
        if (b->status <= -CAPTURE_TIME) {
          b->owner = 2;
          b->status = 0;
          nether->requestStatsRecomputing();
        }
      }
    }
  }
  return true;
}


void Map::nuclearExplosionAt(std::shared_ptr<Robot> robot, const Vector& position)
{
  /* Robot destroyed: */
  nether->detachShip(robot);

  /* Find buildings to destroy: */
  buildings.erase(std::remove_if(buildings.begin(), buildings.end(),
                                 [position, this](auto& b) {
                                   float distance = (b->pos - (position - Vector(0.5, 0.5, 0.5))).norma();
                                   if (distance <= NUCLEAR_RADIUS) {
                                     nether->ai.removeBuilding(b->pos);
                                     return true;
                                   } else {
                                     return false;
                                   }
                                 }),
                  buildings.end());

  nether->stats.requestRecomputing();
}

void Map::processMapSectionSexp(const sexp::Value& cons)
{
  if (sexp::car(cons).as_string() == "terrain") {
    height = sexp::list_length(cons) - 1;
    // Seems cadr is cdar in this implementation
    width = sexp::list_length(sexp::cdar(cons));
    resize(width, height);
    const std::unordered_map<std::string, int> tiles = {
        {"G", 0},   {"S", 1},   {"S2", 2},  {"M", 3},  {"H1", 4},
        {"H2", 5},  {"H3", 6},  {"H4", 7},  {"H5", 8}, {"H6", 9},
        {"GG", 10}, {"SS", 11}, {"MM", 12}, {"?", 13}};

    int index = 0;
    for (const sexp::Value& row: sexp::ListAdapter(sexp::cdr(cons))) {
      for (const sexp::Value &tile: sexp::ListAdapter(row)) {
        int tileCode = tiles.at(tile.as_string());
        if (tileCode >= 10) tileCode -= 10;
        map[index++] = tileCode;
      }
    }
  } else if (sexp::car(cons).as_string() == "buildings") {
    std::cerr << "Got buildings" << std::endl;
    for (const sexp::Value& building: sexp::ListAdapter(sexp::cdr(cons))) {
      Building *b = Building::getFromSexp(building);
      if (b) {
        buildings.emplace_back(b);
      }
    }
  }
}


sexp::Value Map::toSexp() const
{
  std::vector<sexp::Value> sexpMap;
  std::transform(map.cbegin(), map.cend(), std::back_inserter(sexpMap), [](int x) { return sexp::Value::integer(x); });

  return sexp::Value::list(
    sexp::Value::symbol("map"),
    sexp::Value::integer(width),
    sexp::Value::integer(height),
    sexp::Value::array(sexpMap)
  );
}


bool Map::fromSexp(const sexp::Value& value)
{
  width = sexp::cdar(value).as_int();
  height = sexp::cddar(value).as_int();

  std::vector<sexp::Value> mapItems = sexp::cdddar(value).as_array();
  map.clear();
  std::transform(mapItems.cbegin(), mapItems.cend(), std::back_inserter(map),
                 [](const sexp::Value &value) { return value.as_int(); });

  return true;
}
