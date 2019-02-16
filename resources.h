#ifndef RESOURCES_H
#define RESOURCES_H

#include <vector>

#include "piece3dobject.h"
#include "shadow3dobject.h"


class Resources {
public:
  static std::vector<Shadow3DObject> buildingTiles;
  static std::vector<Piece3DObject> pieceTiles[2];
};


#endif // RESOURCES_H
