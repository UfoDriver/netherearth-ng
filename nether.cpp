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

const int N_BUILDINGS = 9;
const int N_BULLETS = 3;
const int N_OBJECTS = 12;
const int N_PIECES = 11;


#ifdef _WRITE_REPORT_
FILE *debug_fp=0;
#endif


NETHER::NETHER(const std::string& mapname): menu(this), radar(this), controlled(NULL)
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

	loadObjects();
		
#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"loading map...\n");
	fflush(debug_fp);
#endif

	/* Load map: */ 
	if (!loadMap(mapname)) {
		map_w=map_h=0;
		map.clear();
	} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Initializing game variables...\n");
	fflush(debug_fp);
#endif

	/* Set camera: */ 
	viewp.x=map_w/2;
	viewp.y=0;
	camera.x=6;
	camera.y=-6;
	camera.z=11;
	zoom=1;

	/* Init game: */ 
	day=0;
	hour=0;
	minute=0;
	second=0;

	resources[0][0]=20;
	resources[0][1]=0;
	resources[0][2]=0;
	resources[0][3]=0;
	resources[0][4]=0;
	resources[0][5]=0;
	resources[0][6]=0;
	resources[1][0]=20;
	resources[1][1]=0;
	resources[1][2]=0;
	resources[1][3]=0;
	resources[1][4]=0;
	resources[1][5]=0;
	resources[1][6]=0;
	statistics[0][0]=0;
	statistics[0][1]=0;
	statistics[0][2]=0;
	statistics[0][3]=0;
	statistics[0][4]=0;
	statistics[0][5]=0;
	statistics[0][6]=0;
	statistics[0][7]=0;
	statistics[1][0]=0;
	statistics[1][1]=0;
	statistics[1][2]=0;
	statistics[1][3]=0;
	statistics[1][4]=0;
	statistics[1][5]=0;
	statistics[1][6]=0;
	statistics[1][7]=0;
	recomputestatistics=true;

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
	menu.newmenu(Menu::GENERAL_MENU);
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
  for (Robot* r: robots[0]) delete r;
  for (Robot* r: robots[1]) delete r;

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

	deleteObjects();

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
	map.clear();
	map_w=map_h=0;

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Game destroyed.\n");
	fclose(debug_fp);
#endif

} /* NETHER::~NETHER */ 


void NETHER::loadObjects()
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

  tiles[4].moveobject(Vector(0, 0, -0.05));
  tiles[5].moveobject(Vector(0, 0, -0.05));
  tiles[6].moveobject(Vector(0, 0, -0.05));
  tiles[7].moveobject(Vector(0, 0, -0.05));
  tiles[8].moveobject(Vector(0, 0, -0.05));
  tiles[9].moveobject(Vector(0, 0, -0.05));

  building_tiles.reserve(N_BUILDINGS);
  for (int i = 0; i < N_BUILDINGS; i++) {
    Shadow3DObject tile(bnames[i], "textures/");
    tile.normalize(bscale[i]);
    tile.makepositive();
    building_tiles.push_back(tile);
  }
  building_tiles[5].moveobject(Vector(0, 0, 0.01));
  building_tiles[6].moveobject(Vector(0.4, 0.4, 0.0));

  for (int i = 0; i < N_PIECES; i++) {
    Piece3DObject tile(pnames[i], "textures/");
    tile.normalize(pscale[i]);
    tile.makepositive();
    piece_tiles[0].push_back(tile);

    Piece3DObject tile2(pnames2[i], "textures/");
    tile2.normalize(pscale[i]);
    tile2.makepositive();
    piece_tiles[1].push_back(tile2);
  } /* for */ 
  piece_tiles[0][0].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[0][1].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[0][2].moveobject(Vector(-0.5, -0.5, 0.2));
  piece_tiles[0][3].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[0][4].moveobject(Vector(-0.5, -0.45, 0.0));
  piece_tiles[0][5].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[0][6].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[0][7].moveobject(Vector(-0.32, -0.3, 0.0));
  piece_tiles[0][8].moveobject(Vector(-0.45, -0.45, 0.6));
  piece_tiles[0][9].moveobject(Vector(-0.4, -0.5, 0.0));
  piece_tiles[0][10].moveobject(Vector(-0.4, 0.2, 0.0));

  piece_tiles[1][0].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[1][1].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[1][2].moveobject(Vector(-0.5, -0.5, 0.2));
  piece_tiles[1][3].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[1][4].moveobject(Vector(-0.5, -0.45, 0.0));
  piece_tiles[1][5].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[1][6].moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tiles[1][7].moveobject(Vector(-0.32, -0.3, 0.0));
  piece_tiles[1][8].moveobject(Vector(-0.45, -0.45, 0.6));
  piece_tiles[1][9].moveobject(Vector(-0.4, -0.5, 0.0));
  piece_tiles[1][10].moveobject(Vector(-0.4, 0.2, 0.0));

  ship = new Ship("models/ship.asc", "textures/");
  ship->normalize(0.5f);
  ship->makepositive();

  bullet_tiles.reserve(N_BULLETS);
  for (int i = 0; i < N_BULLETS; i++) {
    Piece3DObject tile(bullnames[i],"textures/");
    tile.normalize(bullscale[i]);
    bullet_tiles.push_back(tile);
  }

  ship->ComputeShadow(lightposv);
  for (Shadow3DObject& tile: building_tiles) tile.ComputeShadow(lightposv);
  for(int i = 0; i < N_PIECES; i++) {
    piece_tiles[0][i].ComputeFixedShadows(lightposv);
    piece_tiles[1][i].ComputeFixedShadows(lightposv);
  }
  for (Piece3DObject& tile: bullet_tiles) tile.ComputeFixedShadows(lightposv);

  construction_tiles.emplace_back("models/construction1.asc","textures/");
  construction_tiles.emplace_back("models/construction2.asc","textures/");
  construction_tiles.emplace_back("models/construction3.asc","textures/");
  construction_tiles[0].normalize(10.0);
  construction_tiles[1].normalize(9.0);
  construction_tiles[2].normalize(7.0);

  message_tiles.reserve(3);
  message_tiles.emplace_back("models/go.ase","textures/");
  message_tiles.emplace_back("models/youwin.ase","textures/");
  message_tiles.emplace_back("models/gameover.ase","textures/");
  for (auto& tile: message_tiles) tile.normalize(4.0);
}


void NETHER::deleteObjects()
{
  tiles.clear();
  delete ship;
  ship = 0;
  building_tiles.clear();
  piece_tiles[0].clear();
  piece_tiles[1].clear();
  construction_tiles.clear();
  message_tiles.clear();
  bullet_tiles.clear();
}


void NETHER::refreshDisplayLists(void)
{
  for (C3DObject& tile: tiles) {
    tile.refresh_display_lists();
  }

  ship->refresh_display_lists();

  for (Shadow3DObject& tile: building_tiles) {
      tile.refresh_display_lists();
  }

  for (int i = 0; i < N_PIECES; i++) {
    piece_tiles[0][i].refresh_display_lists();
    piece_tiles[1][i].refresh_display_lists();
  }

  for (C3DObject& tile: construction_tiles) {
    tile.refresh_display_lists();
  }
}


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
    message_tiles[0].draw(Color(1.0, 1.0, 1.0));
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
    if (statistics[0][0] == 0) message_tiles[2].draw(Color(1.0, 1.0, 1.0));
    else message_tiles[1].draw(Color(1.0, 1.0, 1.0));
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
  MINY =- 8 * zoom;
  MINX =- (10 + viewp.z * 4) * zoom;
  MAXY = (9 + viewp.z * 4) * zoom;
  MAXX = 8 * zoom;

  if (explosions.size()) {
    int minstep = 128;
    std::for_each(explosions.begin(), explosions.end(),
                  [&minstep](const auto& explosion) {
                    if (explosion.size == 2 && explosion.step < minstep) minstep = explosion.step;
                  });
    float r = (128 - minstep) / 256.0;
    float offs = sin(minstep) * r;
    gluLookAt(viewp.x + camera.x * zoom + offs,
              viewp.y + camera.y * zoom + offs,
              viewp.z + camera.z * zoom,
              viewp.x + offs,
              viewp.y + offs,
              viewp.z,
              0, 0, 1);
  } else {
    gluLookAt(viewp.x + camera.x * zoom,
              viewp.y + camera.y * zoom,
              viewp.z + camera.z * zoom,
              viewp.x,
              viewp.y,
              viewp.z,
              0, 0, 1);
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  /* Draw the map: */
  drawmap(shadows);

  /* Draw the robots and bullets: */
  {
    for(int i = 0; i < 2; i++) {
      for (Robot* r: robots[i]) {
        if (r->pos.y >= (viewp.y + MINY) &&
            r->pos.y <= (viewp.y + MAXY) &&
            r->pos.x >= (viewp.x + MINX) &&
            r->pos.x <= (viewp.x + MAXX)) {
          glPushMatrix();
          glTranslatef(r->pos.x, r->pos.y, r->pos.z);
          r->draw(i, shadows, piece_tiles, lightposv);
          glPopMatrix();
        }
      }
    }

    std::for_each(bullets.cbegin(), bullets.cend(),
                  [this, shadows](auto& bullet) {
                    if (bullet.pos.y >= (viewp.y + MINY) &&
                        bullet.pos.y <= (viewp.y + MAXY) &&
                        bullet.pos.x >= (viewp.x + MINX) &&
                        bullet.pos.x <= (viewp.x + MAXX)) {
                      glPushMatrix();
                      glTranslatef(bullet.pos.x, bullet.pos.y, bullet.pos.z);
                      bullet.draw(shadows, bullet_tiles, particles);
                      glPopMatrix();
                    }
                  });
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
      minz = MapMaxZ(x, y);
    } else {
      minz = controlled->pos.z;
    }

    glPushMatrix();
    glTranslatef(sx, sy, minz+0.05);
    ship->DrawShadow(Color(0, 0, 0, 0.5));
    glPopMatrix();
  }

  /* Draw the extras: */

  /* Draw nuclear explosions: */
  if (!shadows) {
    for (const Explosion& exp: explosions) {
      float a = (128.0f - exp.step) / 80.0f;
      float r = 1.0;
      if (exp.size == 0) {
        r = (float(exp.step) / 512.0f) + 0.1;
      }
      if (exp.size == 1) {
        r = (float(exp.step) / 96.0f) + 0.5;
      }
      if (exp.size == 2) {
        r = (float(exp.step) / 48.0f) + 1.0;
      }
      if (a < 0) a = 0;
      if (a > 1) a = 1;

      glPushMatrix();
      glTranslatef(exp.pos.x, exp.pos.y, exp.pos.z);
      glColor4f(1.0f, 0.5f, 0.0,a);
      glDepthMask(GL_FALSE);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnable(GL_BLEND);
      // Somehow solid sphere dumps core
      // glutSolidSphere(r, 8, 8);
      glutWireSphere(r, 8, 8);
      glDisable(GL_BLEND);
      glDepthMask(GL_TRUE);
      glPopMatrix();
    }
  }

  if (!shadows) {
    for (const Particle& particle: particles) {
      if (particle.pos.y >= (viewp.y + MINY) &&
          particle.pos.y <= (viewp.y + MAXY) &&
          particle.pos.x >= (viewp.x + MINX) &&
          particle.pos.x <= (viewp.x + MAXX))
        particle.draw();
    }
  }
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
					recomputestatistics=true;
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


bool NETHER::ShipCollision(C3DObject *obj,float x,float y,float z)
{
	float m1[16]={1,0,0,0,
				  0,1,0,0,
				  0,0,1,0,
				  x,y,z,1};
	float m2[16]={1,0,0,0,
				  0,1,0,0,
				  0,0,1,0,
				  0,0,0,1};

	/* Collision with buildings: */
	for (const Building& b: buildings) {
		if (((b.pos.x-x)*(b.pos.x-x)+
			 (b.pos.y-y)*(b.pos.y-y)+
			 (b.pos.z-z)*(b.pos.z-z))<COLISION_TEST_THRESHOLD) {

			m2[12]=b.pos.x;
			m2[13]=b.pos.y;
			m2[14]=b.pos.z;

			switch(b.type) {
				case Building::TYPE::FENCE:
					if (obj->cmc.collision_simple(m1,&(building_tiles[5].cmc),m2)) return true;
					break;
				case Building::TYPE::WALL1:
					if (obj->cmc.collision_simple(m1,&(building_tiles[0].cmc),m2)) return true;
					break;
				case Building::TYPE::WALL2:
					if (obj->cmc.collision_simple(m1,&(building_tiles[1].cmc),m2)) return true;
					break;
				case Building::TYPE::WALL3:
					if (obj->cmc.collision_simple(m1,&(building_tiles[2].cmc),m2)) return true;
					break;
				case Building::TYPE::WALL4:
					if (obj->cmc.collision_simple(m1,&(building_tiles[3].cmc),m2)) return true;
					break;
				case Building::TYPE::WALL5:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					break;
				case Building::TYPE::WALL6:
					if (obj->cmc.collision_simple(m1,&(building_tiles[7].cmc),m2)) return true;
					break;
				case Building::TYPE::WARBASE:
					if (obj->cmc.collision_simple(m1,&(building_tiles[8].cmc),m2)) return true;
					break;
			case Building::TYPE::FACTORY_ELECTRONICS:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tiles[0][7].cmc),m2)) return true;
					break;
			case Building::TYPE::FACTORY_NUCLEAR:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tiles[0][6].cmc),m2)) return true;
					break;
			case Building::TYPE::FACTORY_PHASERS:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tiles[0][5].cmc),m2)) return true;
					break;
			case Building::TYPE::FACTORY_MISSILES:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tiles[0][4].cmc),m2)) return true;
					break;
			case Building::TYPE::FACTORY_CANNONS:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tiles[0][3].cmc),m2)) return true;
					break;
			case Building::TYPE::FACTORY_CHASSIS:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tiles[0][1].cmc),m2)) return true;
					break;
			} /* switch */ 
		} /* if */ 
	} /* while */ 

	/* Collision with the robots: */ 
	for(int i = 0; i < 2; i++) {
      for (Robot* r: robots[i]) {
        if (((r->pos.x-x)*(r->pos.x-x)+
             (r->pos.y-y)*(r->pos.y-y))<COLISION_TEST_THRESHOLD) {
          m2[12]=r->pos.x;
          m2[13]=r->pos.y;
          m2[14]=r->pos.z; 
          if (obj->cmc.collision_simple(m1,&(r->cmc),m2)) return true;
        } /* if */ 
      } /* while */ 
	} /* while */ 

	return false;
} /* NETHER::ShipCollision */ 


bool NETHER::saveGame(const std::string& filename)
{
  std::ofstream oFile(filename);

  oFile << map_w << ' ' << map_h << '\n';

  for (int i = 0; i < map_h; i++) {
    for (int j = 0; j < map_w; j++) {
      oFile << map[j + i * map_w] << ' ';
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

  oFile << buildings.size() << '\n';
  for (Building& b: buildings) {
    oFile << b;
  }

  for (int i = 0; i < 2; i++) {
    oFile << robots[i].size() << '\n';
    for (Robot* r: robots[i]) {
      oFile << *r;
    }
  }

  oFile << bullets.size() << '\n';
  for (Bullet& bullet: bullets) {
    oFile << std::make_pair(bullet, robots);
  }

  oFile << explosions.size() << '\n';
  for (Explosion& e: explosions) {
    oFile << e;
  }

  oFile << day << ' ' << hour << ' ' << minute << ' ' << second << '\n';
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      oFile << resources[i][j] << ' ';
    }
    oFile << '\n';
   }

  oFile << find_index(robots[0], controlled) << '\n';
  oFile << menu.act_menu << ' ' << menu.act_button << std::endl;

  return true;
}


bool NETHER::loadGame(const std::string& filename)
{
  std::ifstream inFile(filename);

  AI_deleteprecomputations();

  inFile >> map_w >> map_h;

  explosions.clear();
  buildings.clear();
  for (int i = 0; i < 2; i++) {
    for (Robot* r: robots[i]) delete r;
    robots[i].clear();
  }
  bullets.clear();
  map.clear();
  map.reserve(map_w * map_h);
  for (int i = 0; i < map_h; i++) {
    for (int j = 0; j < map_w; j++) {
      int tile;
      inFile >> tile;
      map.push_back(tile);
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
    buildings.push_back(Building(inFile));
  }

  for (int i = 0; i < 2; i++) {
    inFile >> length;
    for (int k = 0; k < length; k++) {
      robots[i].push_back(new Robot(inFile));
    }
  }

  inFile >> length;
  for (int k = 0; k < length; k++) {
    bullets.emplace_back(inFile, robots);
  }

  inFile >> length;
  for (int k = 0; k < length; k++) {
    explosions.emplace_back(inFile);
  }

  inFile >> day >> hour >> minute >> second;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      inFile >> resources[i][j];
    }
  }

  int i;
  inFile >> i;
  if (i >= 0)
    controlled = robots[0][i];
  else
    controlled = 0;

  int actMenu_, actButton_;
  inFile >> actMenu_ >> actButton_;
  menu.act_menu = Menu::MENU_TYPES(actMenu_);
  menu.act_button = StatusButton::BUTTON_NAMES(actButton_);

  AI_precomputations();
  return true;
}


bool NETHER::saveDebugReport(const std::string& filename)
{
  std::ofstream log(filename);
  log << "NETHER EARTH NG Debug Report\n\n";
  log << "MAPW: " << map_w << "\nMAPH: " << map_h << '\n';
  log << "MAP:\n";
  for(int i = 0; i < map_h; i++) {
    for(int j = 0; j < map_w; j++) {
      log << map[j+i*map_w] << ' ';
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

  log << "# OF BUILDINGS: " << buildings.size() << '\n';
  for (const Building& b: buildings) {
    log << "BUILDING:\n"
        << " TYPE: " << int(b.type)
        << "\n OWNER: " << b.owner
        << "\n STATUS: " << b.status << "\n"
        << b.pos;
  }

  for (int i = 0; i < 2; i++) {
    log << "\n# OF ROBOTS PLAYER " << i << ": " << robots[i].size() << '\n';

    const char* tractions[3] = {"BIPOD", "TRACKS", "ANTIGRAV"};
    const char* pieces[5] = {"CANNONS", "MISSILES", "PHASERS", "NUCLEAR", "ELECTRONICS"};
    for (Robot* r: robots[i]) {
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

  log << "\n# BULLETS: " << bullets.size() << '\n';
  for (Bullet& bullet: bullets) {
    log << " BULLET:\n TYPE: " << int(bullet.type)
        << "\n STEP: " << bullet.step
        << "\n ANGLE: " << bullet.angle << '\n';
    log << " POSITION: ";
    log << bullet.pos;

    if (std::count(robots[0].cbegin(), robots[0].cend(), bullet.owner)) {
      log << " OWNER: PLAYER 0 ROBOT " << bullet.owner->getId() << '\n';
    } else if (std::count(robots[1].cbegin(), robots[1].cend(), bullet.owner)) {
      log << " OWNER: PLAYER 1 ROBOT " << bullet.owner->getId() << '\n';
    }

    log << " MINIMUM CONTAINER BOX: \n";
    log << bullet.cmc << '\n';
  }

  log << "# EXPLOSIONS " << explosions.size() << '\n';
  for (Explosion& e: explosions) {
    log << "EXPLOSION:\n POSITION:\n";
    log << e.pos;
    log << " STEP: " << e.step << "\n SIZE: " << e.size << "\n\n";
  }

  log << "\nTIME: DAY " << day << ' ' << hour << ':' << minute << ':' << second << '\n';
  log << "\nRESOURCES:\n";

  for(int i = 0; i < 2; i++) {
    log << "  PLAYER " << i << ": ";
    for(int j = 0; j < 7; j++) {
      log << resources[i][j] << ' ';
    }
    log << '\n';
  }

  log << "\nROBOT UNDER CONTROL: ";
  if (controlled)
    log << controlled->getId() << '\n';
  else
    log << "None\n";
  log << "\nMENU " << menu.act_menu << "\nACT BUTTON: " << menu.act_button << '\n';

  return true;
}
