#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <vector>

#include "bullet.h"
#include "camera.h"
#include "explosion.h"
#include "robots.h"


class NETHER;

/*
  Manage map, robots, bullets, particles and explosions. Handles their interactions as collisions.
  Can be drawn.
*/
class Scene
{
public:

  void clear();

  bool cycle(unsigned char* keyboard, NETHER* nether);
  void draw(const Camera& camera, const Vector& light, const bool shadows);

  void nuclearExplosionAt(const Vector& position, NETHER* nether);


  std::vector<Explosion> explosions;
  std::vector<std::unique_ptr<Bullet>> bullets;
  std::vector<Particle> particles;

  Robots robots;

  sexp::Value toSexp() const;
  bool fromSexp(const sexp::Value&);

private:
  void cycleBullets(NETHER* nether);
  void cycleRobots(unsigned char *keyboard, NETHER* nether);
};

#endif // SCENE_H
