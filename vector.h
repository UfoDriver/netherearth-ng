#ifndef __AW_VECTOR
#define __AW_VECTOR

#include <iostream>
#include <stdio.h>

class Vector {
public:
  Vector(): x(0), y(0), z() {};
  Vector(double nx, double ny, double nz): x(nx), y(ny), z(nz) {};
  Vector(const Vector &v): x(v.x), y(v.y), z(v.z) {};
  explicit Vector(FILE *fp): x(0), y(0), z(0) {
    load(fp);
  }

  Vector operator+(const Vector &v) const;
  Vector operator-(const Vector &v) const;
  Vector operator-() const;

  Vector operator^(const Vector &v) const;
  double operator*(const Vector &v) const;
  Vector operator*(double ctnt);

  Vector operator/(double ctnt);

  bool operator==(const Vector &v);
  bool operator!=(const Vector &v);

  bool zero();

  double norma() const;
  double normalize();
  Vector normal(const Vector& v) const;

  bool load(FILE *fp);
  bool save(FILE *fp);

  float x, y, z;
};

std::ostream& operator<<(std::ostream& out, const Vector& vector);
std::istream& operator>>(std::istream& in, Vector& vector);

#endif
