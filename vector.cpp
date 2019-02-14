#include <iomanip>

#include "math.h"
#include "stdio.h"
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


bool Vector::operator==(const Vector &v)
{
  return x == v.x && y == v.y && z == v.z;
}


bool Vector::operator!=(const Vector &v)
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


bool Vector::load(FILE *fp)
{
  float t1, t2, t3;

  if (3 != fscanf(fp, "%f %f %f", &t1, &t2, &t3)) return false;
  x = t1;
  y = t2;
  z = t3;
  return true;
}


bool Vector::save(FILE *fp)
{
  fprintf(fp, "%.8f %.8f %.8f\n", float(x), float(y), float(z));
  return true;
}


std::ostream& operator<<(std::ostream& out, const Vector& vector)
{
  out << std::setw(8) << float(vector.x)
      << std::setw(8) << float(vector.y)
      << std::setw(8) << float(vector.z)
      << '\n';
  return out;
}

std::istream& operator>>(std::istream& in, Vector& vector)
{
  return in >> vector.x >> vector.y >> vector.z;
}
