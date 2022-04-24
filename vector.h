#ifndef VECTOR_H
#define VECTOR_H

#include <iostream>

#include <sexp/util.hpp>
#include <sexp/value.hpp>


class Vector {
public:
  Vector() = default;
  Vector(double x, double y, double z) : x {float(x)}, y {float(y)}, z {float(z)} {};
  Vector(const Vector &v) : x {v.x}, y {v.y}, z {v.z} {};
  Vector(const sexp::Value& value) : x { sexp::car(value).as_float()},
                                     y { sexp::cdar(value).as_float()},
                                     z { sexp::cddar(value).as_float()} {};
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

  bool operator==(const Vector &v) const;
  bool operator!=(const Vector &v) const;

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

  sexp::Value toSexp() const;
  bool fromSexp(const sexp::Value&);
};

#endif // VECTOR_H
