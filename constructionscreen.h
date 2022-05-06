#ifndef CONSTRUCTIONSCREEN_H
#define CONSTRUCTIONSCREEN_H

#include "config.h"
#include "light.h"
#include "robot.h"

class NETHER;


class ConstructionScreen
{
private:
  enum class MENU {
    EXIT = 0,
    START = 10,
    BIPOD = 20,
    TRACKS,
    ANTIGRAV,
    CANNON,
    MISSILES,
    PHASERS,
    NUCLEAR,
    ELECTRONICS
  };
public:
  explicit ConstructionScreen(NETHER* nether): menuPointer(MENU::EXIT), staple(nullptr),
                                               nether(nether) {}
  bool cycle(const Config& config, unsigned char *keyboard);
  void draw(int width, int height, const Light& light);
  void open(const Vector& factoryPos);

private:
  void buildRobot();
  void constructRobot();

  MENU menuPointer;
  std::shared_ptr<Robot> staple;
  NETHER* nether;

  friend MENU operator++(MENU &m, int);
  friend MENU operator--(MENU &m, int);
};


#endif // CONSTRUCTIONSCREEN_H
