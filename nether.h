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
#include "explosion.h"
#include "particle.h"
#include "ship.h"
#include "ai_operator.h"
#include "menu.h"
#include "radar.h"
#include "stats.h"
#include "soundmanager.h"
#include "SDL/SDL.h"


const int INTRO_TIME = 60;
const int END_TIME = 260;

/* BULLET SPEED: */
const float BULLET_SPEED = 0.1f;
const int CANNON_PERSISTENCE = 40;
const int MISSILE_PERSISTENCE = 60;
const int PHASER_PERSISTENCE = 40;

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

enum AI_STATES {AI_STATE_EXPANDING,
                AI_STATE_FIGHTING,
                AI_STATE_DEFENDING,
                AI_STATE_CONQUERING,
                AI_STATE_DESTROYING};


class NETHER {
public:
  enum class STATE {PLAYING,
                    CONSTRUCTION,
                    PAUSE,
                    SAVINGGAME,
                    LOADINGGAME};

  explicit NETHER(const std::string& mapname);
  ~NETHER();

  void loadObjects();
  void deleteObjects();

  bool gamecycle();
  void gameredraw(int w, int h);
  void refreshDisplayLists();

  bool saveGame(const std::string& filename);
  bool loadGame(const std::string& filename);
  bool saveDebugReport(const std::string& filename);

private:
	bool cycle(unsigned char *keyboard);
	bool construction_cycle(unsigned char *keyboard);
	bool option_cycle(unsigned char *keyboard);

  void draw(int w, int h);
  void drawGame(bool shadows);
  void constructionDraw(int w, int h);
  void optionsDraw(int w, int h);

  bool loadMap(const std::string& filename);
	void drawmap(bool shadows);

	float MapMaxZ(float x[2],float y[2]);
	int MapTerrain(float x, float y);
	int WorseMapTerrain(float x[2], float y[2]);

	bool robotCollision(Robot *r,bool complete);

	void AI_enemy();
	Robot* AI_enemy_newrobot(int state,Vector pos);
	void AI_precomputations();
	void AI_deleteprecomputations();
	void AI_release();
	int  AI_WorseMapTerrain(int x,int y,int dx,int dy);
	void AI_newrobot(Vector pos,int owner);
	int  AI_killrobot(Vector pos);
	void AI_moverobot(Vector oldpos,Vector newpos,int owner);
	void AI_removebuilding(Vector pos);
  void AI_availableoperators(const Robot& robot, std::vector<AIOperator>& l);
  bool AI_expandoperators(int x, int y, int angle, const Robot& robot, int previous, int oldcost, int depth);
  int  AI_searchengine(const Robot& robot, int goaltype, Vector goalpos, int depth);
  void AI_resetsearch(Vector pos, int depth);
  int  AI_program_advance(Robot& robot, int player);
  int  AI_program_retreat(Robot& robot, int player);
  int  AI_program_capture(Robot& robot, Vector *program_goal, int player);
  int  AI_program_destroy(Robot& robot, Vector *program_goal, int player);
  int  AI_program_stopdefend(Robot& robot, Vector *program_goal, int player);
  void AI_rankoperators_advance(std::vector<AIOperator>& l);
  void AI_rankoperators_retreat(std::vector<AIOperator>& l);
  void AI_rankoperators_capture(std::vector<AIOperator>&l, Vector goal);
  const AIOperator AI_chooseoperator(std::vector<AIOperator>& l, int factor);
	int  AI_robothere(Vector pos);
	int  AI_RealShotPaths(int x,int y,int player,int persistence);

	/* Game variables: */
  Menu menu;
  Radar radar;
  SoundManager sManager;

  int map_w, map_h;
  std::vector<int> map;
  float lightpos[4];
  Vector lightposv;
  unsigned char old_keyboard[SDLK_LAST];

  float zoom;
  Vector camera, viewp;

  std::vector<Building> buildings;
  std::vector<Robot*> robots[2];
  std::vector<Bullet> bullets;
  std::vector<Explosion> explosions;
  std::vector<Particle> particles;
  Ship *ship;

  Stats stats;

  float animation_timer;
  int construction_pointer;
  bool construction[8];
  STATE game_state;
  Robot in_construction;
  Robot* controlled;

  int game_finished;
  int game_started;

  /* Option/Pause menu variables: */
  int option_menu;

  /* Artificial intelligence variables: */
  std::vector<int> discreetmap;
  std::vector<int> bk_discreetmap;
  std::vector<AIOperator> searchmap;
  std::vector<int> attackmap;

  friend class Menu;
  friend class Radar;
};

#endif
