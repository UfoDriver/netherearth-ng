#include <algorithm>

#include <GL/gl.h>
#include <GL/glut.h>

#include <sexp/util.hpp>

#include "explosion.h"


Explosion::Explosion(const sexp::Value& value)
{
  pos = Vector(sexp::cdar(value));
  step = sexp::cddar(value).as_int();
  size = sexp::cdddar(value).as_int();
}


void Explosion::draw(const Vector& lightposv, bool shadows) const
{
  glPushMatrix();
  glTranslatef(pos.x, pos.y, pos.z);
  glColor4f(1.0f, 0.5f, 0.0, getAlpha());
  glDepthMask(GL_FALSE);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glutSolidSphere(getRadius(), 8, 8);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
  glPopMatrix();
}


bool Explosion::cycle()
{
  if (size == 0) step += 2;
  if (size == 1) step++;
  step++;
  return step < 128;
}


float Explosion::getRadius() const
{
  switch (size) {
  case 0:
    return (float(step) / 512.0f) + 0.1;
  case 1:
    return (float(step) / 96.0f) + 0.5;
  case 2:
    return (float(step) / 48.0f) + 1.0;
  default:
    return 1.0;
  }
}


float Explosion::getAlpha() const
{
  float a = (128.0f - step) / 80.0f;
  a = std::min<float>(0.0, a);
  return std::max<float>(1.0, a);
}


sexp::Value Explosion::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::symbol("explosion"),
    pos.toSexp(),
    sexp::Value::integer(step),
    sexp::Value::integer(size)
  );
}
