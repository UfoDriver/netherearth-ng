#ifdef _WIN32
#include "windows.h"
#endif

#include <GL/gl.h>
#include <cmath>

#include "3dobject.h"
#include "cmc.h"
#include "myglutaux.h"
#include "piece3dobject.h"
#include "quaternion.h"
#include "vector.h"


Piece3DObject::Piece3DObject() : C3DObject()
{
}


Piece3DObject::Piece3DObject(const std::string& file, const std::string& texturedir) :
  C3DObject(file, texturedir)
{
}


Piece3DObject::~Piece3DObject()
{
}


void Piece3DObject::drawShadow(int angle, Vector light, const Color& color)
{
  glEnableClientState(GL_VERTEX_ARRAY);
  glColor4f(color.red, color.green, color.blue, color.alpha);
  glNormal3f(0, 1, 0);

  angle = angle % 360;

  if (color.alpha != 1){
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  }

  switch (angle) {
  case 0:
    glVertexPointer(3, GL_FLOAT, 0, shadowPoints0.data());
    glBegin(GL_TRIANGLES);
    for (const Face& face: shadowFaces0) {
      glArrayElement(face.a);
      glArrayElement(face.b);
      glArrayElement(face.c);
    }
    glEnd();
    break;
  case 90:
    glVertexPointer(3, GL_FLOAT, 0, shadowPoints90.data());
    glBegin(GL_TRIANGLES);
    for (const Face& face: shadowFaces90) {
      glArrayElement(face.a);
      glArrayElement(face.b);
      glArrayElement(face.c);
    }
    glEnd();
    break;
  case 180:
    glVertexPointer(3, GL_FLOAT, 0, shadowPoints180.data());
    glBegin(GL_TRIANGLES);
    for (const Face& face: shadowFaces180) {
      glArrayElement(face.a);
      glArrayElement(face.b);
      glArrayElement(face.c);
    }
    glEnd();
    break;
  case 270:
    glVertexPointer(3, GL_FLOAT, 0, shadowPoints270.data());
    glBegin(GL_TRIANGLES);
    for (const Face& face: shadowFaces270) {
      glArrayElement(face.a);
      glArrayElement(face.b);
      glArrayElement(face.c);
    }
    glEnd();
    break;
  default:
    computeDynamicShadow(angle, light);
    glVertexPointer(3, GL_FLOAT, 0, shadowPointsDynamic.data());
    glBegin(GL_TRIANGLES);
    for (const Face& face: shadowFacesDynamic) {
      glArrayElement(face.a);
      glArrayElement(face.b);
      glArrayElement(face.c);
    }
    glEnd();
  }

  if (color.alpha != 1) glDisable(GL_BLEND);
}


void Piece3DObject::computeShadow(int angle, const Vector& light, std::vector<Vector>& p,
                                  std::vector<Face>& shadowFaces, CMC &cmc)
{
  float l[4];

  angle = angle % 360;

  {
    float lv[4] = {float(light.x), float(light.y), float(light.z), 1};
    float axis[4] = {0, 0, 1, 1};
    float m[16] = {1, 0, 0, 0,
                   0, 1, 0, 0,
                   0, 0, 1, 0,
                   0, 0, 0, 1};
    Quaternion q2;

    q2.from_axis_angle(axis, float((-angle * M_PI) / 180));
    q2.to_matrix(m);
    ApplyMatrix(lv, m, l);
  }
  p.reserve(points.size());
  {
    float ppp[3], tmp[3];
    for(const Vector& point: points) {
      ppp[0] = point.x;
      ppp[1] = point.y;
      ppp[2] = point.z;
      float plane[4] {0, 0, 1, 0};
      PlaneLineCollision(plane, ppp, l, tmp);
      p.emplace_back(tmp[0], tmp[1], 0);
    }
  }

  /* Crear los triángulos proyectadas: */
  shadowFaces.clear();

  for (const Face& face: faces) {
    /* Comprobar que el triángulo es visible: */
    float v[3];
    float w[3];
    float n[3];

    v[0] = points[face.b].x - points[face.a].x;
    v[1] = points[face.b].y - points[face.a].y;
    v[2] = points[face.b].z - points[face.a].z;
    w[0] = points[face.c].x - points[face.b].x;
    w[1] = points[face.c].y - points[face.b].y;
    w[2] = points[face.c].z - points[face.b].z;
    Normalf(v, w, n);

    float value = n[0] * l[0] + n[1] * l[1] + n[2] * l[2];
    if (value > 0) {
      shadowFaces.push_back(face);
    }
  }

  cmc.set(p);
}


void Piece3DObject::computeDynamicShadow(int angle, const Vector& light)
{
  // @TODO: seems like it's broken
  computeShadow(angle, light, shadowPointsDynamic, shadowFacesDynamic, shadowCMCDynamic);
}


void Piece3DObject::computeFixedShadows(Vector light)
{
  computeShadow(0, light, shadowPoints0, shadowFaces0, shadowCMC0);
  computeShadow(90, light, shadowPoints90, shadowFaces90, shadowCMC90);
  computeShadow(180, light, shadowPoints180, shadowFaces180, shadowCMC180);
  computeShadow(270, light, shadowPoints270, shadowFaces270, shadowCMC270);
}
