#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>


class Vector {
public:
  Vector() = default;
  Vector(double x, double y, double z) : x {float(x)}, y {float(y)}, z {float(z)} {};
  Vector(const Vector &v) : x {v.x}, y {v.y}, z {v.z} {};
  //  Vector(Vector &&o) : x(std::move(o.x)), y(std::move(o.y)), z(std::move(o.z)) {};

  Vector& operator=(const Vector& o)
  {
    x = o.x;
    y = o.y;
    z = o.z;
    return *this;
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

  bool compare2D(const Vector &v) const;
  bool zero();

  double norma() const;
  double normalize();
  Vector normal(const Vector &v) const;

  bool aboutToCollide2D(const Vector &vector, float threshold) const;
  bool aboutToCollide3D(const Vector &vector, float threshold) const;

  float x {0};
  float y {0};
  float z {0};
};

std::ostream& operator<<(std::ostream& out, const Vector& vector);
std::istream& operator>>(std::istream& in, Vector& vector);

#endif // VECTOR_H
