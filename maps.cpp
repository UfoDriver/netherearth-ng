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


bool NETHER::loadMap(const std::string& filename)
{
  std::ifstream iFile(filename);
  iFile >> map_w >> map_h;
  map.clear();
  map.reserve(map_w * map_h);

  const std::vector<std::string> tiles = {"G", "S", "S2", "M", "H1",
                                          "H2", "H3", "H4", "H5", "H6",
                                          "GG", "SS", "MM", "?"};
  std::string tilestr;
  for (int i = 0; i < map_w * map_h; i++) {
    iFile >> tilestr;
    int tile = find_index(tiles, tilestr);
    if (tile == 10) tile = 0;
    if (tile == 11) tile = 1;
    if (tile == 12) tile = 3;

    map.push_back(tile);
  }

  while (!iFile.eof()) {
    const std::vector<Building>& newBuildings {Building::readMapFile(iFile)};
    std::copy(newBuildings.cbegin(), newBuildings.cend(), std::back_inserter(buildings));
  }
  return true;
}


void NETHER::drawmap(bool shadows)
{
	int i,j;
	int o,m[8]={13,15,17,19,7,23,21,25};
	Vector light;

	light.x=lightpos[0];
	light.y=lightpos[1];
	light.z=lightpos[2];
	light=light/light.z;

	glMatrixMode(GL_MODELVIEW);
	
	if (!shadows) {
		glPushMatrix();
		for(j=0;j<map_h;j++) {
			if (j>=(viewp.y+MINY) &&
				j<=(viewp.y+MAXY)) {
				glPushMatrix();
				for(i=0;i<map_w;i++) {
					if (i>=(viewp.x+MINX) &&
						i<=(viewp.x+MAXX)) {
						o=map[i+j*map_w];

						if (o==0) {
							/* GRASS: */ 
							if (((i*3+j*7)%m[(i+j)%8])==0) o=10;
							if (((i*3+j*7)%m[(i+j+1)%8])==0) o=11;
						} else {
						} /* if */ 
						if (detaillevel>=1) tiles[o].draw(tiles[0].color);
                        else tiles[o].draw_notexture(tiles[0].color);
					} /* if */ 
					glTranslatef(1.0,0.0,0.0);
				} /* for */ 
				glPopMatrix();
			} /* if */ 
			glTranslatef(0.0,1.0,0.0);
		} /* for */ 
		glPopMatrix();
	} /* if */ 

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


float NETHER::MapMaxZ(float x[2],float y[2])
{
	int i,j;
	float z=0;
	int o;

	for(i=int(x[0]);float(i)<x[1];i++) {
		for(j=int(y[0]);float(j)<y[1];j++) {
			if (i>=0 && i<map_w &&
				j>=0 && j<map_h) {
				o=map[i+j*map_w];
				if (tiles[o].cmc.z[0]>z) z=tiles[o].cmc.z[0];
				if (tiles[o].cmc.z[1]>z) z=tiles[o].cmc.z[1];
			} /* if */ 
		} /* for */ 
	} /* for */ 

	return z;
} /* MapMaxZ */ 


int NETHER::MapTerrain(float x, float y)
{
	switch(map[int(x)+int(y)*map_w]) {
	case 0:
		return T_GRASS;
		break;
	case 1:
	case 2:
		return T_SAND;
		break;
	case 3:
		return T_MOUNTAINS;
		break;
	default:
		return T_HOLE;
		break;
	} /* switch */ 
} /* NETHER::MapTerrain */ 


int NETHER::WorseMapTerrain(float x[2], float y[2])
{
	int t,t2;

	t=MapTerrain(x[0]+0.001f,y[0]+0.001f);
	t2=MapTerrain(x[1]-0.001f,y[0]+0.001f);
	if (t2==T_HOLE || t==T_HOLE) return T_HOLE;
	if ((t2==T_MOUNTAINS && (t==T_SAND || t==T_GRASS)) ||
		(t2==T_SAND && t==T_GRASS)) t=t2;
	t2=MapTerrain(x[0]+0.001f,y[1]-0.001f);
	if (t2==T_HOLE) return T_HOLE;
	if ((t2==T_MOUNTAINS && (t==T_SAND || t==T_GRASS)) ||
		(t2==T_SAND && t==T_GRASS)) t=t2;
	t2=MapTerrain(x[1]-0.001f,y[1]-0.001f);
	if (t2==T_HOLE) return T_HOLE;
	if ((t2==T_MOUNTAINS && (t==T_SAND || t==T_GRASS)) ||
		(t2==T_SAND && t==T_GRASS)) t=t2;
	return t;
} /* NETHER::WorseMapTerrain */ 

