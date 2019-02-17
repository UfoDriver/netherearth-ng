#ifdef _WIN32
#include "windows.h"
#endif

#include <string>

#include "stdio.h"
#include "string.h"

#include "GL/gl.h"
#include "GL/glu.h"
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "nether.h"
#include "utils.h"

#include "glprintf.h"

extern int detaillevel;
extern float MINY,MAXY,MINX,MAXX;


void NETHER::drawmap(bool shadows)
{
  Vector light(lightpos[0], lightpos[1], lightpos[2]);
  light=light / light.z;

	glMatrixMode(GL_MODELVIEW);

    map.draw(viewp, shadows);

	for (const Building& b: buildings) {

		if (b.pos.y>=(viewp.y+MINY) &&
			b.pos.y<=(viewp.y+MAXY) &&
			b.pos.x>=(viewp.x+MINX) &&
			b.pos.x<=(viewp.x+MAXX)) {
			glPushMatrix();
			glTranslatef(float(b.pos.x),float(b.pos.y),float(b.pos.z));
			switch(b.type) {
			case Building::TYPE::FENCE:if (!shadows) {
                if (detaillevel>=2) Resources::buildingTiles[5].draw(Color(0.2f, 0.2f, 0.2f));
                else Resources::buildingTiles[5].draw_notexture(Color(0.2f, 0.2f, 0.2f));
						 } else {
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[5].DrawShadow(Color(0, 0, 0, 0.5));
						 } /* if */ 
						 break;
			case Building::TYPE::WALL1:if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[0].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[0].draw_notexture(Color(0.5, 0.5, 0.5));
						 } else {
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[0].DrawShadow(Color(0, 0, 0, 0.5));
						 } /* if */ 
						 break;
			case Building::TYPE::WALL2:if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[1].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[1].draw_notexture(Color(0.5, 0.5, 0.5));
						 } else { 
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[1].DrawShadow(Color(0, 0, 0, 0.5));
						 } /* if */ 
						 break;
			case Building::TYPE::WALL3:if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[2].draw(Color(0.3f, 0.3f, 0.3f));
                           else Resources::buildingTiles[2].draw_notexture(Color(0.5, 0.5, 0.5));
						 } else {
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[2].DrawShadow(Color(0, 0, 0, 0.5));
						 } /* if */ 
						 break;
			case Building::TYPE::WALL4:if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[3].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[3].draw_notexture(Color(0.5, 0.5, 0.5));
						 } else {
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[3].DrawShadow(Color(0, 0, 0, 0.5));
						 } /* if */ 
						 break;
			case Building::TYPE::WALL5:if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
						 } else {
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
						 } /* if */ 
						 break;
			case Building::TYPE::WALL6:if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[7].draw(Color(0.3f, 0.3f, 0.3f));
                           else Resources::buildingTiles[7].draw_notexture(Color(0.5, 0.5, 0.5));
						 } else {
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[7].DrawShadow(Color(0, 0, 0, 0.5));
						 } /* if */ 
						 break;
			case Building::TYPE::FACTORY_ELECTRONICS:
						 if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
							 glPushMatrix();
							 glTranslatef(0.5,0.5,1);
							 Resources::pieceTiles[0][7].draw_notexture(Color(0.8f, 0.8f, 0.8f));
							 glPopMatrix();
						 } else {
							 glPushMatrix();
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
							 glTranslatef(float(-light.x),float(-light.y),0);
							 Resources::pieceTiles[0][7].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
							 glPopMatrix();
						 } /* if */ 

						 if (b.owner==1) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 if (b.owner==2) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 break;
			case Building::TYPE::FACTORY_NUCLEAR:
						 if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
							 glPushMatrix();
							 glTranslatef(0.5,0.5,1);
							 Resources::pieceTiles[0][6].draw_notexture(Color(0.8f, 0.8f, 0.8f));
							 glPopMatrix();
						 } else {
							 glPushMatrix();
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
							 glTranslatef(float(-light.x),float(-light.y),0);
							 Resources::pieceTiles[0][6].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
							 glPopMatrix();
						 } /* if */ 

						 if (b.owner==1) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 if (b.owner==2) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.8f,0.3f,0.3f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 break;
			case Building::TYPE::FACTORY_PHASERS:
						 if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
							 glPushMatrix();
							 glTranslatef(0.5,0.5,1);
							 Resources::pieceTiles[0][5].draw_notexture(Color(0.8f, 0.8f, 0.8f));
							 glPopMatrix();
						 } else {
							 glPushMatrix();
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
							 glTranslatef(float(-light.x),float(-light.y),0);
							 Resources::pieceTiles[0][5].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
							 glPopMatrix();
						 } /* if */ 

						 if (b.owner==1) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 if (b.owner==2) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 break;
			case Building::TYPE::FACTORY_MISSILES:
						 if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
							 glPushMatrix();
							 glTranslatef(0.5,0.5,1);
							 Resources::pieceTiles[0][4].draw_notexture(Color(0.8f, 0.8f, 0.8f));
							 glPopMatrix();
						 } else {
							 glPushMatrix();
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
							 glTranslatef(float(-light.x),float(-light.y),0);
							 Resources::pieceTiles[0][4].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
							 glPopMatrix();
						 } /* if */ 

						 if (b.owner==1) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 if (b.owner==2) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 break;
			case Building::TYPE::FACTORY_CANNONS:
						 if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
							 glPushMatrix();
							 glTranslatef(0.5,0.5,1);
							 Resources::pieceTiles[0][3].draw_notexture(Color(0.8f, 0.8f, 0.8f));
							 glPopMatrix();
						 } else {
							 glPushMatrix();
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
							 glTranslatef(float(-light.x),float(-light.y),0);
							 Resources::pieceTiles[0][3].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
							 glPopMatrix();
						 } /* if */ 

						 if (b.owner==1) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 if (b.owner==2) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 break; 
			case Building::TYPE::FACTORY_CHASSIS:
						 if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[4].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[4].draw_notexture(Color(0.5, 0.5, 0.5));
							 glPushMatrix();
							 glTranslatef(0.5,0.5,1);
							 Resources::pieceTiles[0][1].draw_notexture(Color(0.8f, 0.8f, 0.8f));
							 glPopMatrix();
						 } else {
							 glPushMatrix();
							 glTranslatef(0,0,0.05f);
							 Resources::buildingTiles[4].DrawShadow(Color(0, 0, 0, 0.5));
							 glTranslatef(float(-light.x),float(-light.y),0);
							 Resources::pieceTiles[0][1].DrawShadow(0, lightposv, Color(0, 0, 0, 0.5));
							 glPopMatrix();
						 } /* if */ 

						 if (b.owner==1) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 if (b.owner==2) {
							if (!shadows) {
								glTranslatef(0,-1,1);
								Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
							} else {
								glTranslatef(float(-light.x),float(-light.y)-1,0.05f);
								Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							} /* if */ 
						 } /* if */ 
						 break;

			case Building::TYPE::WARBASE:if (!shadows) {
                           if (detaillevel>=2) Resources::buildingTiles[8].draw(Color(0.5, 0.5, 0.5));
                           else Resources::buildingTiles[8].draw_notexture(Color(0.5, 0.5, 0.5));
						   } else {
							   glTranslatef(0,0,0.05f);
							   Resources::buildingTiles[8].DrawShadow(Color(0, 0, 0, 0.5));
						   } /* if */ 

						   if (b.owner==1) {
							   if (!shadows) {
								   glTranslatef(0,-2,1);
								   Resources::buildingTiles[6].draw(Color(0.3f, 0.3f, 0.8f));
							   } else {
								   glTranslatef(float(-light.x),float(-light.y)-2,0.05f);
								   Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							   } /* if */ 
						   } /* if */ 
						   if (b.owner==2) {
							  if (!shadows) {
								 glTranslatef(0,-2,1);
								 Resources::buildingTiles[6].draw(Color(0.8f, 0.3f, 0.3f));
							  } else {
								 glTranslatef(float(-light.x),float(-light.y)-2,0.05f);
								 Resources::buildingTiles[6].DrawShadow(Color(0, 0, 0, 0.5));
							  } /* if */ 
						   } /* if */ 
						   break;
			} /* switch */ 
			glPopMatrix();
		} /* if */ 
	} /* while */ 

} /* drawmap */ 
