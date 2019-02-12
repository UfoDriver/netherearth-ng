#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>
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

#include "list.h"
#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "myglutaux.h"
#include "nether.h"
#include "menu.h"

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

#ifdef _WRITE_REPORT_
FILE *debug_fp=0;
#endif


NETHER::NETHER(const std::string& mapname): menu(this), radar(this), n_objs(12),
                                            n_pieces(11), n_bullets(3)
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
	if (!loadmap(mapname)) {
		map_w=map_h=0;
		map=0;
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
	shipp.x=4.0;
	shipp.y=2.0;
	shipp.z=3.0;
	ship_op=OP_NONE;
	ship_op2=OP_NONE;
	ship_timemoving=0;

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

	game_state=STATE_PLAYING;
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
	if (map!=0) delete map;
	map=0;
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

  tile = new C3DObject *[n_objs];
  for (int i = 0; i < n_objs; i++) {
    tile[i] = new C3DObject(tnames[i], "textures/", colors[i]);
    tile[i]->normalize(0.50f);
    tile[i]->makepositive();
  }

  tile[4]->moveobject(Vector(0, 0, -0.05));
  tile[5]->moveobject(Vector(0, 0, -0.05));
  tile[6]->moveobject(Vector(0, 0, -0.05));
  tile[7]->moveobject(Vector(0, 0, -0.05));
  tile[8]->moveobject(Vector(0, 0, -0.05));
  tile[9]->moveobject(Vector(0, 0, -0.05));

  building_tiles.reserve(N_BUILDINGS);
  for (int i = 0; i < N_BUILDINGS; i++) {
    Shadow3DObject tile(bnames[i], "textures/");
    tile.normalize(bscale[i]);
    tile.makepositive();
    building_tiles.push_back(tile);
  }
  building_tiles[5].moveobject(Vector(0, 0, 0.01));
  building_tiles[6].moveobject(Vector(0.4, 0.4, 0.0));

  piece_tile[0]=new Piece3DObject *[n_pieces];
  piece_tile[1]=new Piece3DObject *[n_pieces];
  for(int i=0; i <n_pieces;i++) {
    piece_tile[0][i]=new Piece3DObject(pnames[i],"textures/");
    piece_tile[0][i]->normalize(pscale[i]);
    piece_tile[0][i]->makepositive();
    piece_tile[1][i]=new Piece3DObject(pnames2[i],"textures/");
    piece_tile[1][i]->normalize(pscale[i]);
    piece_tile[1][i]->makepositive();
  } /* for */ 
  piece_tile[0][0]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[0][1]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[0][2]->moveobject(Vector(-0.5, -0.5, 0.2));
  piece_tile[0][3]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[0][4]->moveobject(Vector(-0.5, -0.45, 0.0));
  piece_tile[0][5]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[0][6]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[0][7]->moveobject(Vector(-0.32, -0.3, 0.0));
  piece_tile[0][8]->moveobject(Vector(-0.45, -0.45, 0.6));
  piece_tile[0][9]->moveobject(Vector(-0.4, -0.5, 0.0));
  piece_tile[0][10]->moveobject(Vector(-0.4, 0.2, 0.0));

  piece_tile[1][0]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[1][1]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[1][2]->moveobject(Vector(-0.5, -0.5, 0.2));
  piece_tile[1][3]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[1][4]->moveobject(Vector(-0.5, -0.45, 0.0));
  piece_tile[1][5]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[1][6]->moveobject(Vector(-0.5, -0.5, 0.0));
  piece_tile[1][7]->moveobject(Vector(-0.32, -0.3, 0.0));
  piece_tile[1][8]->moveobject(Vector(-0.45, -0.45, 0.6));
  piece_tile[1][9]->moveobject(Vector(-0.4, -0.5, 0.0));
  piece_tile[1][10]->moveobject(Vector(-0.4, 0.2, 0.0));

  ship = new Shadow3DObject("models/ship.asc", "textures/");
  ship->normalize(0.5f);
  ship->makepositive();

  bullet_tile=new Piece3DObject *[n_bullets];
  for(int i=0;i<n_bullets;i++) {
    bullet_tile[i]=new Piece3DObject(bullnames[i],"textures/");
    bullet_tile[i]->normalize(bullscale[i]);
  } /* for */

  ship->ComputeShadow(lightposv);
  for (Shadow3DObject& tile: building_tiles) tile.ComputeShadow(lightposv);
  for(int i=0;i<n_pieces;i++) piece_tile[0][i]->ComputeFixedShadows(lightposv);
  for(int i=0;i<n_pieces;i++) piece_tile[1][i]->ComputeFixedShadows(lightposv);
  for(int i=0;i<n_bullets;i++) bullet_tile[i]->ComputeFixedShadows(lightposv);

  construction_tile[0]=new C3DObject("models/construction1.asc","textures/");
  construction_tile[1]=new C3DObject("models/construction2.asc","textures/");
  construction_tile[2]=new C3DObject("models/construction3.asc","textures/");
  construction_tile[0]->normalize(10.0);
  construction_tile[1]->normalize(9.0);
  construction_tile[2]->normalize(7.0);

  message_tile[0]=new C3DObject("models/go.ase","textures/");
  message_tile[1]=new C3DObject("models/youwin.ase","textures/");
  message_tile[2]=new C3DObject("models/gameover.ase","textures/");
  message_tile[0]->normalize(4.0);
  message_tile[1]->normalize(4.0);
  message_tile[2]->normalize(4.0);
} /* NETHER::loadobjects */ 


void NETHER::deleteObjects()
{
	for(int i=0; i < n_objs; i++) delete tile[i];
	delete tile;
	tile = 0;
	delete ship;
	ship = 0;
    building_tiles.clear();
	for(int i = 0;i < n_pieces; i++) delete piece_tile[0][i];
	for(int i = 0; i < n_pieces;i++) delete piece_tile[1][i];
	delete piece_tile[0];
	delete piece_tile[1];
	piece_tile[0]=0;
	piece_tile[1]=0;
	delete construction_tile[0];
	delete construction_tile[1];
	delete construction_tile[2];
	construction_tile[0]=0;
	construction_tile[1]=0;
	construction_tile[2]=0;
	delete message_tile[0];
	delete message_tile[1];
	delete message_tile[2];
	for(int i = 0; i < n_bullets; i++) delete bullet_tile[i];
	delete bullet_tile;
	bullet_tile = 0;
}


void NETHER::refresh_display_lists(void)
{
	int i;
 
 	for(i=0;i<n_objs;i++) {
 		tile[i]->refresh_display_lists();
 	} /* for */ 
 
 	ship->refresh_display_lists();
 
 	for (Shadow3DObject& tile: building_tiles) {
 		tile.refresh_display_lists();
 	}
 
 	for(i=0;i<n_pieces;i++) {
 		piece_tile[0][i]->refresh_display_lists();
 		piece_tile[1][i]->refresh_display_lists();
 	} /* for */ 
 
 	for(i=0;i<3;i++) {
 		construction_tile[i]->refresh_display_lists();
 	} /* for */ 
} /* NETHER::refresh_display_lists */ 



bool NETHER::gamecycle(int w, int h)
{
  bool retval = true;
  unsigned char *keyboard;

  SDL_PumpEvents();
  keyboard = SDL_GetKeyState(NULL);

#ifdef _WRITE_REPORT_
  fprintf(debug_fp,"Cycle start.\n");
  fprintf(debug_fp,"game_state: %i\n",game_state);
  fflush(debug_fp);
#endif

  switch(game_state) {
  case STATE_PLAYING:
    retval = cycle(keyboard);
    break;
  case STATE_CONSTRUCTION:
    retval = construction_cycle(keyboard);
    break;
  case STATE_PAUSE:
  case STATE_SAVINGGAME:
  case STATE_LOADINGGAME:
    retval = option_cycle(keyboard);
    break;
  }

  for (int i = 0; i < SDLK_LAST; i++)
    old_keyboard[i] = keyboard[i];

#ifdef _WRITE_REPORT_
  fprintf(debug_fp,"Cycle end: %i\n",retval);
  fflush(debug_fp);
#endif

  return retval;
}


void NETHER::gameredraw(int w,int h)
{

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Redraw start.\n");
	fprintf(debug_fp,"game_state: %i\n",game_state);
	fflush(debug_fp);
#endif

	switch(game_state) {
	case STATE_PLAYING:
		draw(w,h);
		break;
	case STATE_CONSTRUCTION:
		construction_draw(w,h);
		break;
	case STATE_PAUSE:
	case STATE_SAVINGGAME:
	case STATE_LOADINGGAME:
		draw(w,h);
		options_draw(w,h);
		break;
	} /* switch */ 

	SDL_GL_SwapBuffers();

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Redraw end.");
	fflush(debug_fp);
#endif

} /* gameredraw */ 


void NETHER::draw(int width, int height)
{
	float lightpos2[4]={0,0,1000,0};
	float tmpls[4]={1.0F,1.0F,1.0F,1.0};
	float tmpld[4]={0.6F,0.6F,0.6F,1.0};
	float tmpla[4]={0.2F,0.2F,0.2F,1.0};
    float ratio;
	int split = int((width*25.0F)/32.0F);
	int splity = 0;

	if (show_radar) splity = int((height*2.0F)/15.0F)+1;
			   else splity = 0;

	/* Enable Lights, etc.: */ 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_AMBIENT,tmpla);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,tmpld);
	glLightfv(GL_LIGHT0,GL_SPECULAR,tmpls);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel( GL_SMOOTH );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
	glEnable( GL_SCISSOR_TEST );  
	glEnable( GL_DEPTH_TEST );
	glDepthFunc( GL_LEQUAL );
	glClearStencil(0);
	
	/* Draw the GAME screen: */ 
	glLightfv(GL_LIGHT0,GL_POSITION,lightpos);
    glClearColor(0,0,0,0.0);
    glViewport(0,splity,split,height-splity);
	ratio=float(split)/float(height-splity);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    gluPerspective( 30.0, ratio, 1.0, 1024.0 );
	glScissor(0,splity,split,height-splity);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	draw_game(false);
	if (shadows) {
		/* Set STENCIL Buffer: */ 
		glStencilMask(1);
		glEnable(GL_STENCIL_TEST);
		glDepthMask(GL_FALSE);
		glColorMask(GL_FALSE,GL_FALSE,GL_FALSE,GL_FALSE);
		glStencilFunc(GL_ALWAYS,1,1);
		glStencilOp(GL_KEEP,GL_KEEP,GL_REPLACE);
		draw_game(true);
		glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);

		/* Draw shadow poligon: */ 
		glDepthFunc(GL_ALWAYS);
		glDisable(GL_CULL_FACE);

		glColor4f(0.0,0.0,0.0,0.4f);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);

		glStencilFunc(GL_NOTEQUAL,0,1);
		glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glBegin(GL_TRIANGLE_STRIP);
		glVertex3f(-1.0, 1.0,0.0);
		glVertex3f(-1.0,-1.0,0.0);
		glVertex3f( 1.0, 1.0,0.0);
		glVertex3f( 1.0,-1.0,0.0);
		glEnd();
		glPopMatrix();

		glDisable(GL_BLEND);

		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LEQUAL);
		glEnable(GL_CULL_FACE);
		glDisable(GL_STENCIL_TEST);
	} /* if */ 

	if (game_started>0) {
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity( );
		gluPerspective( 30.0, ratio, 1.0, 1024.0 );
		gluLookAt(0,0,30,0,0,0,0,1,0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if (game_started>40) glTranslatef(0,0,(game_started-40)*2);
		if (game_started<20) glTranslatef(0,0,(20-game_started)*2);
		message_tile[0]->draw(Color(1.0, 1.0, 1.0));
	} /* if */ 

	if (game_finished>100) {
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity( );
		gluPerspective( 30.0, ratio, 1.0, 1024.0 );
		gluLookAt(0,0,30,0,0,0,0,1,0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		if (game_finished<120) glTranslatef(0,0,(120-game_finished)*2);
		if (game_finished>240) glTranslatef(0,0,(game_finished-240)*2);
		if (statistics[0][0]==0) message_tile[2]->draw(Color(1.0, 1.0, 1.0));
        else message_tile[1]->draw(Color(1.0, 1.0, 1.0));
	} /* if */ 

	/* Draw the RADAR screen: */ 
	if (show_radar && radar.needsRedraw<=1) {

		glLightfv(GL_LIGHT0,GL_POSITION,lightpos2);
		glClearColor(0.0,0.0,0,0);
		glViewport(0,0,split,splity);
		glMatrixMode( GL_PROJECTION );
		glLoadIdentity( );
		glOrtho(0,float(split),0,float(splity),-100,100);
		glScissor(0,0,split,splity);
		glScalef(width/640.0,height/480.0,1);
		radar.draw();
	} /* if */
	radar.needsRedraw--;
	if (radar.needsRedraw<0) radar.needsRedraw=3;

    menu.draw(width, height);
}



void NETHER::draw_game(bool shadows)
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
		List<Robot> l;
		List<Bullet> l2;
		Robot *r;
		Bullet *b;

		for(int i = 0; i < 2; i++) {
          for (Robot& r: robots[i]) {
				if (r.pos.y>=(viewp.y+MINY) &&
					r.pos.y<=(viewp.y+MAXY) &&
					r.pos.x>=(viewp.x+MINX) &&
					r.pos.x<=(viewp.x+MAXX)) {
					glPushMatrix();
					glTranslatef(r.pos.x,r.pos.y,r.pos.z);
					r.draw(i, shadows, piece_tile, lightposv);
					glPopMatrix();
				} /* if */ 
			} /* while */ 
		} /* for */ 

        std::for_each(bullets.cbegin(), bullets.cend(),
                      [this, shadows](auto& bullet) {
                        if (bullet.pos.y >= (viewp.y + MINY) &&
                            bullet.pos.y <= (viewp.y + MAXY) &&
                            bullet.pos.x >= (viewp.x + MINX) &&
                            bullet.pos.x <= (viewp.x + MAXX)) {
                          glPushMatrix();
                          glTranslatef(bullet.pos.x, bullet.pos.y, bullet.pos.z);
                          bullet.draw(shadows, bullet_tile, particles);
                          glPopMatrix();
                        }
                      });
	}

	/* Draw the ship: */ 
	glPushMatrix();
	glTranslatef(shipp.x,shipp.y,shipp.z);
	if (!shadows) ship->draw(Color(0.7, 0.7, 0.7));
	glPopMatrix();

	if (shadows) {
		float sx,sy;
		float x[2],y[2];
		float minz;
		Vector light;

		light=lightposv;
		light=light/light.z;

		sx=shipp.x-light.x*shipp.z;
		sy=shipp.y-light.y*shipp.z;

		if (controlled==0) {
			x[0]=sx+ship->shdw_cmc.x[0];
			x[1]=sx+ship->shdw_cmc.x[1];
			y[0]=sy+ship->shdw_cmc.y[0];
			y[1]=sy+ship->shdw_cmc.y[1];
			minz=MapMaxZ(x,y);
		} else {
			minz=controlled->pos.z;
		} /* if */ 

		glPushMatrix();
		glTranslatef(sx,sy,minz+0.05);
		ship->DrawShadow(Color(0, 0, 0, 0.5));
		glPopMatrix();
	} 

	/* Draw the extras: */ 

    /* Draw nuclear explosions: */ 
    if (!shadows) {
      std::for_each(explosions.begin(), explosions.end(),
                    [](const auto& exp) {
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
                    });
    }

  if (!shadows) {
    std::for_each(particles.cbegin(), particles.cend(),
                  [this](auto& particle) {
                    if (particle.pos.y >= (viewp.y + MINY) &&
                        particle.pos.y <= (viewp.y + MAXY) &&
                        particle.pos.x >= (viewp.x + MINX) &&
                        particle.pos.x <= (viewp.x + MAXX))
                      particle.draw();
                  });
  }
}


void NETHER::options_draw(int w,int h)
{
	float lightpos2[4]={0,0,1000,0};
	float tmpls[4]={1.0F,1.0F,1.0F,1.0};
	float tmpld[4]={0.6F,0.6F,0.6F,1.0};
	float tmpla[4]={0.2F,0.2F,0.2F,1.0};
    float ratio;
	int splitx[2]={int(w*0.3),int(w*0.7)};
	int splity[2]={int(h*0.3),int(h*0.7)};

	/* Enable Lights, etc.: */ 
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0,GL_AMBIENT,tmpla);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,tmpld);
	glLightfv(GL_LIGHT0,GL_SPECULAR,tmpls);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel( GL_SMOOTH );
	glCullFace( GL_BACK );
	glFrontFace( GL_CCW );
    glEnable( GL_CULL_FACE );
	glEnable( GL_SCISSOR_TEST );  
	glEnable( GL_DEPTH_TEST );
	
	/* Draw the MENU: */ 
	glLightfv(GL_LIGHT0,GL_POSITION,lightpos);
    glClearColor(0,0,0,0.0);
    glViewport(splitx[0],splity[0],splitx[1]-splitx[0],splity[1]-splity[0]);
	ratio=float(splitx[1]-splitx[0])/float(splity[1]-splity[0]);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    gluPerspective( 30.0, ratio, 1.0, 1024.0 );
	glScissor(splitx[0],splity[0],splitx[1]-splitx[0],splity[1]-splity[0]);
	gluLookAt(0,0,30,0,0,0,0,1,0);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

	if (game_state==STATE_PAUSE) {
		if (option_menu==0) glColor3f(1.0,0.0,0.0);
					   else glColor3f(0.5,0.5,1.0);
		glTranslatef(0,5,0);
		scaledglprintf(0.01,0.01,"RETURN TO GAME");

		if (option_menu==1) glColor3f(1.0,0.0,0.0);
					   else glColor3f(0.5,0.5,1.0);
		glTranslatef(0,-5,0);
		scaledglprintf(0.01,0.01,"LOAD GAME");

		if (option_menu==2) glColor3f(1.0,0.0,0.0);
					   else glColor3f(0.5,0.5,1.0);
		glTranslatef(0,-3,0);
		scaledglprintf(0.01,0.01,"SAVE GAME");

		if (option_menu==3) glColor3f(1.0,0.0,0.0);
					   else glColor3f(0.5,0.5,1.0);
		glTranslatef(0,-3,0);
		scaledglprintf(0.01,0.01,"QUIT GAME");
	} /* if */ 

	if (game_state==STATE_SAVINGGAME) {
		int i;
		FILE *fp;
		char filename[80];

		glColor3f(0.5,0.5,1.0);
		glTranslatef(0,6,0);
		scaledglprintf(0.01,0.01,"CHOOSE SLOT TO SAVE");

		if (option_menu==0) glColor3f(1.0,0.0,0.0);
					   else glColor3f(0.5,0.5,1.0);
		glTranslatef(0,-4,0);
		scaledglprintf(0.01,0.01,"CANCEL");

		for(i=0;i<4;i++) {
			if (option_menu==(i+1)) glColor3f(1.0,0.0,0.0);
						       else glColor3f(0.5,0.5,1.0);
			glTranslatef(0,-2,0);
			sprintf(filename,"savedgame%i.txt",i);
			fp=fopen(filename,"r");
			if (fp==0) {
				scaledglprintf(0.01,0.01,"SLOT%i - EMPTY",i+1);
			} else {
				scaledglprintf(0.01,0.01,"SLOT%i - GAME SAVED",i+1);
				fclose(fp);
			} /* if */ 
		} /* for */ 
	} /* if */ 

	if (game_state==STATE_LOADINGGAME) {
		int i;
		FILE *fp;
		char filename[80];

		glColor3f(0.5,0.5,1.0);
		glTranslatef(0,6,0);
		scaledglprintf(0.01,0.01,"CHOOSE SLOT TO LOAD");

		if (option_menu==0) glColor3f(1.0,0.0,0.0);
					   else glColor3f(0.5,0.5,1.0);
		glTranslatef(0,-4,0);
		scaledglprintf(0.01,0.01,"CANCEL");

		for(i=0;i<4;i++) {
			if (option_menu==(i+1)) glColor3f(1.0,0.0,0.0);
						       else glColor3f(0.5,0.5,1.0);
			glTranslatef(0,-2,0);
			sprintf(filename,"savedgame%i.txt",i);
			fp=fopen(filename,"r");
			if (fp==0) {
				scaledglprintf(0.01,0.01,"SLOT%i - EMPTY",i+1);
			} else {
				scaledglprintf(0.01,0.01,"SLOT%i - GAME SAVED",i+1);
				fclose(fp);
			} /* if */ 
		} /* for */ 
	} /* if */ 


} /* NETHER::options_draw */ 


bool NETHER::option_cycle(unsigned char *keyboard)
{
	switch(game_state) {
	case STATE_PAUSE:
		if (keyboard[fire_key] && !old_keyboard[fire_key]) {
			switch(option_menu) {
			case 0:
					game_state=STATE_PLAYING;
					break;
			case 1: game_state=STATE_LOADINGGAME;
					option_menu=0;
					break;
			case 2:	game_state=STATE_SAVINGGAME;
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

	case STATE_SAVINGGAME:
		if (keyboard[fire_key] && !old_keyboard[fire_key]) {
			switch(option_menu) {
			case 0:
					game_state=STATE_PAUSE;
					option_menu=2;
					break;
			case 1: 
			case 2:
			case 3:
			case 4:
				{
					char filename[80];
					sprintf(filename,"savedgame%i.txt",option_menu-1);
					save_game(filename);
					saveDebugReport("debugreport.txt");
					game_state=STATE_PAUSE;
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

	case STATE_LOADINGGAME:
		if (keyboard[fire_key] && !old_keyboard[fire_key]) {
			switch(option_menu) {
			case 0:
					game_state=STATE_PAUSE;
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
					game_state=STATE_PAUSE;
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
	int i;
	List<Robot> l2;
	Robot *r;
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
				case Building::B_FENCE:
					if (obj->cmc.collision_simple(m1,&(building_tiles[5].cmc),m2)) return true;
					break;
				case Building::B_WALL1:
					if (obj->cmc.collision_simple(m1,&(building_tiles[0].cmc),m2)) return true;
					break;
				case Building::B_WALL2:
					if (obj->cmc.collision_simple(m1,&(building_tiles[1].cmc),m2)) return true;
					break;
				case Building::B_WALL3:
					if (obj->cmc.collision_simple(m1,&(building_tiles[2].cmc),m2)) return true;
					break;
				case Building::B_WALL4:
					if (obj->cmc.collision_simple(m1,&(building_tiles[3].cmc),m2)) return true;
					break;
				case Building::B_WALL5:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					break;
				case Building::B_WALL6:
					if (obj->cmc.collision_simple(m1,&(building_tiles[7].cmc),m2)) return true;
					break;
				case Building::B_WARBASE:
					if (obj->cmc.collision_simple(m1,&(building_tiles[8].cmc),m2)) return true;
					break;
			case Building::B_FACTORY_ELECTRONICS:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tile[0][7]->cmc),m2)) return true;
					break;
			case Building::B_FACTORY_NUCLEAR:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tile[0][6]->cmc),m2)) return true;
					break;
			case Building::B_FACTORY_PHASERS:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tile[0][5]->cmc),m2)) return true;
					break;
			case Building::B_FACTORY_MISSILES:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tile[0][4]->cmc),m2)) return true;
					break;
			case Building::B_FACTORY_CANNONS:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tile[0][3]->cmc),m2)) return true;
					break;
			case Building::B_FACTORY_CHASSIS:
					if (obj->cmc.collision_simple(m1,&(building_tiles[4].cmc),m2)) return true;
					m2[12]=b.pos.x+0.5;
					m2[13]=b.pos.y+0.5;
					m2[14]=b.pos.z+1;
					if (obj->cmc.collision_simple(m1,&(piece_tile[0][1]->cmc),m2)) return true;
					break;
			} /* switch */ 
		} /* if */ 
	} /* while */ 

	/* Collision with the robots: */ 
	for(i=0;i<2;i++) {
      for (Robot& r: robots[i]) {
			if (((r.pos.x-x)*(r.pos.x-x)+
				 (r.pos.y-y)*(r.pos.y-y))<COLISION_TEST_THRESHOLD) {
				m2[12]=r.pos.x;
				m2[13]=r.pos.y;
				m2[14]=r.pos.z; 
				if (obj->cmc.collision_simple(m1,&(r.cmc),m2)) return true;
			} /* if */ 
		} /* while */ 
	} /* while */ 

	return false;
} /* NETHER::ShipCollision */ 
