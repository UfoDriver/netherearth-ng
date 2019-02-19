#ifndef NETHER_HEADER
#define NETHER_HEADER

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

enum ROBOT_OPERATORS {ROBOTOP_NONE = -1,
                      ROBOTOP_FORWARD,
                      ROBOTOP_LEFT,
                      ROBOTOP_RIGHT,
                      ROBOTOP_CANNONS,
                      ROBOTOP_MISSILES,
                      ROBOTOP_PHASERS,
                      ROBOTOP_NUCLEAR};


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

  Ship* getShip() const { return ship; }
  STATE getGameState() { return game_state; };
  void setGameState(STATE newState) { game_state = newState; }

private:
  bool cycle(unsigned char *keyboard);
  void draw(int w, int h);
  void drawGame(bool shadows);

  /* Game variables: */
  Ship *ship;
  Menu menu;
  Radar radar;
  SoundManager sManager;
  Map map;
  OptionsScreen optionsScreen;
  ConstructionScreen constructionScreen;
  AI ai;
  Light light;

  unsigned char old_keyboard[SDLK_LAST];

  Camera camera;
  Vector viewp;

  Stats stats;

  float animation_timer;

  STATE game_state;
  Robot* controlled;

  int game_finished;
  int game_started;

  friend class Menu;
  friend class Radar;
  friend class OptionsScreen;
  friend class ConstructionScreen;
  friend class AI;
};

#endif
