#ifndef __AW_VECTOR
#define __AW_VECTOR

#include <iostream>


class Vector {
public:
  Vector() : x {0}, y {0}, z {0} {};
  Vector(double x, double y, double z) : x(x), y(y), z(z) {};
  Vector(const Vector &v) : x{v.x}, y{v.y}, z {v.z} {};

  Vector operator+(const Vector &v) const;
  Vector operator-(const Vector &v) const;
  Vector operator-() const;

  Vector operator^(const Vector &v) const;
  double operator*(const Vector &v) const;
  Vector operator*(double ctnt);

  Vector operator/(double ctnt);

  bool operator==(const Vector &v);
  bool operator!=(const Vector &v);

  bool compare2D(const Vector &v) const;
  bool zero();

  double norma() const;
  double normalize();
  Vector normal(const Vector &v) const;

  bool aboutToCollide2D(const Vector &vector, float threshold) const;
  bool aboutToCollide3D(const Vector &vector, float threshold) const;

  float x, y, z;
};

std::ostream& operator<<(std::ostream& out, const Vector& vector);
std::istream& operator>>(std::istream& in, Vector& vector);

#endif
