#ifdef _WIN32
#include "windows.h"
#endif

#include "radar.h"

#include "string.h"
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

extern bool show_radar;


void Radar::draw(const int width, const int height, const int split, const int splity)
{
  if (show_radar && needsRedraw <= 1) {
    float lightpos2[4] = {0, 0, 1000, 0};
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos2);
    glClearColor(0.0, 0.0, 0,0);
    glViewport(0, 0, split, splity);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, float(split), 0, float(splity), -100, 100);
    glScissor(0, 0, split, splity);
    glScalef(width / 640.0, height / 480.0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(64.0, 8.0, 0.0);
    glColor3f(0.5f, 0.5f, 1.0f);
    scaledglprintf(0.18f, 0.4f, "RADAR:");

    glLoadIdentity();
    glScalef(4.0, 4.0, 4.0);

    int maxy = 94, maxx = 16;

    int startx = (int)((nether->ship->pos.x - 4) * 2);
    int starty = (int)((nether->ship->pos.y - 23) * 2);
    // @TODO: std::min/std::max can be used here. Or, event better, std::clamp (c++17)
    if ((starty + maxy) > (nether->map.height() * 2)) starty = (nether->map.height() * 2) - maxy;
    if (starty < 0) starty = 0;
    if ((startx + maxx) > (nether->map.width() * 2)) startx = (nether->map.width() * 2) - maxx;
    if (startx < 0) startx = 0;

    glNormal3f(0, 0, 1);
    for (int y = 0; y < maxy; y++) {
      for (int x = 0; x < maxx; x++) {
        if (x + startx < (nether->map.width() * 2) &&
            y + starty < (nether->map.height() * 2) &&
            nether->ai.discreetmapSize()) {
          switch(nether->ai.discreetmapValue(x + startx + (y + starty) * (nether->map.width() * 2))) {
          case T_GRASS:
            glColor3f(0.0, 1.0, 0.0);
            break;
          case T_SAND:
            glColor3f(0.2, 0.9, 0.0);
            break;
          case T_MOUNTAINS:
            glColor3f(0.4, 0.8, 0.0);
            break;
          case T_HOLE:
            glColor3f(0.0, 0.8, 0.0);
            break;
          case T_BUILDING:
            glColor3f(0.0, 0.0, 0.0);
            break;
          case T_SHIP:
            glColor3f(1.0f, 1.0f, 1.0f);
            break;
          case T_ROBOT:
            glColor3f(0.0, 0.0, 1.0f);
            break;
          case T_EROBOT:
            glColor3f(1.0f, 0.0, 0.0);
            break;
          }
          glBegin(GL_QUADS);
          glVertex3f(30 + y, maxx - (x + 1), 0);
          glVertex3f(30 + y + 1, maxx - (x + 1), 0);
          glVertex3f(30 + y + 1, maxx - x, 0);
          glVertex3f(30 + y, maxx - x, 0);
          glEnd();
        }
      }
    }

    /* Draw the SHIP: */
    int x = (int)(nether->ship->pos.x * 2 - startx);
    int y = (int)(nether->ship->pos.y * 2 - starty);
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
    glVertex3f(30 + y, maxx - (x + 2), 2);
    glVertex3f(30 + y + 2, maxx - (x + 2), 2);
    glVertex3f(30 + y + 2, maxx - x, 2);
    glVertex3f(30 + y, maxx - x, 2);
    glEnd();
  }
  needsRedraw--;
  if (needsRedraw < 0) needsRedraw = 3;
}
