#ifndef RESOURCES_H
#define RESOURCES_H

#include <vector>

#include "3dobject.h"
#include "piece3dobject.h"
#include "shadow3dobject.h"


class Resources {
public:
  static Resources* instance();
  void refreshDisplayLists();
  void loadObjects();
  void deleteObjects();

  static std::vector<Shadow3DObject> buildingTiles;
  static std::vector<Piece3DObject> pieceTiles[2];
  static std::vector<C3DObject> tiles;
  static std::vector<C3DObject> constructionTiles;
  static std::vector<C3DObject> messageTiles;
  static std::vector<Piece3DObject> bulletTiles;

private:
  Resources() {};
  static Resources* singleton;
};


#endif // RESOURCES_H
