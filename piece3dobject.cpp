#ifdef _WIN32
#include "windows.h"
#endif

#include "stdio.h"

#include "GL/gl.h"
#include "vector.h"
#include "quaternion.h"
#include "cmc.h"
#include "3dobject.h"
#include "piece3dobject.h"

#include "myglutaux.h"

Piece3DObject::Piece3DObject() : C3DObject(),
                                 shdw_npuntos_0(0),
                                 shdw_npuntos_90(0),
                                 shdw_npuntos_180(0),
                                 shdw_npuntos_270(0),
                                 shdw_npuntos_dynamic(0),
                                 shdw_nfaces_0(0),
                                 shdw_nfaces_90(0),
                                 shdw_nfaces_180(0),
                                 shdw_nfaces_270(0),
                                 shdw_nfaces_dynamic(0),
                                 shdw_puntos_0(0),
                                 shdw_puntos_90(0),
                                 shdw_puntos_180(0),
                                 shdw_puntos_270(0),
                                 shdw_puntos_dynamic(0),
                                 shdw_faces_0(0),
                                 shdw_faces_90(0),
                                 shdw_faces_180(0),
                                 shdw_faces_270(0),
                                 shdw_faces_dynamic(0)
{
}


Piece3DObject::Piece3DObject(const std::string& file, const std::string& texturedir) :
  C3DObject(file, texturedir),
  shdw_npuntos_0(0),
  shdw_npuntos_90(0),
  shdw_npuntos_180(0),
  shdw_npuntos_270(0),
  shdw_npuntos_dynamic(0),
  shdw_nfaces_0(0),
  shdw_nfaces_90(0),
  shdw_nfaces_180(0),
  shdw_nfaces_270(0),
  shdw_nfaces_dynamic(0),
  shdw_puntos_0(0),
  shdw_puntos_90(0),
  shdw_puntos_180(0),
  shdw_puntos_270(0),
  shdw_puntos_dynamic(0),
  shdw_faces_0(0),
  shdw_faces_90(0),
  shdw_faces_180(0),
  shdw_faces_270(0),
  shdw_faces_dynamic(0)
{
}


Piece3DObject::~Piece3DObject()
{
  delete shdw_puntos_0;
  delete shdw_faces_0;
  shdw_puntos_0=0;
  shdw_faces_0=0;
  delete shdw_puntos_90;
  delete shdw_faces_90;
  shdw_puntos_90=0;
  shdw_faces_90=0;
  delete shdw_puntos_180;
  delete shdw_faces_180;
  shdw_puntos_180=0;
  shdw_faces_180=0;
  delete shdw_puntos_270;
  delete shdw_faces_270;
  shdw_puntos_270=0;
  shdw_faces_270=0;
  delete shdw_puntos_dynamic;
  delete shdw_faces_dynamic;
  shdw_puntos_dynamic=0;
  shdw_faces_dynamic=0;
}


void Piece3DObject::DrawShadow(int angle,Vector light,float r,float g,float b,float a)
{
  // @TODO: fix shadows draw
  return;
	/* Dibuja el objeto: */ 
	glEnableClientState(GL_VERTEX_ARRAY);
	glColor4f(r,g,b,a);
	glNormal3f(0,1,0);

	angle=angle%360;

	if (a!=1){
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_BLEND);
	} /* if */ 

	switch(angle) {
	case 0:
		glVertexPointer(3,GL_FLOAT,0,shdw_puntos_0);
		glBegin(GL_TRIANGLES);
		for(int i = 0; i < shdw_nfaces_0; i++) {
			glArrayElement(shdw_faces_0[i*3]);
			glArrayElement(shdw_faces_0[i*3+1]);
			glArrayElement(shdw_faces_0[i*3+2]);
		} /* for */ 
		glEnd();
		break;
	case 90:
		glVertexPointer(3,GL_FLOAT,0,shdw_puntos_90);
		glBegin(GL_TRIANGLES);
		for(int i = 0; i < shdw_nfaces_90; i++) {
			glArrayElement(shdw_faces_90[i*3]);
			glArrayElement(shdw_faces_90[i*3+1]);
			glArrayElement(shdw_faces_90[i*3+2]);
		} /* for */ 
		glEnd();
		break;
	case 180:
		glVertexPointer(3,GL_FLOAT,0,shdw_puntos_180);
		glBegin(GL_TRIANGLES);
		for(int i = 0; i < shdw_nfaces_180; i++) {
			glArrayElement(shdw_faces_180[i*3]);
			glArrayElement(shdw_faces_180[i*3+1]);
			glArrayElement(shdw_faces_180[i*3+2]);
		} /* for */ 
		glEnd();
		break;
	case 270:
		glVertexPointer(3,GL_FLOAT,0,shdw_puntos_270);
		glBegin(GL_TRIANGLES);
		for(int i = 0; i < shdw_nfaces_270; i++) {
			glArrayElement(shdw_faces_270[i*3]);
			glArrayElement(shdw_faces_270[i*3+1]);
			glArrayElement(shdw_faces_270[i*3+2]);
		} /* for */ 
		glEnd();
		break;
	default:
		ComputeDynamicShadow(angle,light);

		glVertexPointer(3,GL_FLOAT,0,shdw_puntos_dynamic);
		glBegin(GL_TRIANGLES);
		for(int i = 0; i < shdw_nfaces_dynamic; i++) {
			glArrayElement(shdw_faces_dynamic[i*3]);
			glArrayElement(shdw_faces_dynamic[i*3+1]);
			glArrayElement(shdw_faces_dynamic[i*3+2]);
		} /* for */ 
		glEnd();
		break;
	} /* switch */ 

	if (a!=1) glDisable(GL_BLEND);
} /* Piece3DObject::DraqwShadow */ 


void Piece3DObject::ComputeShadow(int angle, Vector light, int *np, int *nc, const Vector *p,
                                  int **c, CMC *cmc)
{
  int i;
  int pry_npuntos;
  Vector* pry_puntos;
  int *pry_faces,pry_nfaces;
  float plane[4];
  float l[4];
  float v[3],w[3],n[3],value;
  int shdw_nfaces;

  angle=angle%360;

  plane[0]=0;
  plane[1]=0;
  plane[2]=1;
  plane[3]=0;


  /* Rotar la luz: */ 
  {
    float lv[4]={float(light.x),float(light.y),float(light.z),1};
    float axis[4]={0,0,1,1};
    float m[16]={1,0,0,0,
                 0,1,0,0,
                 0,0,1,0,
                 0,0,0,1};
    Quaternion q2;

    q2.from_axis_angle(axis,float((-angle*3.14592f)/180));
    q2.to_matrix(m);

    ApplyMatrix(lv,m,l);
  }

  pry_npuntos=npoints;
  pry_puntos = new Vector[npoints];
  /* Proyectar TODOS los triángulos sobre el plano Z: */ 
  {
    float p[3],tmp[3];

    for(int i = 0; i < npoints; i++) {
      p[0] = points[i].x;
      p[1] = points[i].y;
      p[2] = points[i].z;
      PlaneLineCollision(plane,p,l,tmp);

      pry_puntos[i].x = tmp[0];
      pry_puntos[i].y = tmp[1];
      pry_puntos[i].z = 0;
    } /* for */ 
  }

  /* Crear los triángulos proyectadas: */ 
  shdw_nfaces=0;
  pry_nfaces=nfaces;
  pry_faces=new int[nfaces*3];
  for(int i = 0; i < nfaces; i++) {
    /* Comprobar que el triángulo es visible: */ 

    v[0]=points[faces[i*3+1]].x - points[faces[i*3]].x;
    v[1]=points[faces[i*3+1]].y - points[faces[i*3]].y;
    v[2]=points[faces[i*3+1]].z - points[faces[i*3]].z;
    w[0]=points[faces[i*3+2]].x - points[faces[i*3+1]].x;
    w[1]=points[faces[i*3+2]].y - points[faces[i*3+1]].y;
    w[2]=points[faces[i*3+2]].z - points[faces[i*3+1]].z;
    Normalf(v,w,n);

    value=n[0]*l[0]+n[1]*l[1]+n[2]*l[2];
    if (value>0) {
      pry_faces[shdw_nfaces*3]=faces[i*3];
      pry_faces[shdw_nfaces*3+1]=faces[i*3+1];
      pry_faces[shdw_nfaces*3+2]=faces[i*3+2];
      shdw_nfaces++;
    } /* if */ 
  } /* for */ 


  /* Unir los triángulos proyectados: */ 
  /* ... */ 

  /* Copiarlos a las variables del objeto: */ 
  delete p;
  delete *c;
  p=0;
  *c=0;
//	shdw_puntos_dynamic=0;
//	shdw_faces_dynamic=0;

  *c=new int[shdw_nfaces*3];
  for(i=0;i<shdw_nfaces*3;i++) (*c)[i]=pry_faces[i];
  delete pry_faces;
  pry_faces=0;
  *nc=shdw_nfaces;
  *np=pry_npuntos;
  p=pry_puntos;
  cmc->set(p,*np);
}


void Piece3DObject::ComputeDynamicShadow(int angle,Vector light)
{
  ComputeShadow(angle,light,&shdw_npuntos_dynamic,&shdw_nfaces_dynamic,
                shdw_puntos_dynamic,&shdw_faces_dynamic,&shdw_cmc_dynamic);
}


void Piece3DObject::ComputeFixedShadows(Vector light)
{
  ComputeShadow(0,light,&shdw_npuntos_0,&shdw_nfaces_0,
                shdw_puntos_0,&shdw_faces_0,&shdw_cmc_0);
  ComputeShadow(90,light,&shdw_npuntos_90,&shdw_nfaces_90,
                shdw_puntos_90,&shdw_faces_90,&shdw_cmc_90);
  ComputeShadow(180,light,&shdw_npuntos_180,&shdw_nfaces_180,
                shdw_puntos_180,&shdw_faces_180,&shdw_cmc_180);
  ComputeShadow(270,light,&shdw_npuntos_270,&shdw_nfaces_270,
                shdw_puntos_270,&shdw_faces_270,&shdw_cmc_270);
}
