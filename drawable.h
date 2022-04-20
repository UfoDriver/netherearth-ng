#ifndef DRAWABLE_H
#define DRAWABLE_H

#include "vector.h"


class Drawable
{
public:
  virtual void draw(const Vector& lightposv, bool shadows) const = 0;
};


#endif // DRAWABLE_H
