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


Shadow3DObject::Shadow3DObject() : C3DObject(), shdw_nfaces(0), shdw_faces(0)
{}


Shadow3DObject::Shadow3DObject(const char* file, const char* texturedir) :
  C3DObject(file,texturedir), shdw_nfaces(0), shdw_faces(0)
{}


Shadow3DObject::~Shadow3DObject()
{
  delete shdw_faces;
  shdw_faces = 0;
}


void Shadow3DObject::DrawShadow(const Color& color)
{
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, shdw_puntos.data());
  glColor4f(color.red, color.green, color.blue, color.alpha);
  glNormal3f(0,1,0);

  if (color.alpha != 1) {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
  }
  glBegin(GL_TRIANGLES);
  for(int i = 0; i < shdw_nfaces; i++) {
    glArrayElement(shdw_faces[i * 3]);
    glArrayElement(shdw_faces[i * 3 + 1]);
    glArrayElement(shdw_faces[i * 3 + 2]);
  }
  glEnd();

  if (color.alpha != 1) glDisable(GL_BLEND);
}


void Shadow3DObject::ComputeShadow(Vector light)
{
  int *pry_faces;
  float plane[4] = {0, 0, 1, 0};
  float l[3] = {float(light.x), float(light.y), float(light.z)};
  float n[3], v[3], w[3];
  float p[3];
  float tmp[3];

  for (Vector point: points) {
    p[0] = point.x;
    p[1] = point.y;
    p[2] = point.z;
    PlaneLineCollision(plane, p, l, tmp);
    shdw_puntos.emplace_back(tmp[0], tmp[1], 0);
  }

  /* Crear los triángulos proyectadas: */
  shdw_nfaces=0;
  // pry_nfaces=nfaces;
  pry_faces=new int[nfaces * 3];
  for (int i = 0; i < nfaces; i++) {
    /* Comprobar que el triángulo es visible: */

    v[0] = points[faces[i * 3 + 1]].x - points[faces[i * 3]].x;;
    v[1] = points[faces[i * 3 + 1]].y - points[faces[i * 3]].y;
    v[2] = points[faces[i * 3 + 1]].z - points[faces[i * 3]].z;
    w[0] = points[faces[i * 3 + 2]].x - points[faces[i * 3 + 1]].x;
    w[1] = points[faces[i * 3 + 2]].y - points[faces[i * 3 + 1]].y;
    w[2] = points[faces[i * 3 + 2]].z - points[faces[i * 3 + 1]].z;
    Normalf(v,w,n);

    float value = n[0] * l[0] + n[1] * l[1] + n[2] * l[2];
    if (value > 0) {
      pry_faces[shdw_nfaces * 3] = faces[i * 3];
      pry_faces[shdw_nfaces * 3 + 1] = faces[i * 3 + 1];
      pry_faces[shdw_nfaces * 3 + 2] = faces[i * 3 + 2];
      shdw_nfaces++;
    }
  }

  shdw_faces = new int[shdw_nfaces * 3];
  for (int i = 0; i < shdw_nfaces * 3; i++) shdw_faces[i] = pry_faces[i];
  delete pry_faces;
  pry_faces = 0;
  shdw_cmc.set(shdw_puntos);
}
