#ifndef NETHER_HEADER
#define NETHER_HEADER

#include <cstring>
#include <string>
#include "vector.h"
#include "list.h"
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
#include "ai_operator.h"
#include "menu.h"


const float COLISION_TEST_THRESHOLD = 9.0;
const int INTRO_TIME = 60;
const int END_TIME = 260;

/* BULLET SPEED: */
const float BULLET_SPEED = 0.1f;
const int CANNON_PERSISTENCE = 40;
const int MISSILE_PERSISTENCE = 60;
const int PHASER_PERSISTENCE = 40;

/* GAME STATES: */
enum GAME_STATE {STATE_PLAYING,
                 STATE_CONSTRUCTION,
                 STATE_PAUSE,
                 STATE_SAVINGGAME,
                 STATE_LOADINGGAME};

enum SHIP_OPERATORS {OP_NONE = -1,
                     OP_LEFT,
                     OP_RIGHT,
                     OP_FORWARD,
                     OP_BACKWARD,
                     OP_UP};

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

enum ROBOT_PROGRAMS {PROGRAM_NONE = -1,
                     PROGRAM_FORWARD,
                     PROGRAM_STOPDEFEND,
                     PROGRAM_ADVANCE,
                     PROGRAM_RETREAT,
                     PROGRAM_DESTROY,
                     PROGRAM_CAPTURE};

enum PROGRAM_PARAM {P_PARAM_ROBOTS = 1,
                    P_PARAM_WARBASES,
                    P_PARAM_NFACTORIES,
                    P_PARAM_EFACTORIES};

enum AI_STATES {AI_STATE_EXPANDING,
                AI_STATE_FIGHTING,
                AI_STATE_DEFENDING,
                AI_STATE_CONQUERING,
                AI_STATE_DESTROYING};


class NETHER {
public:
  NETHER(const std::string& mapname);
	~NETHER();

	void loadObjects();
	void deleteObjects();

	bool gamecycle(int w,int h);
	void gameredraw(int w,int h);
	void refresh_display_lists(void);

	bool save_game(char *filename);
  bool loadGame(const std::string& filename);
  bool saveDebugReport(const std::string& filename);

private:
	bool cycle(unsigned char *keyboard);
	bool construction_cycle(unsigned char *keyboard);
	bool option_cycle(unsigned char *keyboard);

	void draw(int w,int h);
	void draw_game(bool shadows);
	void draw_radar(void);
	void construction_draw(int w,int h);
	void options_draw(int w,int h);

    bool loadmap(const std::string& filename);
	void drawmap(bool shadows);

	float MapMaxZ(float x[2],float y[2]);
	int MapTerrain(float x, float y);
	int WorseMapTerrain(float x[2], float y[2]);

	bool ShipCollision(C3DObject *ship,float x,float y,float z);
	bool RobotCollision(Robot *r,bool complete);
	bool BulletCollision(Bullet *b,Robot **r);

	CMC  RobotCMC(Robot *r,int owner);
	CMC  BulletCMC(Bullet *r);

	float RobotSpeed(int traction,int terrain);
	int RobotRotationSpeed(int traction,int terrain);
	bool Walkable(int traction,int terrain);

	int SFX_volume(Vector pos);

	void AI_enemy(void);
	Robot *AI_enemy_newrobot(int state,Vector pos);
	void AI_precomputations(void);
	void AI_deleteprecomputations(void);
	void AI_release(void);
	int  AI_WorseMapTerrain(int x,int y,int dx,int dy);
	void AI_newrobot(Vector pos,int owner);
	int  AI_killrobot(Vector pos);
	void AI_moverobot(Vector oldpos,Vector newpos,int owner);
	void AI_removebuilding(Vector pos);
	void AI_availableoperators(Vector pos,int angle,int traction,List<AIOperator> *l);
	bool AI_expandoperators(int x,int y,int angle,int traction,int previous,int oldcost,int depth);
	int  AI_searchengine(Vector pos,int angle,int goaltype,Vector goalpos,int traction,int depth);
	void AI_resetsearch(Vector pos,int depth);
	int  AI_program_advance(int amount,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	int  AI_program_retreat(int amount,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	int  AI_program_capture(int goal,Vector *program_goal,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	int  AI_program_destroy(int goal,Vector *program_goal,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	int  AI_program_stopdefend(Vector *program_goal,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	void AI_rankoperators_advance(List<AIOperator> *l);
	void AI_rankoperators_retreat(List<AIOperator> *l);
	void AI_rankoperators_capture(List<AIOperator> *l,Vector goal);
	AIOperator *AI_chooseoperator(List<AIOperator> *l,int factor);
	int  AI_robothere(Vector pos);
	int  AI_RealShotPaths(int x,int y,int player,int persistence);

	/* Game variables: */
  Menu menu;
	int map_w,map_h;
	int *map;
	float lightpos[4];
	Vector lightposv;
	unsigned char old_keyboard[322];

	float zoom;
	Vector camera,viewp;
	Vector shipp;
	bool shiplanded;
	int ship_op,ship_op2,ship_op3;
	int ship_timemoving;

	List<Building> buildings;
	List<Robot> robots[2];
	List<Bullet> bullets;
	List<Explosion> explosions;
	List<Particle> particles;

	int day,hour,minute,second;
	int resources[2][7];
	int statistics[2][8];

	float animation_timer;
	int construction_pointer;
	bool construction[8];
	int game_state;
	Robot in_construction;
	Robot *controlled;

	int game_finished;
	int game_started;

	/* Graphics: */ 
	int n_objs;
	C3DObject **tile;
	float *tile_r,*tile_g,*tile_b;
	Shadow3DObject *ship;
	int n_buildings;
	Shadow3DObject **building_tile;
	int n_pieces;
	Piece3DObject **piece_tile[2];
	C3DObject *construction_tile[3];
	C3DObject *message_tile[3];
	int n_bullets;
	Piece3DObject **bullet_tile;

	/* Status variables: */
	int redrawradar;
	bool recomputestatistics;

	/* Option/Pause menu variables: */ 
	int option_menu;

	/* Artificial intelligence variables: */ 
	int *discreetmap;
	int *bk_discreetmap;
	AIOperator **searchmap;
	int *atackmap;

	/* Sonido: */ 
	Mix_Chunk *S_shot,*S_explosion,*S_select,*S_wrong,*S_construction;

  friend class Menu;
};

#endif
