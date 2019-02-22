#ifdef _WIN32
#include "windows.h"
#endif

#include "string.h"
#include "stdlib.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "stdio.h"
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
#include "nether.h"

#include "glprintf.h"

extern int detaillevel;
extern int frames_per_sec;
extern bool fullscreen;
extern int shadows;
extern bool sound;
extern int up_key,down_key,left_key,right_key,fire_key,pause_key;
extern int level;
extern float MINY,MAXY,MINX,MAXX;

const float NUCLEAR_RADIUS = 2.5f;

extern FILE *debug_fp;


void find_and_destroy_robot(std::vector<Robot*> robots[], Robot* robot)
{
  for (int i = 0; i < 2; i++) {
    robots[i].erase(std::remove_if(robots[i].begin(), robots[i].end(),
                                   [robot](const Robot* r) {
                                     return r == robot;
                                   }),
                    robots[i].end());
  }
  delete robot;
}


bool NETHER::cycle(unsigned char *keyboard)
{

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"PLAY CYCLE STARTS\n");
	fflush(debug_fp);
#endif

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Recomputing statistics\n");
	fflush(debug_fp);
#endif

	if (keyboard[SDLK_PAGEUP]) {
		camera.zoom*=1.1;
		if (camera.zoom>4) camera.zoom=4;
	} /* if */ 
	if (keyboard[SDLK_PAGEDOWN]) {
		camera.zoom/=1.1;
		if (camera.zoom<0.5) camera.zoom=0.5;
	} /* if */ 

    stats.recompute(map.buildings);

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Starting enemy AI\n");
	fflush(debug_fp);
#endif

	/* ENEMY Artificial Intelligence: */ 
	if (stats.second==0) {
		if (level==0 && (stats.hour&0x01)==0 && stats.minute==0) ai.enemy();
		if (level==1 && stats.minute==0) ai.enemy();
		if (level>=2 && (stats.minute==0 || stats.minute==30)) ai.enemy();
	} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Starting GAME CYCLE\n");
	fflush(debug_fp);
#endif

	/* GAME Cycle: */ 
	{
		ship->landed=false;
		
#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Menu\n");
	fflush(debug_fp);
#endif
		switch(menu.act_menu) {
		case Menu::TYPE::GENERAL:
			/* Free movement of the ship through the map: */ 
			{
				float x[2],y[2];
				float minz;
				Vector old_shipp;

				ship->pos.x=(int(ship->pos.x*8.0))/8.0;
				ship->pos.y=(int(ship->pos.y*8.0))/8.0;
				old_shipp=ship->pos;
				x[0]=ship->pos.x;
				x[1]=ship->pos.x+1.0;
				y[0]=ship->pos.y;
				y[1]=ship->pos.y+1.0;
				minz=map.maxZ(x, y);

				if (ship->op == Ship::OPS::RIGHT) if (ship->pos.x < map.width() - 1) {
					ship->pos.x += 0.125;
					if (ship->timemoving>=50 && (int(ship->pos.x*8)%2)==1) ship->pos.x+=0.125;
//					if (ship_timemoving>=100 && (int(shipp.x*4)%2)==1) shipp.x+=0.25;
				} /* if */ 
				if (ship->op==Ship::OPS::LEFT) if (ship->pos.x>0) {
					ship->pos.x-=0.125;
					if (ship->timemoving>=50 && (int(ship->pos.x*8)%2)==1) ship->pos.x-=0.125;
//					if (ship_timemoving>=100 && (int(shipp.x*4)%2)==1) shipp.x-=0.25;
				} /* if */ 
				if (ship->op2==Ship::OPS::FORWARD) if (ship->pos.y < map.height()-1) {
					ship->pos.y+=0.125;
					if (ship->timemoving>=50 && (int(ship->pos.y*8)%2)==1) ship->pos.y+=0.125;
//					if (ship_timemoving>=100 && (int(shipp.y*4)%2)==1) shipp.y+=0.25;
				} /* if */ 
				if (ship->op2==Ship::OPS::BACKWARD) if (ship->pos.y>0) {
					ship->pos.y-=0.125;
					if (ship->timemoving>=50 && (int(ship->pos.y*8)%2)==1) ship->pos.y-=0.125;
//					if (ship_timemoving>=100 && (int(shipp.y*4)%2)==1) shipp.y-=0.25;
				} /* if */ 
				if (ship->op3==Ship::OPS::UP) if (ship->pos.z<5.0) ship->pos.z+=0.05;
				if (ship->op3!=Ship::OPS::UP && ship->pos.z>minz) ship->pos.z-=0.025;
				if (ship->pos.z<minz) ship->pos.z=minz;

				if (ship->op==Ship::OPS::NONE && ship->op2==Ship::OPS::NONE) {
					ship->timemoving=0;
				} else {
					ship->timemoving++;
				} /* if */ 

				if (ship->pos!=old_shipp
//					&&
//					shipp==old_shipp
					) {
                  if (ship->checkCollision(map.buildings, map.robots)) {
						ship->timemoving=0;
						Vector p=ship->pos;
						ship->pos.x=old_shipp.x;
						ship->pos.y=old_shipp.y;
						if (p.z!=old_shipp.z && ship->checkCollision(map.buildings, map.robots)) {
							ship->pos.z=old_shipp.z;
							ship->landed=true;
						} else {
							ship->pos.z=p.z;
						} /* if */ 
						ship->pos.x=p.x;
						if (p.x!=old_shipp.x && ship->checkCollision(map.buildings, map.robots)) {
							ship->pos.x=old_shipp.x;
						} else {
							ship->pos.x=p.x;
						} /* if */ 
						ship->pos.y=p.y;
						if (p.y!=old_shipp.y && ship->checkCollision(map.buildings, map.robots)) {
							ship->pos.y=old_shipp.y;
						} else {
							ship->pos.y=p.y;
						} /* if */ 
					} /* if */ 
				} /* if */ 

				if ((int(ship->pos.x*8)%4)==0) ship->op=Ship::OPS::NONE;
				if ((int(ship->pos.y*8)%4)==0) ship->op2=Ship::OPS::NONE;
				if ((int(ship->pos.z*8)%4)==0) ship->op3=Ship::OPS::NONE;

				if (keyboard[left_key]) {
                  ship->op=Ship::OPS::LEFT;
				} /* if */ 
				if (keyboard[right_key]) {
                  ship->op=Ship::OPS::RIGHT;
				} /* if */ 
				if (keyboard[up_key]) {
                  ship->op2=Ship::OPS::FORWARD;
				} /* if */ 
				if (keyboard[down_key]) {
                  ship->op2=Ship::OPS::BACKWARD;
				} /* if */ 
				if (keyboard[fire_key]) {
                  ship->op3=Ship::OPS::UP;
				} /* if */ 
			}
			break;

		case Menu::TYPE::ROBOT:
			/* Browsing through the ROBOT MENU: */ 
			{
				int i;
				StatusButton::NAME buttons[4]={StatusButton::NAME::ROBOT1,
                                               StatusButton::NAME::ROBOT2,
                                               StatusButton::NAME::ROBOT3,
                                               StatusButton::NAME::ROBOT4};
				StatusButton *b;

				if (keyboard[up_key] && !old_keyboard[up_key]) {
					int j=-1;
					for(i=0;i<4 && j==-1;i++) {
                      if (menu.act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=4;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 
				if (keyboard[down_key] && !old_keyboard[down_key]) {
					int j=-1;
					for(i=0;i<4 && j==-1;i++) {
						if (menu.act_button==buttons[i]) {
							j=i+1;
							if (j>=4) j-=4;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 

				for(i=0;i<4;i++) {
                  b=menu.getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=menu.getbutton(menu.act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 

				if (keyboard[fire_key] && !old_keyboard[fire_key]) {
					switch(menu.act_button) {
					case StatusButton::NAME::ROBOT1:
						{
							StatusButton *b;

							b=menu.getbutton(menu.act_button);
							if (b!=0) {
								b->r=1.0f;
								b->g=0.5f;
								b->b=0.5f;
							} /* if */ 

							menu.act_menu=Menu::TYPE::DIRECTCONTROL;
							menu.needsRedraw=2;
                            sManager.playSelect();
						}
						break;
					case StatusButton::NAME::ROBOT2:
						{
                          menu.replaceMenu(Menu::TYPE::ROBOT, Menu::TYPE::ORDERS,
                                           StatusButton::NAME::ORDERS1);
                          sManager.playSelect();
						}
						break;
					case StatusButton::NAME::ROBOT3:
						{
                          menu.replaceMenu(Menu::TYPE::ROBOT, Menu::TYPE::COMBATMODE,
                                           StatusButton::NAME::COMBAT6);
                          sManager.playSelect();
						}
						break;
					case StatusButton::NAME::ROBOT4:
						/* Back to the general menu: */ 
						controlled->electronics_state+=6;
						controlled->shipover=false;
						controlled=0;
						menu.killmenu(Menu::TYPE::ROBOT);
						menu.newmenu(Menu::TYPE::GENERAL);
						ship->op3=Ship::OPS::UP;
                        sManager.playSelect();
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;

		case Menu::TYPE::DIRECTCONTROL:
			/* Direct control of a robot by the user: */ 
			if (keyboard[fire_key] && !old_keyboard[fire_key]) {
				menu.needsRedraw = 2;
				menu.act_menu=Menu::TYPE::ROBOT;
			} /* if */ 
			break;

		case Menu::TYPE::DIRECTCONTROL2:
			/* Direct control of a robot by the user: */ 
			if (keyboard[fire_key] && !old_keyboard[fire_key]) {
				menu.needsRedraw = 2;
				menu.act_menu=Menu::TYPE::COMBATMODE;
			} /* if */ 
			break;

		case Menu::TYPE::COMBATMODE:
			/* Browsing through the COMBAT MENU: */ 
			{
				int i;
				StatusButton::NAME buttons[6]={StatusButton::NAME::COMBAT1,
                                               StatusButton::NAME::COMBAT2,
                                               StatusButton::NAME::COMBAT3,
                                               StatusButton::NAME::COMBAT4,
                                               StatusButton::NAME::COMBAT5,
                                               StatusButton::NAME::COMBAT6};
				StatusButton *b;

				if (keyboard[up_key] && !old_keyboard[up_key]) {
					int j=-1;
					for(i=0;i<6 && j==-1;i++) {
						if (menu.act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=6;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 
				if (keyboard[down_key] && !old_keyboard[down_key]) {
					int j=-1;
					for(i=0;i<6 && j==-1;i++) {
						if (menu.act_button==buttons[i]) {
							j=i+1;
							if (j>=6) j-=6;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 

				for(i=0;i<6;i++) {
					b=menu.getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=menu.getbutton(menu.act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 
				
				if (keyboard[fire_key] && !old_keyboard[fire_key]) {
					switch(menu.act_button) {
					case StatusButton::NAME::COMBAT1:
						/* Fire Nuclear: */ 
						if ((controlled->angle==0 || controlled->angle==90 ||
							 controlled->angle==180 || controlled->angle==270) &&
							 controlled->hasNuclear() &&
							 controlled->firetimer==0) {
                          controlled->op=Robot::OPERATOR::NUCLEAR;
						} /* if */ 						
						break;
					case StatusButton::NAME::COMBAT2:
						/* Fire Phasers: */ 
						if ((controlled->angle==0 || controlled->angle==90 ||
							 controlled->angle==180 || controlled->angle==270) &&
							 controlled->hasPhasers() &&
                            controlled->op==Robot::OPERATOR::NONE) {
                          controlled->op=Robot::OPERATOR::PHASERS;
						} /* if */ 
						break;
					case StatusButton::NAME::COMBAT3:
						/* Fire Missiles: */ 
						if ((controlled->angle==0 || controlled->angle==90 ||
							 controlled->angle==180 || controlled->angle==270) &&
							 controlled->hasMissiles() &&
							 controlled->op==Robot::OPERATOR::NONE) {
                          controlled->op=Robot::OPERATOR::MISSILES;
						} /* if */ 
						break;
					case StatusButton::NAME::COMBAT4:
						/* Fire Canons: */ 
						if ((controlled->angle==0 || controlled->angle==90 ||
							 controlled->angle==180 || controlled->angle==270) &&
							 controlled->hasCannons() &&
                            controlled->op==Robot::OPERATOR::NONE) {
                          controlled->op=Robot::OPERATOR::CANNONS;
						} /* if */ 
						break;
					case StatusButton::NAME::COMBAT5:
						{
							StatusButton *b;

							b=menu.getbutton(menu.act_button);
							if (b!=0) {
								b->r=1.0f;
								b->g=0.5f;
								b->b=0.5f;
							} /* if */ 

							menu.act_menu=Menu::TYPE::DIRECTCONTROL2;
							menu.needsRedraw=2;
                            sManager.playSelect();
						}
						break;
					case StatusButton::NAME::COMBAT6:
						/* Back to the robot menu: */
                      menu.replaceMenu(Menu::TYPE::COMBATMODE, Menu::TYPE::ROBOT,
                                       StatusButton::NAME::ROBOT3);
                      sManager.playSelect();
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;
		case Menu::TYPE::ORDERS:
			/* Browsing through the ORDERS MENU: */ 
			{
				int i;
				StatusButton::NAME buttons[5]={StatusButton::NAME::ORDERS1,
                                               StatusButton::NAME::ORDERS2,
                                               StatusButton::NAME::ORDERS3,
                                               StatusButton::NAME::ORDERS4,
                                               StatusButton::NAME::ORDERS5};
				StatusButton *b;

				if (keyboard[up_key] && !old_keyboard[up_key]) {
					int j=-1;
					for(i=0;i<5 && j==-1;i++) {
						if (menu.act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=5;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 
				if (keyboard[down_key] && !old_keyboard[down_key]) {
					int j=-1;
					for(i=0;i<5 && j==-1;i++) {
						if (menu.act_button==buttons[i]) {
							j=i+1;
							if (j>=5) j-=5;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 

				for(i=0;i<5;i++) {
					b=menu.getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=menu.getbutton(menu.act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 
				
				if (keyboard[fire_key] && !old_keyboard[fire_key]) {
					switch(menu.act_button) {
					case StatusButton::NAME::ORDERS1:
						/* STOP & DEFEND: */ 
						controlled->program=Robot::PROGRAM_STOPDEFEND;
						controlled->program_goal=Vector(-1,-1,-1);
                        menu.replaceMenu(Menu::TYPE::ORDERS, Menu::TYPE::ROBOT,
                                         StatusButton::NAME::ROBOT2);
                        sManager.playSelect();
						break;
					case StatusButton::NAME::ORDERS2:
						/* ADVANCE ?? MILES: */ 
						controlled->program=Robot::PROGRAM_ADVANCE;
						controlled->program_parameter.as_int = 0;
						controlled->program_goal=Vector(-1,-1,-1);

						menu.killmenu(Menu::TYPE::ORDERS);
						menu.newmenu(Menu::TYPE::SELECTDISTANCE);
                        sManager.playSelect();
						break;
					case StatusButton::NAME::ORDERS3:
						/* RETREAT ?? MILES: */ 
						controlled->program=Robot::PROGRAM_RETREAT;
						controlled->program_parameter.as_int = 0;
						controlled->program_goal=Vector(-1,-1,-1);

						menu.killmenu(Menu::TYPE::ORDERS);
						menu.newmenu(Menu::TYPE::SELECTDISTANCE);
                        sManager.playSelect();
						break;
					case StatusButton::NAME::ORDERS4:
						/* SEARCH AND DESTROY: */
                      menu.replaceMenu(Menu::TYPE::ORDERS, Menu::TYPE::TARGET_DESTROY,
                                       StatusButton::NAME::TARGET1);
                        sManager.playSelect();
						break;
					case StatusButton::NAME::ORDERS5:
						/* SEARCH AND CAPTURE: */
                      menu.replaceMenu(Menu::TYPE::ORDERS, Menu::TYPE::TARGET_CAPTURE,
                                       StatusButton::NAME::TARGET1);
                        sManager.playSelect();
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;

		case Menu::TYPE::SELECTDISTANCE:
			{
				if (keyboard[up_key] && !old_keyboard[up_key]) {
                  controlled->program_parameter.as_int += 10;
                  if (controlled->program_parameter.as_int > 190) controlled->program_parameter.as_int=190;
					controlled->program_goal=Vector(-1,-1,-1);

					menu.needsRedraw=2;
				} /* if */ 
				if (keyboard[down_key] && !old_keyboard[down_key]) {
                  controlled->program_parameter.as_int -= 10;
					if (controlled->program_parameter.as_int < 0) controlled->program_parameter.as_int = 0;
					controlled->program_goal=Vector(-1,-1,-1);

					menu.needsRedraw=2;
				} /* if */ 
				if (keyboard[fire_key] && !old_keyboard[fire_key]) {
                  if (controlled->program_parameter.as_int == 0) controlled->program=Robot::PROGRAM_STOPDEFEND;
					controlled->program_goal=Vector(-1,-1,-1);

                    menu.replaceMenu(Menu::TYPE::SELECTDISTANCE, Menu::TYPE::ROBOT,
                                     StatusButton::NAME::ROBOT2);
                    sManager.playSelect();
				} /* if */ 
			}
			break;

		case Menu::TYPE::TARGET_DESTROY:
			/* Browsing through the SELECT TARGET FOR DESTROYING MENU: */ 
			{
				int i;
				StatusButton::NAME buttons[3]={StatusButton::NAME::TARGET1,
                                               StatusButton::NAME::TARGET2,
                                               StatusButton::NAME::TARGET3};
				StatusButton *b;

				if (keyboard[up_key] && !old_keyboard[up_key]) {
					int j=-1;
					for(i=0;i<3 && j==-1;i++) {
						if (menu.act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=3;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 
				if (keyboard[down_key] && !old_keyboard[down_key]) {
					int j=-1;
					for(i=0;i<3 && j==-1;i++) {
						if (menu.act_button==buttons[i]) {
							j=i+1;
							if (j>=3) j-=3;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 

				for(i=0;i<3;i++) {
					b=menu.getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=menu.getbutton(menu.act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 
				
				if (keyboard[fire_key] && !old_keyboard[fire_key]) {
					switch(menu.act_button) {
					case StatusButton::NAME::TARGET1:
						if (controlled->pieces[0] ||
							controlled->pieces[1] ||
							controlled->pieces[2]) {
                          menu.replaceMenu(Menu::TYPE::TARGET_DESTROY, Menu::TYPE::ROBOT,
                                           StatusButton::NAME::ROBOT2);
                          controlled->program=Robot::PROGRAM_DESTROY;
                          controlled->program_parameter.param = Robot::P_PARAM_ROBOTS;
							controlled->program_goal=Vector(-1,-1,-1);
                        sManager.playSelect();
						} else {
							/* The robot has no standard WEAPONS!: */ 
                            sManager.playWrong();
						} /* if */ 
						break;
					case StatusButton::NAME::TARGET2:
						if (controlled->pieces[3]) {
                          menu.replaceMenu(Menu::TYPE::TARGET_DESTROY, Menu::TYPE::ROBOT,
                                           StatusButton::NAME::ROBOT2);
                          controlled->program=Robot::PROGRAM_DESTROY;
                          controlled->program_parameter.param = Robot::P_PARAM_EFACTORIES;
							controlled->program_goal=Vector(-1,-1,-1);
                            sManager.playSelect();
						} else {
							/* The robot has no NUCLEAR weapons: */ 
                          sManager.playWrong();
						} /* if */ 
						break;
					case StatusButton::NAME::TARGET3:
						if (controlled->pieces[3]) {
                          menu.replaceMenu(Menu::TYPE::TARGET_DESTROY, Menu::TYPE::ROBOT,
                                           StatusButton::NAME::ROBOT2);
                          controlled->program=Robot::PROGRAM_DESTROY;
                          controlled->program_parameter.param = Robot::P_PARAM_WARBASES;
							controlled->program_goal=Vector(-1,-1,-1);
                            sManager.playSelect();
						} else {
							/* The robot has no NUCLEAR weapons: */ 
                          sManager.playWrong();
						} /* if */ 
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;

		case Menu::TYPE::TARGET_CAPTURE:
			/* Browsing through the SELECT TARGET FOR CAPTURING MENU: */ 
			{
				int i;
				StatusButton::NAME buttons[3]={StatusButton::NAME::TARGET1,
                                               StatusButton::NAME::TARGET2,
                                               StatusButton::NAME::TARGET3};
				StatusButton *b;

				if (keyboard[up_key] && !old_keyboard[up_key]) {
					int j=-1;
					for(i=0;i<3 && j==-1;i++) {
						if (menu.act_button==buttons[i]) {
							j=i-1;
							if (j<0) j+=3;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 
				if (keyboard[down_key] && !old_keyboard[down_key]) {
					int j=-1;
					for(i=0;i<3 && j==-1;i++) {
						if (menu.act_button==buttons[i]) {
							j=i+1;
							if (j>=3) j-=3;
							menu.act_button=buttons[j];
						} /* if */ 
					} /* for */ 
					menu.needsRedraw=2;
				} /* if */ 

				for(i=0;i<3;i++) {
					b=menu.getbutton(buttons[i]);
					if (b!=0) {
						b->r=0.0f;
						b->g=0.0f;
						b->b=0.8f;
					} /* if */ 
				} /* if */ 
				b=menu.getbutton(menu.act_button);
				if (b!=0) {
					b->r=0.5f;
					b->g=0.5f;
					b->b=1.0f;
				} /* if */ 

				if (keyboard[fire_key] && !old_keyboard[fire_key]) {
					switch(menu.act_button) {
					case StatusButton::NAME::TARGET1:
                      menu.replaceMenu(Menu::TYPE::TARGET_CAPTURE, Menu::TYPE::ROBOT,
                                       StatusButton::NAME::ROBOT2);
                      controlled->program=Robot::PROGRAM_CAPTURE;
                      controlled->program_parameter.param = Robot::P_PARAM_NFACTORIES;
						controlled->program_goal=Vector(-1,-1,-1);
                        sManager.playSelect();
						break;
					case StatusButton::NAME::TARGET2:
                      menu.replaceMenu(Menu::TYPE::TARGET_CAPTURE, Menu::TYPE::ROBOT,
                                       StatusButton::NAME::ROBOT2);
                      controlled->program=Robot::PROGRAM_CAPTURE;
                      controlled->program_parameter.param = Robot::P_PARAM_EFACTORIES;
						controlled->program_goal=Vector(-1,-1,-1);
                        sManager.playSelect();
						break;
					case StatusButton::NAME::TARGET3:
                      menu.replaceMenu(Menu::TYPE::TARGET_CAPTURE, Menu::TYPE::ROBOT,
                                       StatusButton::NAME::ROBOT2);
                      controlled->program=Robot::PROGRAM_CAPTURE;
                      controlled->program_parameter.param = Robot::P_PARAM_WARBASES;
						controlled->program_goal=Vector(-1,-1,-1);
                        sManager.playSelect();
						break;
					} /* switch */ 
				} /* if */ 
			}
			break;
		} /* switch */ 


#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Viewport\n");
	fflush(debug_fp);
#endif

		viewp.x=ship->pos.x+0.5;
		viewp.y=ship->pos.y+0.5;
		viewp.z=ship->pos.z-3*camera.zoom;
		if (viewp.z<0) viewp.z=0;
		if (viewp.x<3*camera.zoom) {
			viewp.x=3*camera.zoom;
			if (viewp.x>map.width()-3*camera.zoom) viewp.x=map.width()/2;
		} else {
			if (viewp.x>map.width()-3*camera.zoom) {
				viewp.x=map.width()-3*camera.zoom;
				if (viewp.x<3*camera.zoom) viewp.x=map.width()/2;
			} /* if */ 
		} /* if */ 
		if (viewp.y<3*camera.zoom) {
			viewp.y=3*camera.zoom;
			if (viewp.y>map.height()-3*camera.zoom) viewp.y=map.height()/2; 
		} else {
			if (viewp.y>map.height()-3*camera.zoom) {
				viewp.y=map.height()-3*camera.zoom;
				if (viewp.y<3*camera.zoom) viewp.y=map.height()/2; 
			} /* if */ 
		} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Resources\n");
	fflush(debug_fp);
#endif

    if (stats.tick(level)) {
      menu.needsRedraw = 2;
      StatusButton* timeb = menu.getbutton(StatusButton::NAME::TIME);
      if (timeb != 0) {
        std::ostringstream t1Formatter;
        t1Formatter << "Day: " << stats.day;
        timeb->text1 = t1Formatter.str();
        std::ostringstream t2Formatter;
        t2Formatter << "Hour: " << std::setw(2) << stats.hour << ':' << std::setw(2) << stats.minute;
        timeb->text2 = t2Formatter.str();
      }
    }

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Ship landing\n");
	fflush(debug_fp);
#endif

		/* Test if the ship has landed over a Factory: */ 
		{
          for (const Building& b: map.buildings) {
				if (b.type==Building::TYPE::WARBASE && b.owner==1 && 
					ship->pos.x==b.pos.x && ship->pos.y==b.pos.y && ship->landed) {
                  constructionScreen.open(b);
				} /* if */ 
			} /* while */ 
		}

		/* Test if the ship has landed over a robot: */ 
		if (menu.act_menu==Menu::TYPE::GENERAL &&
			(int(ship->pos.x*8)%4)==0 &&
			(int(ship->pos.y*8)%4)==0) {
            for (Robot* r: map.robots[0]) {
				if (ship->pos.x==(r->pos.x-0.5) && ship->pos.y==(r->pos.y-0.5) && ship->landed) {
					/* The ship has landed over a robot: */ 
					r->shipover=true;
					controlled=r;
					if (controlled->op==Robot::OPERATOR::FORWARD) controlled->op=Robot::OPERATOR::NONE;
					if (controlled->program==Robot::PROGRAM_FORWARD) controlled->program=Robot::PROGRAM_STOPDEFEND;
					menu.replaceMenu(Menu::TYPE::GENERAL, Menu::TYPE::ROBOT,
                                     StatusButton::NAME::ROBOT4);
				} /* while */ 
			} /* while */ 
		} /* if */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Robots\n");
	fflush(debug_fp);
#endif

		/* Robot cycles: */ 
		{
			float x[2],y[2],minz;
			Vector old_pos;
			int old_chassis_state;
			int terrain;
			
			for (int i = 0; i < 2; i++) {

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Player %i robots\n",i);
	fflush(debug_fp);
#endif

                for (Robot* r: map.robots[i]) {
					/* Robot cycle: */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"START ROBOT\n");
	fflush(debug_fp);
#endif
					
					/* Animations: */ 
					if (r->electronics_state!=0) {
						r->electronics_state+=6;
						if (r->electronics_state>=360) r->electronics_state=0;
					} /* if */ 
					
					/* Apply ROBOT operator: */ 
					old_pos=r->pos;
					old_chassis_state=r->chassis_state;
					if (r->traction==2) {
						r->chassis_state++;
					} /* if */ 
					x[0]=r->pos.x-0.5;
					x[1]=r->pos.x+0.5;
					y[0]=r->pos.y-0.5;
					y[1]=r->pos.y+0.5;
					minz=map.maxZ(x,y);
					terrain=map.worseTerrain(x,y);

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Avoid robot deadlock\n");
	fflush(debug_fp);
#endif

					/* Avoid that a Robot can walk agains another and they both get stuck: */ 
    if (r->op==Robot::OPERATOR::FORWARD &&
						(int(r->pos.x*256)%128)==0 &&
						(int(r->pos.y*256)%128)==0) {
						switch(r->angle) {
						case 0:
                          if (ai.worseMapTerrain(int((r->pos.x+0.5)/0.5),int((r->pos.y-0.5)/0.5),1,2) > T_HOLE)
                            r->op=Robot::OPERATOR::NONE;
							break;
						case 90:
							if (ai.worseMapTerrain(int((r->pos.x-0.5)/0.5),int((r->pos.y+0.5)/0.5),2,1) > T_HOLE)
                              r->op=Robot::OPERATOR::NONE;
							break;
						case 180:
							if (ai.worseMapTerrain(int((r->pos.x-1.0)/0.5),int((r->pos.y-0.5)/0.5),1,2) > T_HOLE)
                              r->op=Robot::OPERATOR::NONE;
							break;
						case 270:
							if (ai.worseMapTerrain(int((r->pos.x-0.5)/0.5),int((r->pos.y-1.0)/0.5),2,1) > T_HOLE)
                              r->op=Robot::OPERATOR::NONE;
							break;
						} /* switch */ 
					} /* if */ 

    if (r->op==Robot::OPERATOR::FORWARD) {
						float speed = r->robotSpeed(terrain);

						/* BIPOD ANIMATION: */ 
						if (r->traction==0) {
							r->chassis_state+=int(speed/0.00390625);
							if (r->chassis_state>64) r->chassis_state=-63;
						} /* if */ 

						/* TRACKS PARTICLES: */ 
						if (r->traction==1) {
							if (detaillevel>=4) {
								Vector pos,sp1;

								for (int i= 0; i < 2; i++) {
									pos.x=r->pos.x+float(rand()%10)/100.0;
									pos.y=r->pos.y+float(rand()%10)/100.0;
									pos.z=0;
                                    Color color(0.9F+float(rand()%21-10)/100.0,
                                                0.7F+float(rand()%21-10)/100.0,
                                                0.5F+float(rand()%21-10)/100.0);
									switch(r->angle) {
									case 0:sp1=Vector(-0.05,float(rand()%9-4)/200.0,0);
										pos.x-=0.25;
										pos.y+=((rand()%2)==0 ? -0.5 : 0.5);
										break;
									case 90:sp1=Vector(float(rand()%9-4)/200.0,-0.05,0);
										pos.y-=0.25;
										pos.x+=((rand()%2)==0 ? -0.5 : 0.5);
										break;
									case 180:sp1=Vector(0.05,float(rand()%9-4)/200.0,0);
										pos.x+=0.25;
										pos.y+=((rand()%2)==0 ? -0.5 : 0.5);
										break;
									case 270:sp1=Vector(float(rand()%9-4)/200.0,0.05,0);
										pos.y+=0.25;
										pos.x+=((rand()%2)==0 ? -0.5 : 0.5);
										break;
									} /* switch */ 									
									map.particles.emplace_back(pos, sp1, Vector(0, 0, 0.05), 0, 0.3, color, 1.0, 0.0, 20+ (rand() % 10));
								} /* for */ 
							} /* if */ 
						} /* if */ 

						switch(r->angle) {
						case 0:if (r->pos.x<map.width()-0.5) r->pos.x+=speed;
							break;
						case 90:if (r->pos.y>0.5) r->pos.y+=speed;
							break;
						case 180:if (r->pos.x>0.5) r->pos.x-=speed;
							break;
						case 270:if (r->pos.y<map.height()-0.5) r->pos.y-=speed;
							break;
						} /* switch */ 
					} else {
						if (r->traction==0) r->chassis_state=0;
					} /* if */ 

    if (r->op==Robot::OPERATOR::LEFT) r->angle -= r->robotRotationSpeed(terrain);
					if (r->op==Robot::OPERATOR::RIGHT) r->angle += r->robotRotationSpeed(terrain);
					if (r->angle>=360) r->angle-=360;
					if (r->angle<0) r->angle+=360;

					if (r->op==Robot::OPERATOR::CANNONS && r->firetimer==0) {
						Vector pos(r->pos);
						pos.z = r->piecez(0) + 0.3f;
                        map.bullets.emplace_back(Bullet::TYPE::CANNONS, pos, r);
                        sManager.playShot(ship->pos, r->pos);
					} /* if */ 

					if (r->op==Robot::OPERATOR::MISSILES && r->firetimer==0) {
						Vector pos (r->pos);
						pos.z = r->piecez(1) + 0.2f;
						map.bullets.emplace_back(Bullet::TYPE::MISSILES, pos, r);
                        sManager.playShot(ship->pos, r->pos);
					} /* if */ 

					if (r->op==Robot::OPERATOR::PHASERS && r->firetimer==0) {
						Vector pos(r->pos);
						pos.z = r->piecez(2) + 0.3f;
						map.bullets.emplace_back(Bullet::TYPE::PHASERS, pos, r);
                        sManager.playShot(ship->pos, r->pos);
					} /* if */ 

					if (r->op==Robot::OPERATOR::CANNONS ||
						r->op==Robot::OPERATOR::MISSILES ||
						r->op==Robot::OPERATOR::PHASERS)
                      r->firetimer++;

					if (r->op==Robot::OPERATOR::NUCLEAR) {
                      Explosion exp(r->pos, 2);

                      map.explosions.push_back(exp);

						/* Robot destroyed: */ 
						if (r==controlled) {
							controlled->shipover=false;
							controlled=0;
							menu.killmenu(menu.act_menu);
							menu.newmenu(Menu::TYPE::GENERAL);
						} /* if */ 

						/* Find Robots to destroy: */ 
                        for(int i = 0; i < 2; i++) {
                          map.robots[i].erase(std::remove_if(map.robots[i].begin(), map.robots[i].end(),
                                                         [exp, this] (auto& r) {
                                                           float distance=(r->pos - exp.pos).norma();
                                                           if (distance <= NUCLEAR_RADIUS) {
                                                             ai.killRobot(r->pos);
                                                             return true;
                                                           } else {
                                                             return false;
                                                           }
                                                         }),
                                          map.robots[i].end());
                        }

						/* Find buildings to destroy: */ 
                        map.buildings.erase(std::remove_if(map.buildings.begin(), map.buildings.end(),
                                                       [exp, this](auto& b) {
                                                         float distance = (b.pos - (exp.pos - Vector(0.5, 0.5, 0.5))).norma();
                                                         if (distance <= NUCLEAR_RADIUS) {
                                                           ai.removeBuilding(b.pos);
                                                           return true;
                                                         } else {
                                                           return false;
                                                         }
                                                       }),
                                        map.buildings.end());
                        sManager.playExplosion(ship->pos, r->pos);
                        stats.requestRecomputing();
					} /* if */ 

                    //!					if (r!=0) {
                    {
						x[0]=r->pos.x-0.5;
						x[1]=r->pos.x+0.5;
						y[0]=r->pos.y-0.5;
						y[1]=r->pos.y+0.5;
						minz=map.maxZ(x, y);
						terrain=map.worseTerrain(x, y);
						r->pos.z=minz;

						if (r->shipover) {
							ship->pos.x=r->pos.x-0.5;
							ship->pos.y=r->pos.y-0.5;
							ship->pos.z=r->pos.z+r->cmc.z[1];
						} /* if */ 

						/* Collision: */ 
						if (r->checkCollision(map.buildings, map.robots, false, ship) || !r->walkable(terrain)) {
							r->pos=old_pos;
							if (r->traction==0) r->chassis_state=old_chassis_state;
							if (r->shipover) {
								ship->pos.x=r->pos.x-0.5;
								ship->pos.y=r->pos.y-0.5;
								ship->pos.z=r->pos.z+r->cmc.z[1];
							} /* if */ 
						} else {
							ai.moveRobot(old_pos,r->pos,i);
						} /* if */ 
						
						if (r->op==Robot::OPERATOR::FORWARD && (r->angle==0 || r->angle==180) && (int(r->pos.x*256)%128)==0)
                          r->op=Robot::OPERATOR::NONE;
						if (r->op==Robot::OPERATOR::FORWARD && (r->angle==90 || r->angle==270) && (int(r->pos.y*256)%128)==0)
                          r->op=Robot::OPERATOR::NONE;
						if (r->op==Robot::OPERATOR::LEFT && (r->angle%90)==0) r->op=Robot::OPERATOR::NONE;
						if (r->op==Robot::OPERATOR::RIGHT && (r->angle%90)==0) r->op=Robot::OPERATOR::NONE;
						if (r->op==Robot::OPERATOR::CANNONS && r->firetimer>=64) {
							r->op=Robot::OPERATOR::NONE;
							r->firetimer=0;
						} /* if */ 
						if (r->op==Robot::OPERATOR::MISSILES && r->firetimer>=64) {
							r->op=Robot::OPERATOR::NONE;
							r->firetimer=0;
						} /* if */ 
						if (r->op==Robot::OPERATOR::PHASERS && r->firetimer>=64) {
							r->op=Robot::OPERATOR::NONE;
							r->firetimer=0;
						} /* if */ 

						/* Follow ROBOT program: */ 
						if (r->op==Robot::OPERATOR::NONE && !r->shipover) {
#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Robot program: %i\n",r->program);
	fflush(debug_fp);
#endif
							switch(r->program) {
							case Robot::PROGRAM_NONE:
								break;
							case Robot::PROGRAM_FORWARD:
								r->op=Robot::OPERATOR::FORWARD;
								break;
							case Robot::PROGRAM_STOPDEFEND:
                              r->op = ai.programStopDefend(*r, &(r->program_goal), i + 1);
								break;
							case Robot::PROGRAM_ADVANCE:
                              r->op=ai.programAdvance(*r, i + 1);
								if (r->op==Robot::OPERATOR::FORWARD && r->angle==90) r->program_parameter.as_int--;
								if (r->op==Robot::OPERATOR::FORWARD && r->angle==270) r->program_parameter.as_int++;
								if (r->program_parameter.as_int == 0) r->program=Robot::PROGRAM_STOPDEFEND;
								break;
							case Robot::PROGRAM_RETREAT:
                              r->op=ai.programRetreat(*r, i + 1);
								if (r->op==Robot::OPERATOR::FORWARD && r->angle==270) r->program_parameter.as_int--;
								if (r->op==Robot::OPERATOR::FORWARD && r->angle==90) r->program_parameter.as_int++;
								if (r->program_parameter.as_int == 0) r->program=Robot::PROGRAM_STOPDEFEND;
								break;
							case Robot::PROGRAM_DESTROY:
								r->op = ai.programDestroy(*r, &(r->program_goal), i + 1);
								// if (r->program_goal.x==-1) r->program=PROGRAM_STOPDEFEND;
								break;
							case Robot::PROGRAM_CAPTURE:
								r->op = ai.programCapture(*r, &(r->program_goal), i + 1);
								// if (r.program_goal.x==-1) r.program=PROGRAM_STOPDEFEND;
								break;
							} /* switch */ 
						} /* if */ 

						/* Follow USER's command: */ 
						if (r->op==Robot::OPERATOR::NONE && r->shipover &&
							(menu.act_menu==Menu::TYPE::DIRECTCONTROL ||
							 menu.act_menu==Menu::TYPE::DIRECTCONTROL2)) {
							if (keyboard[right_key]) {
								if (r->angle==0) {
									r->op=Robot::OPERATOR::FORWARD;
								} else {
									if (r->angle==270) r->op=Robot::OPERATOR::RIGHT;
												  else r->op=Robot::OPERATOR::LEFT;
								} /* if */ 
							} /* if */ 
							if (keyboard[left_key]) {
								if (r->angle==180) {
									r->op=Robot::OPERATOR::FORWARD;
								} else {
									if (r->angle==90) r->op=Robot::OPERATOR::RIGHT;
												 else r->op=Robot::OPERATOR::LEFT;
								} /* if */ 
							} /* if */ 
							if (keyboard[up_key]) {
								if (r->angle==90) {
									r->op=Robot::OPERATOR::FORWARD;
								} else {
									if (r->angle==0) r->op=Robot::OPERATOR::RIGHT;
												else r->op=Robot::OPERATOR::LEFT;
								} /* if */ 
							} /* if */ 
							if (keyboard[down_key]) {
								if (r->angle==270) {
									r->op=Robot::OPERATOR::FORWARD;
								} else {
									if (r->angle==180) r->op=Robot::OPERATOR::RIGHT;
												  else r->op=Robot::OPERATOR::LEFT;
								} /* if */ 
							} /* if */ 

						} /* if */ 
					} /* if */ 
				} /* while */ 

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Deleting robots\n",i);
	fflush(debug_fp);
#endif

			} /* for */ 
		}

#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Buildings\n");
	fflush(debug_fp);
#endif

    /* Buildings: */ 
    for (Building& b: map.buildings) {
      if (b.type==Building::TYPE::FACTORY_ELECTRONICS ||
          b.type==Building::TYPE::FACTORY_NUCLEAR ||
          b.type==Building::TYPE::FACTORY_PHASERS ||
          b.type==Building::TYPE::FACTORY_MISSILES ||
          b.type==Building::TYPE::FACTORY_CANNONS	||
          b.type==Building::TYPE::FACTORY_CHASSIS) {
        int robot=ai.robotHere(b.pos+Vector(1,0,0));
        if (robot==0) {
          b.status=0;
        } else {
          if (robot==T_ROBOT) b.status++;
          if (robot==T_EROBOT) b.status--;

          if (b.status>=12*12*12) {
            b.owner=1;
            b.status=0;
            stats.requestRecomputing();
          } /* if */ 
          if (b.status<=-12*12*12) {
            b.owner=2;
            b.status=0;
            stats.requestRecomputing();
          } /* if */ 
        } /* if */ 
      } /* if */ 

      if (b.type==Building::TYPE::WARBASE) {
        int robot=ai.robotHere(b.pos+Vector(2,0,0));
        if (robot==0) {
          b.status=0;
        } else {
          if (robot==T_ROBOT) b.status++;
          if (robot==T_EROBOT) b.status--;
          if (b.status>=12*12*12) {
            b.owner=1;
            b.status=0;
            stats.requestRecomputing();
          } /* if */ 

          if (b.status<=-12*12*12) {
            b.owner=2;
            b.status=0;
            stats.requestRecomputing();
          } /* if */ 
        } /* if */ 
      } /* if */ 
    } /* while */ 


#ifdef _WRITE_REPORT_
	fprintf(debug_fp,"Bullets\n");
	fflush(debug_fp);
#endif

    /* Bullets: */

    map.bullets.erase(remove_if(map.bullets.begin(), map.bullets.end(),
                            [this](auto& bullet) {
                              bool ret = false;

                              int persistence = CANNON_PERSISTENCE;
                              if (bullet.angle == 0) bullet.pos.x += BULLET_SPEED;
                              if (bullet.angle == 90) bullet.pos.y += BULLET_SPEED;
                              if (bullet.angle == 180) bullet.pos.x -= BULLET_SPEED;
                              if (bullet.angle == 270) bullet.pos.y -= BULLET_SPEED;
                              bullet.step++;

                              if (bullet.type == Bullet::TYPE::MISSILES) persistence = MISSILE_PERSISTENCE;
                              if (bullet.type == Bullet::TYPE::PHASERS) persistence = PHASER_PERSISTENCE;
                              Robot* r = 0;
                              if (bullet.step >= persistence || bullet.checkCollision(map.buildings, map.robots, &r)) {
                                ret = true;
                                if (bullet.step < persistence) {
                                  map.explosions.emplace_back(bullet.pos, 0);
                                }
                              }
                              if (r != 0) {
                                /* The bullet has collided with a robot: */
                                if (!r->bulletHit(bullet.type)) {
                                  /* Robot destroyed: */
                                  map.explosions.emplace_back(r->pos,1);
                                  sManager.playExplosion(ship->pos, r->pos);
                                  if (r == controlled) {
                                    controlled->shipover = false;
                                    controlled = 0;
                                    menu.killmenu(Menu::TYPE::ALL);
                                    menu.newmenu(Menu::TYPE::GENERAL);
                                  }
                                  ai.killRobot(r->pos);

                                  find_and_destroy_robot(map.robots, r);
                                }
                              }
                              return ret;
                            }),
                  map.bullets.end());

    map.cycle();
    menu.cycle();

    if (gameState == NETHER::STATE::PLAYING && keyboard[pause_key] && !old_keyboard[pause_key]) {
      optionsScreen.open();
    }

    if ((stats.stats[0][0] == 0 || stats.stats[1][0] == 0) && gameFinished == 0) {
      gameFinished++;
      gameStarted = 0;
    }
    if (gameFinished > 0) gameFinished++;
    if (gameFinished >= END_TIME) return false;

    if (gameStarted > 0) gameStarted--;

    return true;
  }
}
