#ifndef NETHER_HEADER
#define NETHER_HEADER

#include <cstring>
#include <string>
#include "vector.h"


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

enum BUTTON_NAMES {TIME_BUTTON = 1,
                   STATUS_BUTTON,
                   RESOURCE_BUTTON,
                   ROBOT1_BUTTON,
                   ROBOT2_BUTTON,
                   ROBOT3_BUTTON,
                   ROBOT4_BUTTON,
                   COMBAT1_BUTTON,
                   COMBAT2_BUTTON,
                   COMBAT3_BUTTON,
                   COMBAT4_BUTTON,
                   COMBAT5_BUTTON,
                   COMBAT6_BUTTON,
                   ORDERS1_BUTTON,
                   ORDERS2_BUTTON,
                   ORDERS3_BUTTON,
                   ORDERS4_BUTTON,
                   ORDERS5_BUTTON,
                   ORDERS_BUTTON,
                   TARGET1_BUTTON,
                   TARGET2_BUTTON,
                   TARGET3_BUTTON};

enum MENU_TYPES {GENERAL_MENU,
                 ROBOT_MENU,
                 DIRECTCONTROL_MENU,
                 COMBATMODE_MENU,
                 DIRECTCONTROL2_MENU,
                 ORDERS_MENU,
                 SELECTDISTANCE_MENU,
                 TARGETD_MENU,
                 TARGETC_MENU,
                 ALL_MENUS};

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

enum BUILDINGS_AND_WALLS {B_FENCE,
                          B_WALL1,
                          B_WALL2,
                          B_WALL3,
                          B_WALL4,
                          B_WALL5,
                          B_WALL6,
                          B_FACTORY_ELECTRONICS,
                          B_FACTORY_NUCLEAR,
                          B_FACTORY_PHASERS,
                          B_FACTORY_MISSILES,
                          B_FACTORY_CANNONS,
                          B_FACTORY_CHASSIS,
                          B_WARBASE};

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


class StatusButton {
public:
  StatusButton(int ID, int x, int y, int sx, int sy, const std::string& text1,
               const std::string& text2, float r, float g, float b, int status):
    ID(ID), x(x), y(y), sx(sx), sy(sy), text1(text1), text2(text2), r(r), g(g), b(b), status(status)
  {}

  int ID;
  int x, y;
  int sx, sy;
  std::string text1;
  std::string text2;
  float r, g, b;
  int status;
};


class BUILDING {
public:
	int type;
	Vector pos;
	int owner;

	int status;		/* This variable controls the status of the building: if its free, or being captured	*/ 
					/* by some of the players.																*/ 
};


class ROBOT {
public:
	ROBOT();
	bool valid(void);
	float piecez(int piece);
	bool bullethit(int type); 

	int traction;
	bool pieces[5];

	int program;
	int program_parameter;
	Vector program_goal;

	int op;
	bool shipover;
	int firetimer;
	int strength;

	Vector pos;
	int angle;
	CMC cmc;

	/* Animation variables: */ 
	int electronics_state;
	int chassis_state;
};


class BULLET {
public:
	BULLET(void);
	BULLET(int t,Vector p,int a,ROBOT *r);

	int type;
	int step;

	Vector pos;
	int angle;
	ROBOT *owner;	/* The robot who fired this bullet */ 

	CMC cmc;
};


class EXPLOSION {
public:
	EXPLOSION(void);
	EXPLOSION(Vector p,int sz);

	Vector pos;
	int step;
	int size;
};


class PARTICLE {
public:
	PARTICLE(void);
	PARTICLE(Vector p,Vector speed1,Vector speed2,float sz1,float sz2,float r,float g,float b,float a1,float a2,int lifetime);

	Vector pos,speed1,speed2;
	float size1,size2;
	float r,g,b;
	float a1,a2;
	int lifetime,acttime;
};


class AI_OPERATOR {
public:
	Vector newpos;
	int first_robotop;
	int cost;
	int previous;
	bool deadend;
	bool used;
};


class NETHER {
public:
  NETHER(const std::string& mapname);
	~NETHER();

	void loadobjects();
	void deleteobjects();

	bool gamecycle(int w,int h);
	void gameredraw(int w,int h);
	void refresh_display_lists(void);

	bool save_game(char *filename);
	bool load_game(char *filename);
	bool save_debug_report(char *filename);

private:
	bool cycle(unsigned char *keyboard);
	bool construction_cycle(unsigned char *keyboard);
	bool option_cycle(unsigned char *keyboard);

	void draw(int w,int h);
	void draw_game(bool shadows);
	void draw_radar(void);
	void draw_status(void);
	void construction_draw(int w,int h);
	void options_draw(int w,int h);

    bool loadmap(const std::string& filename);
	void drawmap(bool shadows);

	float MapMaxZ(float x[2],float y[2]);
	int MapTerrain(float x, float y);
	int WorseMapTerrain(float x[2], float y[2]);

	bool ShipCollision(C3DObject *ship,float x,float y,float z);
	bool RobotCollision(ROBOT *r,bool complete);
	bool BulletCollision(BULLET *b,ROBOT **r);

	void DrawBullet(BULLET *bullet,bool shadows);
	void DrawRobot(ROBOT *robot,int owner,bool shadows);
	void RobotCost(int player,ROBOT *r,int *res);
	int  RobotCost(ROBOT *r);
	CMC  RobotCMC(ROBOT *r,int owner);
	CMC  BulletCMC(BULLET *r);
	void DrawParticle(PARTICLE *p);
	bool CycleParticle(PARTICLE *p);

	float RobotSpeed(int traction,int terrain);
	int RobotRotationSpeed(int traction,int terrain);
	bool Walkable(int traction,int terrain);

	int SFX_volume(Vector pos);

  void newbutton(int ID, int x, int y, int sx, int sy, const std::string& t1, const std::string& t2,
                 float r, float g, float b);
	void newbuttondelayed(int ID, int x, int y, int sx, int sy, const std::string& t1,
                          const std::string& t2, float r, float g, float b);
	void killbutton(int ID);
	StatusButton *getbutton(int ID);
	void newmenu(ushort menu);
	void killmenu(ushort menu);

	void AI_enemy(void);
	ROBOT *AI_enemy_newrobot(int state,Vector pos);
	void AI_precomputations(void);
	void AI_deleteprecomputations(void);
	void AI_release(void);
	int  AI_WorseMapTerrain(int x,int y,int dx,int dy);
	void AI_newrobot(Vector pos,int owner);
	int  AI_killrobot(Vector pos);
	void AI_moverobot(Vector oldpos,Vector newpos,int owner);
	void AI_removebuilding(Vector pos);
	void AI_availableoperators(Vector pos,int angle,int traction,List<AI_OPERATOR> *l);
	bool AI_expandoperators(int x,int y,int angle,int traction,int previous,int oldcost,int depth);
	int  AI_searchengine(Vector pos,int angle,int goaltype,Vector goalpos,int traction,int depth);
	void AI_resetsearch(Vector pos,int depth);
	int  AI_program_advance(int amount,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	int  AI_program_retreat(int amount,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	int  AI_program_capture(int goal,Vector *program_goal,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	int  AI_program_destroy(int goal,Vector *program_goal,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	int  AI_program_stopdefend(Vector *program_goal,Vector pos,int angle,int traction,bool electronics,int player,bool *pieces);
	void AI_rankoperators_advance(List<AI_OPERATOR> *l);
	void AI_rankoperators_retreat(List<AI_OPERATOR> *l);
	void AI_rankoperators_capture(List<AI_OPERATOR> *l,Vector goal);
	AI_OPERATOR *AI_chooseoperator(List<AI_OPERATOR> *l,int factor);
	int  AI_robothere(Vector pos);
	int  AI_RealShotPaths(int x,int y,int player,int persistence);

	/* Game variables: */ 
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

	List<BUILDING> buildings;
	List<ROBOT> robots[2];
	List<BULLET> bullets;
	List<EXPLOSION> explosions;
	List<PARTICLE> particles;

	int day,hour,minute,second;
	int resources[2][7];
	int statistics[2][8];

	float animation_timer;
	int construction_pointer;
	bool construction[8];
	int game_state;
	ROBOT in_construction;
	ROBOT *controlled;

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
	List<StatusButton> buttons;
	int act_menu;
	int act_button;
	int redrawmenu,redrawradar;
	bool recomputestatistics;

	/* Option/Pause menu variables: */ 
	int option_menu;

	/* Artificial intelligence variables: */ 
	int *discreetmap;
	int *bk_discreetmap;
	AI_OPERATOR **searchmap;
	int *atackmap;

	/* Sonido: */ 
	Mix_Chunk *S_shot,*S_explosion,*S_select,*S_wrong,*S_construction;
	

};

#endif
