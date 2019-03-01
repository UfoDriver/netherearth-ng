#include "constants.h"
#include "resources.h"

extern int shadows;


Resources* Resources::instance()
{
  if (!singleton)
    singleton = new Resources;
  return singleton;
}


void Resources::refreshDisplayLists()
{
  for (C3DObject& tile: tiles) {
    tile.refresh_display_lists();
  }

  for (Shadow3DObject& tile: buildingTiles) {
      tile.refresh_display_lists();
  }

  for (int i = 0; i < N_PIECES; i++) {
    pieceTiles[0][i].refresh_display_lists();
    pieceTiles[1][i].refresh_display_lists();
  }

  for (C3DObject& tile: constructionTiles) {
    tile.refresh_display_lists();
  }
}


void Resources::loadObjects()
{
  const char *tnames[12]={"models/grass1.ase","models/rough.ase","models/rocks.ase","models/heavyrocks.ase",
                    "models/hole1.asc","models/hole2.asc","models/hole3.asc",
                    "models/hole4.asc","models/hole5.asc","models/hole6.asc",
                    "models/grass2.ase","models/grass3.ase"};
  const char *bnames[9]={"models/lowwall1.ase","models/lowwall2.ase","models/lowwall3.ase",
                   "models/highwall1.ase","models/factory.ase","models/fence.asc",
                   "models/flag.asc","models/highwall2.ase","models/warbase.ase"};
  const char *pnames[11]={"models/h-bipod.ase","models/h-tracks.ase","models/h-antigrav.ase",
                    "models/h-cannon.ase","models/h-missiles.ase","models/h-phasers.ase",
                    "models/h-nuclear.ase","models/h-electronics.ase",
                    "models/h-bipod-base.ase","models/h-bipod-rleg.ase","models/h-bipod-lleg.ase"};
  const char *pnames2[11]={"models/e-bipod.ase","models/e-tracks.ase","models/e-antigrav.ase",
                     "models/e-cannon.ase","models/e-missiles.ase","models/e-phasers.ase",
                     "models/nuclear.asc","models/e-electronics.ase",
                     "models/e-bipod-base.ase","models/e-bipod-rleg.ase","models/e-bipod-lleg.ase"};
  const char *bullnames[3]={"models/bullet1.asc","models/bullet2.asc","models/bullet3.asc"};
  float pscale[11]={0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.5,0.45,0.375,0.375};
  float bscale[9]={0.5,0.5,0.5,
                   0.5,0.5,1.0,
                   0.25,0.5,0.5};
  float bullscale[3]={0.05,0.3,0.4};
  Color colors[12] = {{0.0f, 0.733f, 0.0f},
                      {0.7f, 0.5f, 0.0f},
                      {0.6f, 0.45f, 0.0f},
                      {0.5f, 0.4f, 0.0f},
                      {0.0f, 0.733f, 0.0f},
                      {0.0f, 0.733f, 0.0f},
                      {0.0f, 0.733f, 0.0f},
                      {0.0f, 0.733f, 0.0f},
                      {0.0f, 0.733f, 0.0f},
                      {0.0f, 0.733f, 0.0f},
                      {0.0f, 0.733f, 0.0f},
                      {0.0f, 0.733f, 0.0f}};

  tiles.reserve(N_OBJECTS);
  for (int i = 0; i < N_OBJECTS; i++) {
    C3DObject tile(tnames[i], "textures/", colors[i]);
    tile.normalize(0.50f);
    tile.makepositive();
    tiles.push_back(tile);
  }

  for (int i = 4; i < 10; i++) {
    tiles[i].moveobject(Vector(0, 0, -0.05));
  }

  buildingTiles.reserve(N_BUILDINGS);
  for (int i = 0; i < N_BUILDINGS; i++) {
    Shadow3DObject tile(bnames[i], "textures/");
    tile.normalize(bscale[i]);
    tile.makepositive();
    buildingTiles.push_back(tile);
  }
  buildingTiles[5].moveobject(Vector(0, 0, 0.01));
  buildingTiles[6].moveobject(Vector(0.4, 0.4, 0.0));

  for (int i = 0; i < N_PIECES; i++) {
    Piece3DObject tile(pnames[i], "textures/");
    tile.normalize(pscale[i]);
    tile.makepositive();
    pieceTiles[0].push_back(tile);

    Piece3DObject tile2(pnames2[i], "textures/");
    tile2.normalize(pscale[i]);
    tile2.makepositive();
    pieceTiles[1].push_back(tile2);
  } /* for */
  pieceTiles[0][0].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[0][1].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[0][2].moveobject(Vector(-0.5, -0.5, 0.2));
  pieceTiles[0][3].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[0][4].moveobject(Vector(-0.5, -0.45, 0.0));
  pieceTiles[0][5].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[0][6].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[0][7].moveobject(Vector(-0.32, -0.3, 0.0));
  pieceTiles[0][8].moveobject(Vector(-0.45, -0.45, 0.6));
  pieceTiles[0][9].moveobject(Vector(-0.4, -0.5, 0.0));
  pieceTiles[0][10].moveobject(Vector(-0.4, 0.2, 0.0));

  pieceTiles[1][0].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[1][1].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[1][2].moveobject(Vector(-0.5, -0.5, 0.2));
  pieceTiles[1][3].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[1][4].moveobject(Vector(-0.5, -0.45, 0.0));
  pieceTiles[1][5].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[1][6].moveobject(Vector(-0.5, -0.5, 0.0));
  pieceTiles[1][7].moveobject(Vector(-0.32, -0.3, 0.0));
  pieceTiles[1][8].moveobject(Vector(-0.45, -0.45, 0.6));
  pieceTiles[1][9].moveobject(Vector(-0.4, -0.5, 0.0));
  pieceTiles[1][10].moveobject(Vector(-0.4, 0.2, 0.0));

  bulletTiles.reserve(N_BULLETS);
  for (int i = 0; i < N_BULLETS; i++) {
    Piece3DObject tile(bullnames[i],"textures/");
    tile.normalize(bullscale[i]);
    bulletTiles.push_back(tile);
  }

  Vector lightposv;
  if (shadows == 1) {
    lightposv.x = -1000;
    lightposv.y = -3000;
    lightposv.z = 5000;
  } else {
    lightposv.x = 0;
    lightposv.y = 0;
    lightposv.z = 5000;
  }

  for (Shadow3DObject& tile: buildingTiles) tile.computeShadow(lightposv);
  for (int i = 0; i < N_PIECES; i++) {
    pieceTiles[0][i].computeFixedShadows(lightposv);
    pieceTiles[1][i].computeFixedShadows(lightposv);
  }
  for (Piece3DObject& tile: bulletTiles) tile.computeFixedShadows(lightposv);

  constructionTiles.emplace_back("models/construction1.asc","textures/");
  constructionTiles.emplace_back("models/construction2.asc","textures/");
  constructionTiles.emplace_back("models/construction3.asc","textures/");
  constructionTiles[0].normalize(10.0);
  constructionTiles[1].normalize(9.0);
  constructionTiles[2].normalize(7.0);

  messageTiles.reserve(3);
  messageTiles.emplace_back("models/go.ase","textures/");
  messageTiles.emplace_back("models/youwin.ase","textures/");
  messageTiles.emplace_back("models/gameover.ase","textures/");
  for (auto& tile: Resources::messageTiles) tile.normalize(4.0);
}


void Resources::deleteObjects()
{
  tiles.clear();
  buildingTiles.clear();
  pieceTiles[0].clear();
  pieceTiles[1].clear();
  constructionTiles.clear();
  messageTiles.clear();
  bulletTiles.clear();
}


Resources* Resources::singleton = NULL;

std::vector<Shadow3DObject> Resources::buildingTiles;
std::vector<Piece3DObject> Resources::pieceTiles[2];
std::vector<C3DObject> Resources::tiles;
std::vector<C3DObject> Resources::constructionTiles;
std::vector<C3DObject> Resources::messageTiles;
std::vector<Piece3DObject> Resources::bulletTiles;
