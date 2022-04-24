#include <cmath>
#include <iomanip>

#include <sexp/util.hpp>
#include <sexp/value.hpp>

#include "vector.h"


Vector Vector::operator+(const Vector &v) const
{
  return Vector(x + v.x, y + v.y, z + v.z);
}


Vector Vector::operator-(const Vector &v) const
{
  return Vector(x - v.x, y - v.y, z - v.z);
}


Vector Vector::operator-() const
{
  return Vector(-x, -y, -z);
}


Vector Vector::operator^(const Vector &v) const
{
  return Vector(y * v.z - v.y * z,
                z * v.x - v.z * x,
                x * v.y - v.x * y);
}


double Vector::operator*(const Vector &v) const
{
  return x * v.x + y * v.y + z * v.z;
}


Vector Vector::operator*(double ctnt)
{
  x *= ctnt;
  y *= ctnt;
  z *= ctnt;
  return *this;
}


Vector Vector::operator/(double ctnt)
{
  x /= ctnt;
  y /= ctnt;
  z /= ctnt;
  return *this;
}


bool Vector::operator==(const Vector &v) const
{
  return x == v.x && y == v.y && z == v.z;
}


bool Vector::compare2D(const Vector& v) const
{
  return x == v.x && y == v.y;
}


bool Vector::operator!=(const Vector &v) const
{
  return x != v.x || y != v.y || z != v.z;
}

double Vector::norma() const
{
  return sqrt(x * x + y * y + z * z);
}


Vector Vector::normal(const Vector& v) const
{
  Vector res = *this ^ v;
  res.normalize();
  return res;
}


double Vector::normalize()
{
  double n = norma();

  if (n == 0) return 0;

  x /= n;
  y /= n;
  z /= n;

  return n;
}


bool Vector::zero()
{
  return x == 0 && y == 0 && z == 0;
}


bool Vector::aboutToCollide2D(const Vector& vector, float threshold) const
{
  return ((x - vector.x) * (x - vector.x) +
          (y - vector.y) * (y - vector.y) +
          (z - vector.z) * (z - vector.z)) < threshold;

}


bool Vector::aboutToCollide3D(const Vector& vector, float threshold) const
{
  return ((x - vector.x) * (x - vector.x) +
          (y - vector.y) * (y - vector.y)) < threshold;
}


sexp::Value Vector::toSexp() const
{
  return sexp::Value::list(
    sexp::Value::real(x),
    sexp::Value::real(y),
    sexp::Value::real(z)
  );
}


bool Vector::fromSexp(const sexp::Value& value)
{
  x = sexp::car(value).as_float();
  y = sexp::cdar(value).as_float();
  z = sexp::cddar(value).as_float();

  return true;
}
