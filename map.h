#ifndef MAP_H
#define MAP_H

#include <memory>
#include <vector>

#include "building.h"
#include "bullet.h"
#include "camera.h"
#include "explosion.h"
#include "particle.h"
#include "robots.h"
#include "vector.h"

class NETHER;
class Robot;


class Map
{
public:
  explicit Map(NETHER* nether) : Width{0}, Height{0}, nether {nether} {}

  std::vector<int> map;

  void resize(int const width, int const height);
  void draw(const Camera& camera, const Vector& light, const bool shadows);

  int width() const { return Width; }
  int height() const { return Height; }
  bool cycle(unsigned char* keyboard);
  bool loadMap(const std::string& filename);
  float maxZ(float x[2], float y[2]) const;
  float maxZ(const Vector& position) const;
  int terrain(float x, float y);
  int worseTerrain(const Vector& position);
  void nuclearExplosionAt(Robot* robot, const Vector& position);

  std::vector<std::unique_ptr<Building>> buildings;
  std::vector<std::unique_ptr<Bullet>> bullets;
  std::vector<Explosion> explosions;
  std::vector<Particle> particles;
  Robots robots;

private:
  void cycleBuildings();
  void cycleBullets();
  void cycleRobots(unsigned char *keyboard);
  int Width;
  int Height;
  NETHER* nether;
};


std::ostream& operator<<(std::ostream& out, const Map& map);
std::istream& operator>>(std::istream& in, Map& map);


#endif // MAP_H
