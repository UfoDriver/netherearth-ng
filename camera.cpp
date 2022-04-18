#include <algorithm>

#include <GL/gl.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <sexp/util.hpp>

#include "camera.h"


void Camera::cycle(unsigned char* keyboard)
{
  if (keyboard[SDLK_PAGEUP]) {
    zoom = std::min(zoom * 1.1, 4.0);
  }
  if (keyboard[SDLK_PAGEDOWN]) {
    zoom = std::max(zoom / 1.1, 0.5);
  }
}


void Camera::drawViewport()
{
  //         factors ship at top viewport.z max ship height = 5
  // z 0.5 = 4/5     4/10      3.5
  // z 0.75                    2.75
  // z 1   = 7/10    7/13      2
  // z 1.25                    1.25
  // z 1.5 = 10/15   10/15     0.5
  // z 1.75                    0
  // z 2   = 13/19   13/19     0
  // z 2.5 = 16/24   16/24     0
  // z 3   = 19/29   19/29     0
  // z 3.5 = 22/34   22/34     0
  // z 4   = 25/37   25/37     0

  // glPushMatrix();
  // glColor4f(1, 0, 0, .5);
  // glTranslatef(viewport.x, viewport.y, 0);

  // glBegin(GL_QUADS);
  // glVertex3f(-minx, -miny, 0.1);
  // glVertex3f(maxx, -miny, 0.1);
  // glVertex3f(maxx, maxy, 0.1);
  // glVertex3f(-minx, maxy, 0.1);
  // glEnd();

  // glColor4f(0, 0, 1, .5);
  // glRotatef(180.0, 1.0, 0.0, 0.0);
  // glTranslatef(0.0, 0.0, -1);
  // glutWireCone(0.3, 1, 20, 20);

  // glPopMatrix();
}


bool Camera::canSee(const Vector& pos) const
{
  return
    pos.y >= (viewport.y - miny) &&
    pos.y <= (viewport.y + maxy) &&
    pos.x >= (viewport.x - minx) &&
    pos.x <= (viewport.x + maxx);
}


void Camera::updateViewportForShip(const Vector& pos, int mapWidth, int mapHeight)
{
  viewport.x = pos.x + 0.5;
  viewport.y = pos.y + 0.5;
  viewport.z = std::max(0.0f, pos.z - 3 * zoom);

  if (viewport.x < 3 * zoom) {
    viewport.x = 3 * zoom;
    if (viewport.x > mapWidth - 3 * zoom)
      viewport.x = mapWidth / 2;
  } else {
    if (viewport.x > mapHeight - 3 * zoom) {
      viewport.x = mapWidth - 3 * zoom;
      if (viewport.x < 3 * zoom)
        viewport.x = mapWidth / 2;
    }
  }

  if (viewport.y < 3 * zoom) {
    viewport.y = 3 * zoom;
    if (viewport.y > mapHeight - 3 * zoom)
      viewport.y = mapHeight / 2;
  } else {
    if (viewport.y > mapHeight - 3 * zoom) {
      viewport.y = mapHeight - 3 * zoom;
      if (viewport.y < 3 * zoom)
        viewport.y = mapHeight / 2;
    }
  }

  miny = 8 * zoom;
  maxx = 8 * zoom;
  minx = (10 + viewport.z * 4) * zoom;
  maxy = (9 + viewport.z * 4) * zoom;
}


void Camera::lookAt(float offset) const
{
  gluLookAt(viewport.x + x * zoom + offset,
            viewport.y + y * zoom + offset,
            viewport.z + z * zoom,
            viewport.x + offset,
            viewport.y + offset,
            viewport.z,
            0, 0, 1);
}


sexp::Value Camera::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("camera"),
    Vector::toSexp(),
    viewport.toSexp()
  );
}


bool Camera::fromSexp(const sexp::Value& value)
{
  Vector::fromSexp(sexp::cdar(value));
  viewport.fromSexp(sexp::cddar(value));
  return true;
}
