#include <GL/gl.h>
#include <memory>
#include <sexp/util.hpp>
#include <sexp/value.hpp>

#include "building.h"
#include "constants.h"
#include "map.h"
#include "nether.h"
#include "robot.h"
#include "ship.h"


Ship::Ship(const std::string &model, const std::string &texturesDir, NETHER *nether)
    : Shadow3DObject(model, texturesDir), nether{nether} {
  normalize(0.5f);
  makepositive();
}


bool Ship::checkCollision(const std::vector<std::shared_ptr<BuildingBlock>>& buildingBlocks,
                          const std::vector<std::shared_ptr<Robot>>& robots)
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
  for (const auto& b: buildingBlocks) {
    if (b->pos.aboutToCollide3D(pos, COLISION_TEST_THRESHOLD)) {
      if (b->collisionCheck(cmc, m1)) return true;
    }
  }

  /* Collision with the robots: */
  for (std::shared_ptr<Robot> r: robots) {
    if (r->pos.aboutToCollide2D(pos, COLISION_TEST_THRESHOLD)) {
      m2[12] = r->pos.x;
      m2[13] = r->pos.y;
      m2[14] = r->pos.z;
      if (cmc.collision_simple(m1, r->cmc, m2)) return true;
    }
  }

  return false;
}


void Ship::draw(const bool shadows, const Vector& light, const Map& map, const std::shared_ptr<Robot> controlled)
{
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);
  if (!shadows)
    Shadow3DObject::draw(Color(0.7, 0.7, 0.7));
  // cmc.draw(Color(255, 0, 0));
  glPopMatrix();

  if (shadows) {
    float sx, sy;
    float minz;

    sx = pos.x - light.x * pos.z;
    sy = pos.y - light.y * pos.z;

    if (controlled == 0) {
      float x[2], y[2];
      x[0] = sx + shadowCMC.x[0];
      x[1] = sx + shadowCMC.x[1];
      y[0] = sy + shadowCMC.y[0];
      y[1] = sy + shadowCMC.y[1];
      minz = map.getMaxZ(x, y);
    } else {
      minz = controlled->pos.z;
    }

    glPushMatrix();
    glTranslatef(sx, sy, minz + 0.05);
    drawShadow(Color(0, 0, 0, 0.5));
    glPopMatrix();
  }
}


bool Ship::landedHere(const Vector& position) const
{
  return landed && pos.compare2D(position);
}


void Ship::cycle(const Config& config, unsigned char* keyboard)
{
  float x[2], y[2];
  float minz;
  Vector old_shipp;

  pos.x = (int(pos.x * 8.0)) / 8.0;
  pos.y = (int(pos.y * 8.0)) / 8.0;
  old_shipp = pos;
  x[0] = pos.x;
  x[1] = pos.x + 1.0;
  y[0] = pos.y;
  y[1] = pos.y + 1.0;
  minz = nether->scene.map.getMaxZ(x, y);

  if (op == Ship::OPS::RIGHT && pos.x < nether->scene.map.getWidth() - 1) {
    pos.x += 0.125;
    if (timemoving >= 50 && (int(pos.x * 8) % 2) == 1)
      pos.x += 0.125;
  }
  if (op == Ship::OPS::LEFT && pos.x > 0) {
    pos.x -= 0.125;
    if (timemoving >= 50 && (int(pos.x * 8) % 2) == 1)
      pos.x -= 0.125;
  }
  if (op2 == Ship::OPS::FORWARD && pos.y < nether->scene.map.getHeight() - 1) {
    pos.y += 0.125;
    if (timemoving >= 50 && (int(pos.y * 8) % 2) == 1)
      pos.y += 0.125;
  }
  if (op2==Ship::OPS::BACKWARD && pos.y > 0) {
    pos.y -= 0.125;
    if (timemoving >= 50 && (int(pos.y * 8) % 2) == 1)
      pos.y -= 0.125;
  }

  if (op3 == Ship::OPS::UP && pos.z < 5.0)
    pos.z += 0.05;
  if (op3 != Ship::OPS::UP && pos.z > minz)
    pos.z -= 0.025;
  pos.z = std::max(pos.z, minz);

  if (op == Ship::OPS::NONE && op2 == Ship::OPS::NONE) {
    timemoving = 0;
  } else {
    timemoving++;
  }

  if (pos != old_shipp) {
    if (checkCollision(nether->scene.map.buildingBlocks, nether->scene.robots)) {
      timemoving = 0;
      Vector p = pos;
      pos.x = old_shipp.x;
      pos.y = old_shipp.y;
      if (p.z != old_shipp.z && checkCollision(nether->scene.map.buildingBlocks, nether->scene.robots)) {
        pos.z = old_shipp.z;
        landed = true;
      } else {
        pos.z = p.z;
      }
      pos.x = p.x;
      if (p.x != old_shipp.x && checkCollision(nether->scene.map.buildingBlocks, nether->scene.robots)) {
        pos.x = old_shipp.x;
      } else {
        pos.x = p.x;
      }
      pos.y = p.y;
      if (p.y != old_shipp.y && checkCollision(nether->scene.map.buildingBlocks, nether->scene.robots)) {
        pos.y = old_shipp.y;
      } else {
        pos.y = p.y;
      }
    }
  }

  if ((int(pos.x * 8) % 4) == 0)
    op = Ship::OPS::NONE;
  if ((int(pos.y * 8) % 4) == 0)
    op2 = Ship::OPS::NONE;
  if ((int(pos.z * 8) % 4) == 0)
    op3 = Ship::OPS::NONE;

  if (keyboard[config.keyLeft]) {
    op = Ship::OPS::LEFT;
  }
  if (keyboard[config.keyRight]) {
    op = Ship::OPS::RIGHT;
  }
  if (keyboard[config.keyUp]) {
    op2 = Ship::OPS::FORWARD;
  }
  if (keyboard[config.keyDown]) {
    op2 = Ship::OPS::BACKWARD;
  }
  if (keyboard[config.keyFire]) {
    op3 = Ship::OPS::UP;
  }
}


sexp::Value Ship::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("ship"),
    pos.toSexp()
  );
}


bool Ship::fromSexp(const sexp::Value& value)
{
  pos.fromSexp(sexp::cdar(value));

  return true;
}
