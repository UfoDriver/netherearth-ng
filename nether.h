#ifndef NETHER_HEADER
#define NETHER_HEADER

#include <array>
#include <cstring>
#include <string>
#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "building.h"
#include "statusbutton.h"
#include "robot.h"
#include "bullet.h"
#include "ai.h"
#include "explosion.h"
#include "particle.h"
#include "ship.h"
#include "camera.h"
#include "map.h"
#include "light.h"
#include "ai_operator.h"
#include "menu.h"
#include "radar.h"
#include "stats.h"
#include "optionsscreen.h"
#include "constructionscreen.h"
#include "soundmanager.h"
#include "SDL/SDL.h"


/* GAME STATES: */

enum TERRAINS {T_GRASS,
               T_SAND,
               T_MOUNTAINS,
               T_HOLE,
               T_BUILDING,
               T_SHIP,
               T_ROBOT,
               T_EROBOT,
               T_OUT};

enum RESOURCES {R_GENERAL,
                R_ELECTRONICS,
                R_NUCLEAR,
                R_PHASERS,
                R_MISSILES,
                R_CANNONS,
                R_CHASSIS};


class NETHER {
public:
  enum class STATE {PLAYING,
                    CONSTRUCTION,
                    PAUSE,
                    SAVINGGAME,
                    LOADINGGAME};

  explicit NETHER(const std::string& mapname);
  ~NETHER();

  bool gamecycle();
  void gameredraw(int w, int h);

  bool saveGame(const std::string& filename);
  bool loadGame(const std::string& filename);
  bool saveDebugReport(const std::string& filename);

  void redrawMenu() { menu.requestRedraw(); }
  void redrawRadar() { radar.requestRedraw(); }
  void requestStatsRecomputing() { stats.requestRecomputing(); }

  Robot* getControlled() const { return controlled; }
  Ship* getShip() const { return ship; }
  STATE getGameState() const { return gameState; };
  void setGameState(STATE newState) { gameState = newState; }
  void setGameFinished(int time) { gameFinished = time; }
  void setGameStarted(int time) { gameStarted = time; }
  void increaseAnimationTimer(float delta) { animationTimer += delta; }
  float getAnimationTimer() const { return animationTimer; }
  void addNewRobot(Robot* robot, int player);

  std::pair<int, int> getRobotsCount() const;
  std::array<std::pair<int, int>, 7> getBuildingStats() const;
  std::array<std::pair<int, int>, 7> getResourceStats() const;

  Map map;
  AI ai;
  SoundManager sManager;

  unsigned char old_keyboard[SDLK_LAST];

private:
  bool cycle(unsigned char *keyboard);
  void draw(int w, int h);
  void drawGame(bool shadows);

  /* Game variables: */
  Ship *ship;
  Menu menu;
  Radar radar;
  OptionsScreen optionsScreen;
  ConstructionScreen constructionScreen;
  Light light;

  Camera camera;
  Vector viewp;

  Stats stats;

  STATE gameState;
  float animationTimer;
  int gameFinished;
  int gameStarted;

  Robot* controlled;

  friend class ConstructionScreen;
  friend class AI;
};

#endif
