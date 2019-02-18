#ifndef CONSTRUCTIONSCREEN_H
#define CONSTRUCTIONSCREEN_H

#include "robot.h"

class NETHER;


class ConstructionScreen
{
public:
  explicit ConstructionScreen(NETHER* nether): nether(nether), construction_pointer(0) {}
  bool cycle(unsigned char *keyboard);
  void draw(int width, int height, const Vector& lightposv);
  void open(const Building& factory);

private:
  int construction_pointer;
  bool construction[8];
  Robot in_construction;
  NETHER* nether;
};


#endif // CONSTRUCTIONSCREEN_H
