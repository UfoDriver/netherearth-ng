#include <memory>
#include <numeric>
#include <cmath>

#include <GL/gl.h>

#include "constants.h"
#include "nether.h"
#include "robot.h"
#include "scene.h"
#include "ship.h"


Scene::Scene(NETHER *nether, const std::string& mapName)
  : nether{nether}, map{nether}, ship{"models/ship.asc", "textures/", nether}
{
  ship.computeShadow(nether->light.asVector());
};


Scene::~Scene()
{
  map.resize(0, 0);
}


bool Scene::cycle(unsigned char* keyboard)
{
  map.cycle();
  cycleRobots(keyboard);
  cycleBullets();

  explosions.erase(std::remove_if(explosions.begin(), explosions.end(),
                                  [](auto& exp) { return !exp.cycle(); }),
                   explosions.end());

  particles.erase(std::remove_if(particles.begin(), particles.end(),
                                 [](auto& particle) { return !particle.cycle(); }),
                  particles.end());

  return true;
}

void Scene::cycleBullets()
{
  bullets.erase(remove_if(bullets.begin(), bullets.end(),
                          [this](auto& bullet) {
                            bool ret = false;

                            if (bullet->angle == 0) bullet->pos.x += BULLET_SPEED;
                            if (bullet->angle == 90) bullet->pos.y += BULLET_SPEED;
                            if (bullet->angle == 180) bullet->pos.x -= BULLET_SPEED;
                            if (bullet->angle == 270) bullet->pos.y -= BULLET_SPEED;
                            bullet->step++;

                            std::shared_ptr<Robot> r;
                            if (bullet->step >= bullet->getPersistence() || bullet->checkCollision(map.buildings, robots, r)) {
                              ret = true;
                              if (bullet->step < bullet->getPersistence()) {
                                explosions.emplace_back(bullet->pos, 0);
                              }
                            }

                            if (r) {
                              /* The bullet has collided with a robot: */
                              if (!r->bulletHit(bullet)) {
                                /* Robot destroyed: */
                                explosions.emplace_back(r->pos, 1);
                                nether->sManager.playExplosion(ship.pos, r->pos);
                                nether->detachShip(r);
                                nether->ai.killRobot(r->pos);
                                robots.findAndDestroy(r);
                              }
                            }
                            return ret;
                          }),
                bullets.end());
}


void Scene::cycleRobots(unsigned char* keyboard)
{
  for (std::shared_ptr<Robot> r: robots) {
    r->cycle(nether.get());
    r->dispatchOperator(nether.get(), keyboard);
  }
}


void Scene::clear()
{
  explosions.clear();
  bullets.clear();
  particles.clear();
  robots.clear();
}


void Scene::draw(const Camera& camera, const Vector& light, const bool shadows)
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  map.draw(camera, light, shadows);
  ship.draw(shadows, light, map, nether->getControlledRobot());

  // if (explosions.size()) {
  //   int minstep = std::accumulate(explosions.cbegin(), explosions.cend(), 128,
  //                                 [](const int acc, const auto& e) {
  //                                   if (e.size == 2 && e.step < acc)
  //                                     return e.step;
  //                                   else
  //                                     return acc;
  //                                 });
  //   float r = (128 - minstep) / 256.0;
  //   float offset = sin(minstep) * r;
  //   camera.lookAt(offset);
  // } else {
  //   camera.lookAt();
  // }

  for (std::shared_ptr<Robot> r: robots) {
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


  if (!shadows) {
    for (const Explosion& exp: explosions) {
      if (camera.canSee(exp.pos)) {
        exp.draw(light, shadows);
      }
    }

    for (const Particle& particle: particles) {
      if (camera.canSee(particle.pos))
        particle.draw();
    }
  }
}


void Scene::nuclearExplosionAt(const Vector& position)
{
  Explosion explosion(position, 2);
  explosions.push_back(explosion);

  /* Find Robots to destroy: */
  // Ops, modifying container while iterating
  robots.erase(std::remove_if(robots.begin(), robots.end(),
                              [explosion, this] (auto& r) {
                                float distance=(r->pos - explosion.pos).norma();
                                if (distance <= NUCLEAR_RADIUS) {
                                  if (nether->getControlledRobot() == r)
                                    nether->detachShip(r);
                                  nether->ai.killRobot(r->pos);
                                  return true;
                                } else {
                                  return false;
                                }
                              }),
               robots.end());

  nether->sManager.playExplosion(ship.pos, position);
}
