#ifndef CONSTRUCTIONSCREEN_H
#define CONSTRUCTIONSCREEN_H

#include "light.h"
#include "robot.h"

class NETHER;


class ConstructionScreen
{
private:
  enum class MENU {EXIT = 0,
                   START = 10,
                   BIPOD = 20,
                   TRACKS,
                   ANTIGRAV,
                   CANNON,
                   MISSILES,
                   PHASERS,
                   NUCLEAR,
                   ELECTRONICS};
public:
  explicit ConstructionScreen(NETHER* nether): menuPointer(MENU::EXIT), staple(nullptr),
                                               nether(nether) {}
  bool cycle(unsigned char *keyboard);
  void draw(int width, int height, const Light& light);
  void open(const BuildingBlock& factory);

private:
  void buildRobot();
  void constructRobot();

  MENU menuPointer;
  Robot* staple;
  NETHER* nether;

  friend MENU operator++(MENU &m, int);
  friend MENU operator--(MENU &m, int);
};


#endif // CONSTRUCTIONSCREEN_H
