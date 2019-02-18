#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"

#include "GL/gl.h"
#include "GL/glu.h"
#include "GL/glut.h"
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "myglutaux.h"
#include "building.h"
#include "explosion.h"
#include "nether.h"
#include "menu.h"
#include "utils.h"

#include "glprintf.h"

extern int frames_per_sec;
extern bool fullscreen;
extern int shadows;
extern bool sound;
extern int up_key,down_key,left_key,right_key,fire_key,pause_key;
extern int level;
extern float MINY,MAXY,MINX,MAXX;
extern bool show_radar;


#ifdef _WRITE_REPORT_
FILE *debug_fp=0;
#endif


NETHER::NETHER(const std::string& mapname): menu(this), radar(this), camera(0, 0, 0, 0),
                                            controlled(NULL)
{
#ifdef _WRITE_REPORT_
	debug_fp=fopen("report.txt","w");
	fprintf(debug_fp,"Creating game...\n");
	fflush(debug_fp);
#endif
	if (shadows==1) {
		lightpos[0]=-1000;
		lightpos[1]=-3000;
		lightpos[2]=5000;
		lightpos[3]=1;
		lightposv.x=lightpos[0];
		lightposv.y=lightpos[1];
		lightposv.z=lightpos[2];
	} else {
		lightpos[0]=0;
		lightpos[1]=0;
		lightpos[2]=5000;
		lightpos[3]=1;
		lightposv.x=lightpos[0];
		lightposv.y=lightpos[1];
		lightposv.z=lightpos[2];
	} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"loading objects...\n");
	fflush(debug_fp);
#endif

    Resources::instance()->loadObjects();

    ship = new Ship("models/ship.asc", "textures/");
    ship->ComputeShadow(lightposv);

		
#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"loading map...\n");
	fflush(debug_fp);
#endif

	/* Load map: */ 
	map.loadMap(mapname);

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Initializing game variables...\n");
	fflush(debug_fp);
#endif

	/* Set camera: */ 
	viewp.x = map.width() / 2;
	viewp.y=0;
	camera.x=6;
	camera.y=-6;
	camera.z=11;
	camera.zoom=1;

	game_state=NETHER::STATE::PLAYING;
	animation_timer=0;
	construction_pointer=0;
	controlled=0;
	game_finished=0;
	game_started=INTRO_TIME;

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Creating menus...\n");
	fflush(debug_fp);
#endif

	/* Init status: */ 
	menu.newmenu(Menu::TYPE::GENERAL);
	menu.needsRedraw=2;
	radar.needsRedraw=1;

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Initializing AI...\n");
	fflush(debug_fp);
#endif

	/* Init AI: */ 
	AI_precomputations();

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Game created.\n");
	fflush(debug_fp);
#endif

} /* NETHER::NETHER */ 


NETHER::~NETHER()
{
  for (Robot* r: map.robots[0]) delete r;
  for (Robot* r: map.robots[1]) delete r;

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Destroying Game...\n");
	fflush(debug_fp);
#endif

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Deleting sounds...\n");
	fflush(debug_fp);
#endif

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Deleting objects...\n");
	fflush(debug_fp);
#endif

    delete ship;
    ship = 0;
    Resources::instance()->deleteObjects();

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Deleting AI...\n");
	fflush(debug_fp);
#endif

	AI_deleteprecomputations();

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Deleting map...\n");
	fflush(debug_fp);
#endif

	/* Delete map: */ 
	map.resize(0, 0);

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Game destroyed.\n");
	fclose(debug_fp);
#endif

} /* NETHER::~NETHER */ 


bool NETHER::gamecycle()
{
  bool retval = true;

  SDL_PumpEvents();
  unsigned char* keyboard = SDL_GetKeyState(NULL);

  switch(game_state) {
  case NETHER::STATE::PLAYING:
    retval = cycle(keyboard);
    break;
  case NETHER::STATE::CONSTRUCTION:
    retval = construction_cycle(keyboard);
    break;
  case NETHER::STATE::PAUSE:
  case NETHER::STATE::SAVINGGAME:
  case NETHER::STATE::LOADINGGAME:
    retval = option_cycle(keyboard);
    break;
  }

  for (int i = 0; i < SDLK_LAST; i++)
    old_keyboard[i] = keyboard[i];

  return retval;
}


void NETHER::gameredraw(int w,int h)
{
  switch(game_state) {
  case NETHER::STATE::PLAYING:
    draw(w, h);
    break;
  case NETHER::STATE::CONSTRUCTION:
    constructionDraw(w, h);
    break;
  case NETHER::STATE::PAUSE:
  case NETHER::STATE::SAVINGGAME:
  case NETHER::STATE::LOADINGGAME:
    draw(w, h);
    optionsDraw(w, h);
    break;
  }

  SDL_GL_SwapBuffers();
}


void NETHER::draw(int width, int height)
{
  float lightpos2[4] = {0, 0, 1000, 0};
  float tmpls[4] = {1.0F, 1.0F, 1.0F, 1.0};
  float tmpld[4] = {0.6F, 0.6F, 0.6F, 1.0};
  float tmpla[4] = {0.2F, 0.2F, 0.2F, 1.0};
  float ratio;
  int split = int((width * 25.0F) / 32.0F);
  int splity = 0;

  if (show_radar)
    splity = int((height * 2.0F) / 15.0F) + 1;
  else
    splity = 0;

  /* Enable Lights, etc.: */
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, tmpla);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, tmpld);
  glLightfv(GL_LIGHT0, GL_SPECULAR, tmpls);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glEnable(GL_SCISSOR_TEST);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glClearStencil(0);

  /* Draw the GAME screen: */
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  glClearColor(0, 0, 0, 0.0);
  glViewport(0, splity, split, height - splity);
  ratio = float(split) / float(height - splity);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, ratio, 1.0, 1024.0);
  glScissor(0, splity, split, height - splity);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  drawGame(false);
  if (shadows) {
    /* Set STENCIL Buffer: */
    glStencilMask(1);
    glEnable(GL_STENCIL_TEST);
    glDepthMask(GL_FALSE);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    drawGame(true);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    /* Draw shadow poligon: */
    glDepthFunc(GL_ALWAYS);
    glDisable(GL_CULL_FACE);

    glColor4f(0.0, 0.0, 0.0, 0.4f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glStencilFunc(GL_NOTEQUAL, 0, 1);
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glBegin(GL_TRIANGLE_STRIP);
    glVertex3f(-1.0, 1.0, 0.0);
    glVertex3f(-1.0, -1.0, 0.0);
    glVertex3f(1.0, 1.0, 0.0);
    glVertex3f(1.0, -1.0, 0.0);
    glEnd();
    glPopMatrix();

    glDisable(GL_BLEND);

    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glDisable(GL_STENCIL_TEST);
  }

  if (game_started > 0) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, ratio, 1.0, 1024.0);
    gluLookAt(0,0, 30, 0, 0, 0, 0, 1, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (game_started > 40) glTranslatef(0, 0, (game_started - 40) * 2);
    if (game_started < 20) glTranslatef(0, 0, (20 - game_started) * 2);
    Resources::messageTiles[0].draw(Color(1.0, 1.0, 1.0));
  }

  if (game_finished > 100) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(30.0, ratio, 1.0, 1024.0);
    gluLookAt(0, 0, 30, 0, 0, 0, 0, 1,0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    if (game_finished < 120) glTranslatef(0, 0, (120 - game_finished) * 2);
    if (game_finished > 240) glTranslatef(0, 0, (game_finished - 240) * 2);
    if (stats.stats[0][0] == 0) Resources::messageTiles[2].draw(Color(1.0, 1.0, 1.0));
    else Resources::messageTiles[1].draw(Color(1.0, 1.0, 1.0));
  }

  /* Draw the RADAR screen: */
  if (show_radar && radar.needsRedraw <= 1) {

    glLightfv(GL_LIGHT0, GL_POSITION, lightpos2);
    glClearColor(0.0, 0.0, 0,0);
    glViewport(0, 0, split, splity);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, float(split), 0, float(splity), -100, 100);
    glScissor(0, 0, split, splity);
    glScalef(width / 640.0, height / 480.0, 1);
    radar.draw();
  }
  radar.needsRedraw--;
  if (radar.needsRedraw < 0) radar.needsRedraw = 3;

  menu.draw(width, height);
}


void NETHER::drawGame(bool shadows)
{
  MINY =- 8 * camera.zoom;
  MINX =- (10 + viewp.z * 4) * camera.zoom;
  MAXY = (9 + viewp.z * 4) * camera.zoom;
  MAXX = 8 * camera.zoom;

  /* Draw the map: */
  {
    Vector light(lightpos[0], lightpos[1], lightpos[2]);
    light=light / light.z;

    map.draw(viewp, shadows, light, camera);
  }

  /* Draw the robots and bullets: */
  {
    for(int i = 0; i < 2; i++) {
      for (Robot* r: map.robots[i]) {
        if (r->pos.y >= (viewp.y + MINY) &&
            r->pos.y <= (viewp.y + MAXY) &&
            r->pos.x >= (viewp.x + MINX) &&
            r->pos.x <= (viewp.x + MAXX)) {
          glPushMatrix();
          glTranslatef(r->pos.x, r->pos.y, r->pos.z);
          r->draw(i, shadows, Resources::pieceTiles, lightposv);
          glPopMatrix();
        }
      }
    }
  }

  /* Draw the ship: */
  glPushMatrix();
  glTranslatef(ship->pos.x, ship->pos.y, ship->pos.z);
  if (!shadows) ship->draw(Color(0.7, 0.7, 0.7));
  glPopMatrix();

  if (shadows) {
    float sx, sy;
    float minz;
    Vector light;

    light = lightposv;
    light = light / light.z;

    sx = ship->pos.x - light.x * ship->pos.z;
    sy = ship->pos.y - light.y * ship->pos.z;

    if (controlled == 0) {
      float x[2], y[2];
      x[0] = sx + ship->shdw_cmc.x[0];
      x[1] = sx + ship->shdw_cmc.x[1];
      y[0] = sy + ship->shdw_cmc.y[0];
      y[1] = sy + ship->shdw_cmc.y[1];
      minz = map.maxZ(x, y);
    } else {
      minz = controlled->pos.z;
    }

    glPushMatrix();
    glTranslatef(sx, sy, minz+0.05);
    ship->DrawShadow(Color(0, 0, 0, 0.5));
    glPopMatrix();
  }

  /* Draw the extras: */
}


void NETHER::optionsDraw(int w, int h)
{
  float tmpls[4] = {1.0F, 1.0F, 1.0F, 1.0};
  float tmpld[4] = {0.6F, 0.6F, 0.6F, 1.0};
  float tmpla[4] = {0.2F, 0.2F, 0.2F, 1.0};
  int splitx[2] = {int(w * 0.3), int(w * 0.7)};
  int splity[2] = {int(h * 0.3), int(h * 0.7)};

  /* Enable Lights, etc.: */
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
  glEnable(GL_LIGHT0);
  glLightfv(GL_LIGHT0, GL_AMBIENT, tmpla);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, tmpld);
  glLightfv(GL_LIGHT0, GL_SPECULAR, tmpls);
  glEnable(GL_LIGHTING);
  glEnable(GL_COLOR_MATERIAL);
  glShadeModel(GL_SMOOTH);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);
  glEnable(GL_CULL_FACE);
  glEnable(GL_SCISSOR_TEST);
  glEnable(GL_DEPTH_TEST);

  /* Draw the MENU: */
  glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
  glClearColor(0, 0, 0, 0.0);
  glViewport(splitx[0], splity[0], splitx[1] - splitx[0], splity[1] - splity[0]);
  float ratio = float(splitx[1] - splitx[0]) / float(splity[1] - splity[0]);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(30.0, ratio, 1.0, 1024.0);
  glScissor(splitx[0], splity[0], splitx[1] - splitx[0], splity[1] - splity[0]);
  gluLookAt(0, 0, 30, 0, 0, 0, 0, 1, 0);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (game_state == NETHER::STATE::PAUSE) {
    if (option_menu == 0)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, 5, 0);
    scaledglprintf(0.01, 0.01, "RETURN TO GAME");

    if (option_menu == 1)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -5,0);
    scaledglprintf(0.01, 0.01, "LOAD GAME");

    if (option_menu == 2)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -3, 0);
    scaledglprintf(0.01, 0.01, "SAVE GAME");

    if (option_menu == 3)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -3, 0);
    scaledglprintf(0.01, 0.01, "QUIT GAME");
  }

  if (game_state == NETHER::STATE::SAVINGGAME) {
    glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, 6, 0);
    scaledglprintf(0.01, 0.01, "CHOOSE SLOT TO SAVE");

    if (option_menu == 0)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -4, 0);
    scaledglprintf(0.01, 0.01, "CANCEL");

    for (int i = 0;i < 4; i++) {
      if (option_menu == (i + 1))
        glColor3f(1.0, 0.0, 0.0);
      else
        glColor3f(0.5, 0.5, 1.0);
      glTranslatef(0, -2,0);
      char filename[80];
      sprintf(filename, "savedgame%i.txt", i);
      FILE *fp = fopen(filename, "r");
      if (fp == 0) {
        scaledglprintf(0.01, 0.01, "SLOT%i - EMPTY", i + 1);
      } else {
        scaledglprintf(0.01, 0.01, "SLOT%i - GAME SAVED", i + 1);
        fclose(fp);
      }
    }
  }

  if (game_state == NETHER::STATE::LOADINGGAME) {
    glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, 6, 0);
    scaledglprintf(0.01, 0.01, "CHOOSE SLOT TO LOAD");

    if (option_menu == 0)
      glColor3f(1.0, 0.0, 0.0);
    else
      glColor3f(0.5, 0.5, 1.0);
    glTranslatef(0, -4,0);
    scaledglprintf(0.01, 0.01, "CANCEL");

    for (int i = 0; i < 4; i++) {
      if (option_menu == (i + 1))
        glColor3f(1.0, 0.0, 0.0);
      else
        glColor3f(0.5, 0.5, 1.0);
      glTranslatef(0, -2,0);
      char filename[80];
      sprintf(filename, "savedgame%i.txt", i);
      FILE *fp = fopen(filename, "r");
      if (fp == 0) {
        scaledglprintf(0.01, 0.01, "SLOT%i - EMPTY", i + 1);
      } else {
        scaledglprintf(0.01, 0.01, "SLOT%i - GAME SAVED", i + 1);
        fclose(fp);
      }
    }
  }
}


bool NETHER::option_cycle(unsigned char *keyboard)
{
	switch(game_state) {
	case NETHER::STATE::PAUSE:
		if (keyboard[fire_key] && !old_keyboard[fire_key]) {
			switch(option_menu) {
			case 0:
              game_state=NETHER::STATE::PLAYING;
					break;
			case 1: game_state=NETHER::STATE::LOADINGGAME;
					option_menu=0;
					break;
			case 2:	game_state=NETHER::STATE::SAVINGGAME;
					option_menu=0;
					break;
			case 3:	return false;
					break;
			} /* if */ 
		} /* if */ 

		if (keyboard[down_key] && !old_keyboard[down_key]) {
			option_menu++;
			if (option_menu>=4) option_menu=0;
		} /* if */ 

		if (keyboard[up_key] && !old_keyboard[up_key]) {
			option_menu--;
			if (option_menu<0) option_menu=3;
		} /* if */ 
		break;

	case NETHER::STATE::SAVINGGAME:
		if (keyboard[fire_key] && !old_keyboard[fire_key]) {
			switch(option_menu) {
			case 0:
              game_state=NETHER::STATE::PAUSE;
					option_menu=2;
					break;
			case 1: 
			case 2:
			case 3:
			case 4:
				{
					char filename[80];
					sprintf(filename,"savedgame%i.txt",option_menu-1);
					saveGame(filename);
					saveDebugReport("debugreport.txt");
					game_state=NETHER::STATE::PAUSE;
					option_menu=2;
					menu.needsRedraw=2;
					radar.needsRedraw=1;
				}
			} /* if */ 
		} /* if */ 

		if (keyboard[down_key] && !old_keyboard[down_key]) {
			option_menu++;
			if (option_menu>=5) option_menu=0;
		} /* if */ 

		if (keyboard[up_key] && !old_keyboard[up_key]) {
			option_menu--;
			if (option_menu<0) option_menu=4;
		} /* if */ 
		break;

	case NETHER::STATE::LOADINGGAME:
		if (keyboard[fire_key] && !old_keyboard[fire_key]) {
			switch(option_menu) {
			case 0:
              game_state=NETHER::STATE::PAUSE;
					option_menu=1;
					break;
			case 1: 
			case 2:
			case 3:
			case 4:
				{
					char filename[80];
					sprintf(filename,"savedgame%i.txt",option_menu-1);
					menu.killmenu(menu.act_menu);
					loadGame(filename);
					menu.newmenu(menu.act_menu);
					menu.needsRedraw=2;
					stats.requestRecomputing();
					game_finished=0;
					game_started=INTRO_TIME;
					game_state=NETHER::STATE::PAUSE;
					option_menu=2;
				}
			} /* switch */ 
		} /* if */ 

		if (keyboard[down_key] && !old_keyboard[down_key]) {
			option_menu++;
			if (option_menu>=5) option_menu=0;
		} /* if */ 

		if (keyboard[up_key] && !old_keyboard[up_key]) {
			option_menu--;
			if (option_menu<0) option_menu=4;
		} /* if */ 
		break;
	} /* switch */ 

	return true;
} /* NETHER::option_cycle */ 


bool NETHER::saveGame(const std::string& filename)
{
  std::ofstream oFile(filename);

  oFile << map.width() << ' ' << map.height() << '\n';

  for (int i = 0; i < map.height(); i++) {
    for (int j = 0; j < map.width(); j++) {
      oFile << map.map[j + i * map.width()] << ' ';
    }
    oFile << '\n';
  }

  oFile << std::setw(8) << lightpos[0] << ' '
        << std::setw(8) << lightpos[1] << ' '
        << std::setw(8) << lightpos[2] << ' '
        << std::setw(8) << lightpos[3] << '\n';
  oFile << lightposv
        << camera
        << viewp
        << ship->pos;

  oFile << ship->landed << '\n';

  oFile << map.buildings.size() << '\n';
  for (Building& b: map.buildings) {
    oFile << b;
  }

  for (int i = 0; i < 2; i++) {
    oFile << map.robots[i].size() << '\n';
    for (Robot* r: map.robots[i]) {
      oFile << *r;
    }
  }

  oFile << map.bullets.size() << '\n';
  for (Bullet& bullet: map.bullets) {
    oFile << std::make_pair(bullet, map.robots);
  }

  oFile << map.explosions.size() << '\n';
  for (Explosion& e: map.explosions) {
    oFile << e;
  }

  oFile << stats;

  oFile << find_index(map.robots[0], controlled) << '\n';
  oFile << int(menu.act_menu) << ' ' << int(menu.act_button) << std::endl;

  return true;
}


bool NETHER::loadGame(const std::string& filename)
{
  int mapWidth, mapHeight;
  std::ifstream inFile(filename);

  AI_deleteprecomputations();

  inFile >> mapWidth >> mapHeight;

  map.explosions.clear();
  map.buildings.clear();
  for (int i = 0; i < 2; i++) {
    for (Robot* r: map.robots[i]) delete r;
    map.robots[i].clear();
  }
  map.bullets.clear();
  map.resize(mapWidth, mapHeight);
  for (int i = 0; i < mapHeight; i++) {
    for (int j = 0; j < mapWidth; j++) {
      int tile;
      inFile >> tile;
      map.map.push_back(tile);
    }
  }

  inFile >> lightpos[0] >> lightpos[1] >> lightpos[2] >> lightpos[3];
  inFile >> lightposv
         >> camera
         >> viewp
         >> ship->pos;

  inFile >> ship->landed;

  int length;
  inFile >> length;
  for (int k = 0; k < length; k++) {
    map.buildings.push_back(Building(inFile));
  }

  for (int i = 0; i < 2; i++) {
    inFile >> length;
    for (int k = 0; k < length; k++) {
      map.robots[i].push_back(new Robot(inFile));
    }
  }

  inFile >> length;
  for (int k = 0; k < length; k++) {
    map.bullets.emplace_back(inFile, map.robots);
  }

  inFile >> length;
  for (int k = 0; k < length; k++) {
    map.explosions.emplace_back(inFile);
  }

  inFile >> stats;

  int i;
  inFile >> i;
  if (i >= 0)
    controlled = map.robots[0][i];
  else
    controlled = 0;

  int actMenu_, actButton_;
  inFile >> actMenu_ >> actButton_;
  menu.act_menu = Menu::TYPE(actMenu_);
  menu.act_button = StatusButton::NAME(actButton_);

  AI_precomputations();
  return true;
}


bool NETHER::saveDebugReport(const std::string& filename)
{
  std::ofstream log(filename);
  log << "NETHER EARTH NG Debug Report\n\n";
  log << "MAPW: " << map.width() << "\nMAPH: " << map.height() << '\n';
  log << "MAP:\n";
  for (int i = 0; i < map.height(); i++) {
    for (int j = 0; j < map.width(); j++) {
      log << map.map[j + i * map.width()] << ' ';
    }
    log << '\n';
  }

  log << "LIGHTPOS: " << lightpos[0] << ' '
      << lightpos[1] << ' '
      << lightpos[2] << ' '
      << lightpos[3] << '\n';
  log << "LIGHTPOSV: " << lightposv;
  log << "CAMERA: " << camera;
  log << "VIEWP: " << viewp;
  log << "SHIPP: " << ship->pos;
  if (ship->landed)
    log << "SHIP LANDED\n";
  else
    log << "SHIP NOT LANDED\n";

  log << "# OF BUILDINGS: " << map.buildings.size() << '\n';
  for (const Building& b: map.buildings) {
    log << "BUILDING:\n"
        << " TYPE: " << int(b.type)
        << "\n OWNER: " << b.owner
        << "\n STATUS: " << b.status << "\n"
        << b.pos;
  }

  for (int i = 0; i < 2; i++) {
    log << "\n# OF ROBOTS PLAYER " << i << ": " << map.robots[i].size() << '\n';

    const char* tractions[3] = {"BIPOD", "TRACKS", "ANTIGRAV"};
    const char* pieces[5] = {"CANNONS", "MISSILES", "PHASERS", "NUCLEAR", "ELECTRONICS"};
    for (Robot* r: map.robots[i]) {
      log << "ROBOT:\n";
      log << ' ' << tractions[r->traction] << '\n';
      for (int j = 0; j < 5; j++) {
        if (r->pieces[j])
          log << ' ' << pieces[j] << '\n';
      }
      log << " PROGRAM: " << r->program << '\n';
      log << " PROGRAM PARAMETER: " << r->program_parameter.as_int << '\n';
      log << " PROGRAM GOAL: ";
      log << r->program_goal;
           log << " ACTUAL OPERATOR: " << r->op << '\n';
      if (r->shipover)
        log << " HAS THE SHIP OVER IT\n";
      else
        log << " HAS NO SHIP OVER IT\n";
      log << " FIRETIMER: " << r->firetimer << "\n STRENGTH: " << r->strength << '\n';
      log << " POSITION: ";
      log << r->pos;
      log << " ANGLE: " << r->angle << '\n';
      log << " MINIMUM CONTAINER BOX:\n";
      log << r->cmc;
      log << " ELECTRONICS STATE: " << r->electronics_state
          <<"\n CHASSIS STATE: " << r->chassis_state;
      log << "\n\n";
    }
  }

  log << "\n# BULLETS: " << map.bullets.size() << '\n';
  for (Bullet& bullet: map.bullets) {
    log << " BULLET:\n TYPE: " << int(bullet.type)
        << "\n STEP: " << bullet.step
        << "\n ANGLE: " << bullet.angle << '\n';
    log << " POSITION: ";
    log << bullet.pos;

    if (std::count(map.robots[0].cbegin(), map.robots[0].cend(), bullet.owner)) {
      log << " OWNER: PLAYER 0 ROBOT " << bullet.owner->getId() << '\n';
    } else if (std::count(map.robots[1].cbegin(), map.robots[1].cend(), bullet.owner)) {
      log << " OWNER: PLAYER 1 ROBOT " << bullet.owner->getId() << '\n';
    }

    log << " MINIMUM CONTAINER BOX: \n";
    log << bullet.cmc << '\n';
  }

  log << "# EXPLOSIONS " << map.explosions.size() << '\n';
  for (Explosion& e: map.explosions) {
    log << "EXPLOSION:\n POSITION:\n";
    log << e.pos;
    log << " STEP: " << e.step << "\n SIZE: " << e.size << "\n\n";
  }

  log << "\nTIME: DAY " << stats.day << ' ' << stats.hour << ':' << stats.minute << ':' << stats.second << '\n';
  log << "\nRESOURCES:\n";

  for(int i = 0; i < 2; i++) {
    log << "  PLAYER " << i << ": ";
    for(int j = 0; j < 7; j++) {
      log << stats.resources[i][j] << ' ';
    }
    log << '\n';
  }

  log << "\nROBOT UNDER CONTROL: ";
  if (controlled)
    log << controlled->getId() << '\n';
  else
    log << "None\n";
  log << "\nMENU " << int(menu.act_menu) << "\nACT BUTTON: " << int(menu.act_button) << '\n';

  return true;
}


bool Map::cycle()
{
  // Keyboard/zoom
  // ship movement
  // robot menu process/redrawing
  // viewport calculation
  // Status redraw
  // Ship on the factory/robot handling
  // Robots cycling
  // Building cycling (capturing)
  // Bullets cycling
  // Nuclear explosion stepping/removal
  // Menu cycling
  // Game goals checking

  explosions.erase(std::remove_if(explosions.begin(), explosions.end(),
                                  [](auto& exp) { return !exp.cycle(); }),
                   explosions.end());

  particles.erase(std::remove_if(particles.begin(), particles.end(),
                                 [](auto& particle) { return !particle.cycle(); }),
                  particles.end());

  return true;
}
