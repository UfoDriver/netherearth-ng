#ifndef MAP_H
#define MAP_H

#include <vector>

#include "building.h"
#include "bullet.h"
#include "camera.h"
#include "explosion.h"
#include "particle.h"
#include "robot.h"
#include "vector.h"

class NETHER;


class Map
{
public:
  Map(NETHER* nether): Width(0), Height(0), nether(nether) {}

  std::vector<int> map;

  void resize(int const width, int const height);
  void draw(const Vector& viewp, const bool shadows, const Vector& light, const Camera& camera);

  int width() const { return Width; }
  int height() const { return Height; }
  bool cycle(unsigned char *keyboard);
  bool loadMap(const std::string& filename);
  float maxZ(float x[2], float y[2]) const;
  int terrain(float x, float y);
  int worseTerrain(float x[2], float y[2]);

  std::vector<Building> buildings;
  std::vector<Bullet> bullets;
  std::vector<Explosion> explosions;
  std::vector<Particle> particles;
  std::vector<Robot*> robots[2];
private:
  void cycleBuildings();
  void cycleBullets();
  void cycleRobots(unsigned char *keyboard);
  void find_and_destroy_robot(Robot* robot);
  int Width;
  int Height;
  NETHER* nether;
};


std::ostream& operator<<(std::ostream& out, const Map& map);
std::istream& operator>>(std::istream& in, Map& map);


#endif // MAP_H
