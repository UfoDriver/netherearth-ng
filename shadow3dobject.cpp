#ifdef _WIN32
#include "windows.h"
#endif

#include "stdio.h"

#include "GL/gl.h"
#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"

#include "myglutaux.h"


Shadow3DObject::Shadow3DObject() : C3DObject()
{}


Shadow3DObject::Shadow3DObject(const std::string& file, const std::string& texturedir) :
  C3DObject(file,texturedir)
{}


Shadow3DObject::~Shadow3DObject()
{
}


void Shadow3DObject::drawShadow(const Color& color)
{
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, shadowPoints.data());
  glColor4f(color.red, color.green, color.blue, color.alpha);
  glNormal3f(0,1,0);

  if (color.alpha != 1) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  }
  glBegin(GL_TRIANGLES);
  for (const Face& face: shadowFaces) {
    glArrayElement(face.a);
    glArrayElement(face.b);
    glArrayElement(face.c);
  }
  glEnd();

  if (color.alpha != 1) glDisable(GL_BLEND);
}


void Shadow3DObject::computeShadow(Vector light)
{
  float l[3] = {float(light.x), float(light.y), float(light.z)};

  for (Vector point: points) {
    float plane[4] = {0, 0, 1, 0};
    float p[3];
    float tmp[3];
    p[0] = point.x;
    p[1] = point.y;
    p[2] = point.z;
    PlaneLineCollision(plane, p, l, tmp);
    shadowPoints.emplace_back(tmp[0], tmp[1], 0);
  }

  shadowFaces.clear();
  for (const Face& face: faces) {
    float n[3], v[3], w[3];
    v[0] = points[face.b].x - points[face.a].x;;
    v[1] = points[face.b].y - points[face.a].y;
    v[2] = points[face.b].z - points[face.a].z;
    w[0] = points[face.c].x - points[face.b].x;
    w[1] = points[face.c].y - points[face.b].y;
    w[2] = points[face.c].z - points[face.b].z;
    Normalf(v,w,n);

    float value = n[0] * l[0] + n[1] * l[1] + n[2] * l[2];
    if (value > 0) {
      shadowFaces.emplace_back(face);
    }
  }

  shadowCMC.set(shadowPoints);
}
