#ifndef SCENE_H
#define SCENE_H

#include <memory>
#include <vector>

#include "bullet.h"
#include "camera.h"
#include "explosion.h"
#include "map.h"
#include "robots.h"
#include "ship.h"


class NETHER;

/*
  Manage map, robots, bullets, particles and explosions. Handles their interactions as collisions.
  Can be drawn.
*/
class Scene
{
public:
  Scene(NETHER* nether, const std::string& mapName);
  ~Scene();

  void clear();

  void draw(const Camera& camera, const Vector& light, const bool shadows);
  bool cycle(unsigned char* keyboard);

  void nuclearExplosionAt(const Vector& position);


  std::vector<Explosion> explosions;
  std::vector<std::unique_ptr<Bullet>> bullets;
  std::vector<Particle> particles;
  Map map;

  Robots robots;
  Ship ship;

  sexp::Value toSexp() const;
  bool fromSexp(const sexp::Value&);

private:
  void cycleBullets();
  void cycleRobots(unsigned char *keyboard);

  std::shared_ptr<NETHER> nether;
};

#endif // SCENE_H
