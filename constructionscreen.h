#ifndef CONSTRUCTIONSCREEN_H
#define CONSTRUCTIONSCREEN_H

#include "light.h"
#include "robot.h"

class NETHER;


class ConstructionScreen
{
public:
  explicit ConstructionScreen(NETHER* nether): construction_pointer(0), construction{},
                                               nether(nether) {}
  bool cycle(unsigned char *keyboard);
  void draw(int width, int height, const Light& light);
  void open(const Building& factory);

private:
  int construction_pointer;
  bool construction[8];
  Robot in_construction;
  NETHER* nether;
};


#endif // CONSTRUCTIONSCREEN_H
