#ifndef NETHER_HEADER
#define NETHER_HEADER

#include <SDL/SDL.h>
#include <array>
#include <cstring>
#include <string>

#include "3dobject.h"
#include "ai.h"
#include "ai_operator.h"
#include "buildingblock.h"
#include "bullet.h"
#include "camera.h"
#include "cmc.h"
#include "constructionscreen.h"
#include "explosion.h"
#include "light.h"
#include "menu.h"
#include "optionsscreen.h"
#include "particle.h"
#include "piece3dobject.h"
#include "radar.h"
#include "robot.h"
#include "scene.h"
#include "shadow3dobject.h"
#include "soundmanager.h"
#include "ship.h"
#include "stats.h"
#include "statusbutton.h"
#include "vector.h"


/* GAME STATES: */

class NETHER {
public:
  enum class STATE {
    PLAYING,
    CONSTRUCTION,
    PAUSE,
    SAVINGGAME,
    LOADINGGAME
  };

  explicit NETHER(const std::string& mapname);
  ~NETHER();

  bool gamecycle();
  void gameredraw(int w, int h);

  bool saveGame(const std::string& filename);
  bool loadGame(const std::string& filename);

  void requestStatsRecomputing() { stats.requestRecomputing(); }

  std::shared_ptr<Robot> getControlledRobot() const { return controlled; }

  STATE getGameState() const { return gameState; };
  void setGameState(STATE newState) { gameState = newState; }

  void setGameFinished(int time) { gameFinished = time; }
  void setGameStarted(int time) { gameStarted = time; }
  void increaseAnimationTimer(float delta) { animationTimer += delta; }
  float getAnimationTimer() const { return animationTimer; }
  void addNewRobot(std::shared_ptr<Robot> robot, int player);
  void detachShip(std::shared_ptr<Robot> robot);
  Menu::TYPE getActiveMenu() { return menu.getActiveMenu(); }

  std::array<std::pair<int, int>, 7> getBuildingStats() const;
  std::array<std::pair<int, int>, 7> getResourceStats() const;

  Scene scene;
  AI ai;
  SoundManager sManager;
  Stats stats;
  Light light;

private:
  unsigned char prevKeyboard[SDLK_LAST];
  bool cycle(unsigned char *keyboard);
  void draw(int w, int h);
  void drawGame(bool shadows);

  /* Game variables: */
  Menu menu;
  Radar radar;
  OptionsScreen optionsScreen;
  ConstructionScreen constructionScreen;

  Camera camera;

  STATE gameState;
  float animationTimer;
  int gameFinished;
  int gameStarted;

  std::shared_ptr<Robot> controlled;

  friend class AI;
};

#endif
