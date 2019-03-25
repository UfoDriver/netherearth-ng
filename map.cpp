#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <unordered_map>

#include "bulletcannon.h"
#include "bulletmissile.h"
#include "bulletphaser.h"
#include "map.h"
#include "nether.h"
#include "resources.h"

extern int detaillevel;
extern int up_key, down_key, left_key, right_key, fire_key, pause_key;


void Map::resize(const int width, const int height)
{
  Width = width;
  Height = height;
  map.clear();
  map.resize(width * height, 0);
  explosions.clear();
  buildings.clear();
  bullets.clear();
  for (Robot* r: robots) delete r;
  robots.clear();
}


void Map::draw(const Camera& camera, const Vector& light, const bool shadows)
{
  if (explosions.size()) {
    int minstep = std::accumulate(explosions.cbegin(), explosions.cend(), 128,
                                  [](const int acc, const auto& e) {
                                    if (e.size == 2 && e.step < acc)
                                      return e.step;
                                    else
                                      return acc;
                                  });
    float r = (128 - minstep) / 256.0;
    float offset = sin(minstep) * r;
    camera.lookAt(offset);
  } else {
    camera.lookAt();
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (!shadows) {
    glPushMatrix();
    for (int j = 0; j < Height; j++) {
      glPushMatrix();
      for (int i = 0; i < Width; i++) {
        if (camera.canSee(Vector(i, j, 0.0))) {
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
      glTranslatef(0.0, 1.0, 0.0);
    }
    glPopMatrix();
  }

  for (const auto& building: buildings) {
    if (camera.canSee(building->pos)) {
      glPushMatrix();
      glTranslatef(float(building->pos.x), float(building->pos.y), float(building->pos.z));
      building->draw(shadows, detaillevel, light);
      glPopMatrix();
    }
  }

  for (Robot* r: robots) {
    if (camera.canSee(r->pos)) {
      glPushMatrix();
      glTranslatef(r->pos.x, r->pos.y, r->pos.z);
      r->draw(light, shadows);
      glPopMatrix();
    }
  }

  for (const auto& bullet: bullets) {
    if (camera.canSee(bullet->pos)) {
      glPushMatrix();
      glTranslatef(bullet->pos.x, bullet->pos.y, bullet->pos.z);
      bullet->draw(shadows, particles);
      glPopMatrix();
    }
  }

  nether->getShip()->draw(shadows, light, *this, nether->getControlled());

  if (!shadows) {
    for (const Explosion& exp: explosions) {
      glPushMatrix();
      glTranslatef(exp.pos.x, exp.pos.y, exp.pos.z);
      glColor4f(1.0f, 0.5f, 0.0, exp.getAlpha());
      glDepthMask(GL_FALSE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      // Somehow solid sphere dumps core
      // glutSolidSphere(exp.getRadius(), 8, 8);
      glutWireSphere(exp.getRadius(), 8, 8);
      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
      glPopMatrix();
    }

    for (const Particle& particle: particles) {
      if (camera.canSee(particle.pos))
        particle.draw();
    }
  }
}


float Map::maxZ(float x[2], float y[2]) const
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


float Map::maxZ(const Vector& position) const
{
  float z = 0;
  float x[2], y[2];
  x[0] = position.x - 0.5;
  x[1] = position.x + 0.5;
  y[0] = position.y - 0.5;
  y[1] = position.y + 0.5;
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


int Map::worseTerrain(const Vector& pos)
{
  float x[2], y[2];
  x[0] = pos.x - 0.5;
  x[1] = pos.x + 0.5;
  y[0] = pos.y - 0.5;
  y[1] = pos.y + 0.5;

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
  resize(Width, Height);

  const std::unordered_map<std::string, int> tiles = {
      {"G", 0},   {"S", 1},   {"S2", 2},  {"M", 3},  {"H1", 4},
      {"H2", 5},  {"H3", 6},  {"H4", 7},  {"H5", 8}, {"H6", 9},
      {"GG", 10}, {"SS", 11}, {"MM", 12}, {"?", 13}};
  std::string tilestr;
  for (int i = 0; i < Width * Height; i++) {
    iFile >> tilestr;
    int tile = tiles.at(tilestr);
    if (tile >= 10) tile -= 10;
    map[i] = tile;
  }

  do {
    std::vector<std::unique_ptr<BuildingBlock>> newBuildings {BuildingBlock::readMapFile(iFile)};
    buildings.insert(buildings.end(),
                     std::make_move_iterator(newBuildings.begin()),
                     std::make_move_iterator(newBuildings.end()));
  } while (iFile.good());

  return true;
}


bool Map::cycle(unsigned char *keyboard)
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

  cycleRobots(keyboard);
  cycleBuildings();
  cycleBullets();

  explosions.erase(std::remove_if(explosions.begin(), explosions.end(),
                                  [](auto& exp) { return !exp.cycle(); }),
                   explosions.end());

  particles.erase(std::remove_if(particles.begin(), particles.end(),
                                 [](auto& particle) { return !particle.cycle(); }),
                  particles.end());

  return true;
}


void Map::cycleBuildings()
{
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
}


void Map::cycleBullets()
{
  bullets.erase(remove_if(bullets.begin(), bullets.end(),
                          [this](auto& bullet) {
                            bool ret = false;

                            if (bullet->angle == 0) bullet->pos.x += BULLET_SPEED;
                            if (bullet->angle == 90) bullet->pos.y += BULLET_SPEED;
                            if (bullet->angle == 180) bullet->pos.x -= BULLET_SPEED;
                            if (bullet->angle == 270) bullet->pos.y -= BULLET_SPEED;
                            bullet->step++;


                            Robot* r = 0;
                            if (bullet->step >= bullet->getPersistence() || bullet->checkCollision(buildings, robots, &r)) {
                              ret = true;
                              if (bullet->step < bullet->getPersistence()) {
                                explosions.emplace_back(bullet->pos, 0);
                              }
                            }

                            if (r != 0) {
                              /* The bullet has collided with a robot: */
                              if (!r->bulletHit(bullet)) {
                                /* Robot destroyed: */
                                explosions.emplace_back(r->pos,1);
                                nether->sManager.playExplosion(nether->getShip()->pos, r->pos);
                                nether->detachShip(r);
                                nether->ai.killRobot(r->pos);
                                robots.findAndDestroy(r);
                              }
                            }
                            return ret;
                          }),
                bullets.end());
}


void Map::cycleRobots(unsigned char* keyboard)
{
  for (Robot* r: robots) {
    r->cycle(nether);
    r->dispatchOperator(this->nether, keyboard);
  }
}


std::ostream& operator<<(std::ostream& out, const Map& map)
{
  out << map.width() << ' ' << map.height() << '\n';

  for (int i = 0; i < map.height(); i++) {
    for (int j = 0; j < map.width(); j++) {
      out << map.map[j + i * map.width()] << ' ';
    }
    out << '\n';
  }
  return out;
}


std::istream& operator>>(std::istream& in, Map& map)
{
  int width, height;
  in >> width >> height;
  map.resize(width, height);

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      int tile;
      in >> tile;
      map.map.push_back(tile);
    }
  }
  return in;
}


void Map::nuclearExplosionAt(Robot* robot, const Vector& position)
{
  Explosion exp(position, 2);
  explosions.emplace_back(exp);

  /* Robot destroyed: */
  nether->detachShip(robot);

  /* Find Robots to destroy: */
  // Ops, modifying container while iterating
  robots.erase(std::remove_if(robots.begin(), robots.end(),
                              [exp, this] (auto& r) {
                                float distance=(r->pos - exp.pos).norma();
                                if (distance <= NUCLEAR_RADIUS) {
                                  nether->ai.killRobot(r->pos);
                                  return true;
                                } else {
                                  return false;
                                }
                              }),
               robots.end());

  /* Find buildings to destroy: */
  buildings.erase(std::remove_if(buildings.begin(), buildings.end(),
                                 [exp, this](auto& b) {
                                   float distance = (b->pos - (exp.pos - Vector(0.5, 0.5, 0.5))).norma();
                                   if (distance <= NUCLEAR_RADIUS) {
                                     nether->ai.removeBuilding(b->pos);
                                     return true;
                                   } else {
                                     return false;
                                   }
                                 }),
                  buildings.end());
  nether->sManager.playExplosion(nether->getShip()->pos, position);
  nether->stats.requestRecomputing();
}
