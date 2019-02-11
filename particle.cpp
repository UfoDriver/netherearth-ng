#include "particle.h"

#include "GL/gl.h"


Particle::Particle():
  size1(0), size2(0), a1(0), a2(0),
  lifetime(0), acttime(0)
{
}


Particle::Particle(Vector p, Vector spd1, Vector spd2, float sz1, float sz2, const Color& color,
                   float a1p, float a2p, int lt):
  pos(p), speed1(spd1), speed2(spd2), size1(sz1), size2(sz2), color(color), a1(a1p), a2(a2p),
  lifetime(lt), acttime(0)
{
}


void Particle::draw()
{
  float val2 = float(acttime) / float(lifetime);
  float val = 1 - val2;
  float sz = val * size1 + val2 * size2;

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();

  glTranslatef(pos.x, pos.y, pos.z);
  glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glNormal3f(0, 0, 1);

  glBegin(GL_TRIANGLES);
  glColor4f(color.red, color.green, color.blue, val * a1 + val2* a2);
  glVertex3f(0, 0, 0);
  glColor4f(color.red, color.green, color.blue, 0);
  glVertex3f(sz, 0, 0);
  glVertex3f(0, sz, 0);

  glColor4f(color.red, color.green, color.blue, val * a1+val2 * a2);
  glVertex3f(0, 0, 0);
  glColor4f(color.red, color.green, color.blue, 0);
  glVertex3f(0, sz, 0);
  glVertex3f(-sz, 0, 0);

  glColor4f(color.red, color.green, color.blue, val * a1 + val2 * a2);
  glVertex3f(0, 0, 0);
  glColor4f(color.red, color.green, color.blue, 0);
  glVertex3f(-sz, 0, 0);
  glVertex3f(0, -sz, 0);

  glColor4f(color.red, color.green, color.blue, val * a1 + val2 * a2);
  glVertex3f(0, 0, 0);
  glColor4f(color.red, color.green, color.blue, 0);
  glVertex3f(0,- sz, 0);
  glVertex3f(sz, 0, 0);

  glEnd();

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);

  glPopMatrix();
}


bool Particle::cycle()
{
  float val2 = float(acttime) / float(lifetime);
  float val = 1 - val2;
  pos = pos + (speed1 * val + speed2 * val2);
  acttime++;
  return acttime < lifetime;
}
