#ifndef __BRAIN_CMC
#define __BRAIN_CMC

#include <fstream>
#include <vector>

#include <sexp/value.hpp>

class Color;
class Vector;


class CMC {
public:
  CMC() {};
  CMC(float* p, int np);
  CMC(float* x, float* y, float* z, int np);

  void reset(void);

  void draw(const Color& color) const;
  void drawabsolute(const Color& color) const;

  void set(const std::vector<Vector>& p);
  void set(float* x, float* y, float* z, int np);

  void expand(const CMC& anotherObject, float* matrix);

  bool collision(const CMC& other) const;
  bool collision(float* m, const CMC& other, float* m2) const;
  bool collision_simple(float* m, const CMC& other, float* m2) const;

  sexp::Value toSexp() const;

  float x[2] {0, 0}, y[2] {0, 0}, z[2] {0, 0};
};

std::ostream& operator<<(std::ostream& out, const CMC& cmc);
std::istream& operator>>(std::istream& in, CMC& cmc);

#endif
