#ifdef _WIN32
#include "windows.h"
#endif
#include "GL/gl.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "string.h"
#include "stdio.h"

#include "cmc.h"
#include "3dobject.h"

#include "vector.h"
#include "myglutaux.h"


extern void Normal (double vector1[3],double vector2[3],double resultado[3]);



C3DObject::C3DObject(): ncaras(0), points(0), normales(0), caras(0), r(0), g(0), b(0),
                        display_list(-1), tx(0), ty(0), textures(0)
{
}


C3DObject::C3DObject(const std::string& filename, const std::string& texturedir):
  npoints(0), ncaras(0), points(0), normales(0), caras(0), r(0), g(0), b(0), display_list(-1),
  tx(0), ty(0), textures(0)
{
  int l = filename.length();

  if (filename[l - 1] == 'c' || filename[l - 1] == 'C')
    loadASC(filename);
  else
    loadASE(filename, texturedir);
}

bool C3DObject::readVertex(const std::string& data)
{
  if (!data.compare(0, 12, "Vertex list:")) return false;

  std::istringstream inStr(data);
  int vertexNumber = 0;
  float x, y, z;
  // @TODO: fix me, puntos and indexing are ugly
  inStr.ignore(100, ' ');
  inStr >> vertexNumber;
  inStr.ignore(100, ':');
  inStr.ignore(100, ':');
  inStr >> x;
  inStr.ignore(100, ':');
  inStr >> y;
  inStr.ignore(100, ':');
  inStr >> z;
  points[vertexNumber] = Vector(x, y, z);
  // @TODO: skipped for a while
  // if (textures) {
  //   if (2==fscanf(fp,"%s %s",buffer3,buffer4)) {
  //     if (1==sscanf(buffer3+2,"%f",&x) &&
  //         1==sscanf(buffer4+2,"%f",&y)) {}
  //   }
  // }
  return true;
}


int C3DObject::readFace(const std::string& data, int* smooth) {
  std::istringstream inStr(data);
  int p1, p2, p3;
  int act_face;
  inStr.ignore(100, ' ');
  inStr >> act_face;
  inStr.ignore(100, ':');
  inStr.ignore(100, ':');
  inStr >> p1;
  inStr.ignore(100, ':');
  inStr >> p2;
  inStr.ignore(100, ':');
  inStr >> p3;
  inStr.ignore(100, '\n');

  caras[act_face * 3] = p1;
  caras[act_face * 3 + 1] = p2;
  caras[act_face * 3 + 2] = p3;
  smooth[act_face] = 0;
  return act_face;
}


bool C3DObject::readSmoothing(const std::string& data, int* smooth, int currentFace) {
  std::istringstream inStr(data);
  inStr.ignore(100, ':');
  inStr >> smooth[currentFace];
  return true;
}


bool C3DObject::loadASC(const std::string& filename)
{
  const int ST_INIT = 0;
  const int ST_DATA = 1;

  int *smooth;
  int state = ST_INIT;
  int act_face;

  std::ifstream inFile(filename);
  if (!inFile.is_open()) return false;


  while (!inFile.eof()) {
    std::string buffer;
    std::getline(inFile, buffer);

    switch(state) {
      case ST_INIT:
        if (buffer.find("Vertices:") != std::string::npos) {
          std::istringstream iStr(buffer);
          iStr.seekg(buffer.find("Vertices:") + 9);
          iStr >> npoints;
          points = new Vector[npoints];
          if (ncaras != 0) state = ST_DATA;
        }
        if (buffer.find("Faces:") != std::string::npos) {
          std::istringstream iStr(buffer);
          iStr.seekg(buffer.find("Faces:") + 6);
          iStr >> ncaras;
          caras = new int[ncaras * 3];
          smooth = new int[ncaras];
          r = new float[ncaras];
          g = new float[ncaras];
          b = new float[ncaras];
          for(int i = 0; i < ncaras * 3; i++) caras[i] = 0;
          for(int i = 0;i < ncaras; i++) {
            r[i] = 0.5;
            g[i] = 0.5;
            b[i] = 0.5;
          }
          if (npoints != 0) state = ST_DATA;
        }
        break;
      case ST_DATA:
        if (!buffer.compare(0, 6, "Mapped")) {}
        if (!buffer.compare(0, 6, "Vertex")) { readVertex(buffer); }
        if (!buffer.compare(0, 4, "Face")) { act_face = readFace(buffer, smooth); }
        if (!buffer.compare(0, 9, "Smoothing")) { readSmoothing(buffer, smooth, act_face); }

        if (buffer == "Material:") {
          // Not tested, we don't have models with material embedded
          std::string buffer2;
          inFile >> buffer2; // Should get quoted filename base
          // buffer2.strip('"');
          buffer2.append(".bmp");
        }

        break;
    }
  }

  if (state == ST_INIT) {
    if (points != NULL) delete[] points;
    if (caras != NULL) delete[] caras;
    npoints = 0;
    ncaras = 0;
    points = NULL;
    caras = NULL;
    return false;
  }

  CalculaNormales(smooth);
  cmc.set(points, npoints);
  delete[] smooth;

  return true;
}


float C3DObject::normalize(void)
{
	return normalize(1.0);
} /* normalize */ 


float C3DObject::normalize(float c)
{
	int i;
	float cx,fx,cy,fy,cz,fz,factor;

	cmc.set(points, npoints);
	fx=(cmc.x[1]-cmc.x[0])/2;
	fy=(cmc.y[1]-cmc.y[0])/2;
	fz=(cmc.z[1]-cmc.z[0])/2;
	cx=(cmc.x[1]+cmc.x[0])/2;
	cy=(cmc.y[1]+cmc.y[0])/2;
	cz=(cmc.z[1]+cmc.z[0])/2;

	factor=fx;
	if (fy>factor) factor=fy;
	if (fz>factor) factor=fz;

	factor/=c;

    for(int i = 0; i < npoints; i++) {
      points[i].x = (points[i].x - cx) / factor;
      points[i].y = (points[i].y - cy) / factor;
      points[i].z = (points[i].z - cz) / factor;
    }
    cmc.set(points, npoints);

    return factor;
} /* normalize */ 


float C3DObject::normalizexy(float c)
{
	int i;
	float cx,fx,cy,fy,cz,factor;

	cmc.set(points, npoints);
	fx=(cmc.x[1]-cmc.x[0])/2;
	fy=(cmc.y[1]-cmc.y[0])/2;
	cx=(cmc.x[1]+cmc.x[0])/2;
	cy=(cmc.y[1]+cmc.y[0])/2;
	cz=(cmc.z[1]+cmc.z[0])/2;

	factor=fx;
	if (fy>factor) factor=fy;

	factor/=c;

    for(int i = 0; i < npoints; i++) {
      points[i].x = (points[i].x - cx) / factor;
      points[i].y = (points[i].y - cy) / factor;
      points[i].z = (points[i].z - cz) / factor;
    }
    cmc.set(points, npoints);

	return factor;
} /* normalizexy */ 


void C3DObject::makepositive(void)
{
  cmc.set(points, npoints);

  for (int i = 0; i < npoints; i++) {
    points[i].x -= cmc.x[0];
    points[i].y -= cmc.y[0];
    points[i].z -= cmc.z[0];
  }
  cmc.set(points, npoints);
}


void C3DObject::makepositivex(void)
{
  cmc.set(points, npoints);

  for (int i = 0; i < npoints; i++) {
    points[i].x -= cmc.x[0];
  }
  cmc.set(points, npoints);
}


void C3DObject::makepositivey(void)
{
  cmc.set(points, npoints);

  for (int i = 0; i < npoints; i++) {
    points[i].y -= cmc.y[0];
  }
  cmc.set(points, npoints);
}


void C3DObject::makepositivez(void)
{
  cmc.set(points, npoints);

  for (int i = 0; i < npoints; i++) {
    points[i].z -= cmc.z[0];
  }
  cmc.set(points, npoints);
}


void C3DObject::moveobject(const Vector& distance)
{
  for(int i = 0; i < npoints; i++) {
    points[i] = points[i] + distance;
  }
  cmc.set(points, npoints);
}


void C3DObject::CalculaNormales(int *smooth)
{

	/* Calcular las normales a las caras: */ 
	int j,k,act_vertex;
	double vector1[3],vector2[3],normal[3];
	int num;

	normales=new float[ncaras*3*3];

    Vector* normales_tmp = new Vector[ncaras];

	for (int i = 0; i < ncaras; i++) {
	    vector1[0]=points[caras[i*3+1]].x - points[caras[i*3]].x;
		vector1[1]=points[caras[i*3+1]].y - points[caras[i*3]].y;
	    vector1[2]=points[caras[i*3+1]].z - points[caras[i*3]].z;
	    vector2[0]=points[caras[i*3+2]].x - points[caras[i*3]].x;
	    vector2[1]=points[caras[i*3+2]].y - points[caras[i*3]].y;
	    vector2[2]=points[caras[i*3+2]].z - points[caras[i*3]].z;
	    Normal(vector1,vector2,normal);
		normales_tmp[i].x = float(normal[0]);
		normales_tmp[i].y = float(normal[1]);
		normales_tmp[i].z = float(normal[2]);
	} /* for */ 

	/* Calcular las normales a cada vértice según sus grupos "smooth": */ 
	for(int i = 0; i < ncaras; i++) {
		for(j=0;j<3;j++) {
			act_vertex=caras[i*3+j];
			if (smooth[i]==0) {
				normales[i*9+j*3]=normales_tmp[i].x;
				normales[i*9+j*3+1]=normales_tmp[i].y;
				normales[i*9+j*3+2]=normales_tmp[i].z;
			} else {
				num=0;
				normales[i*9+j*3]=0.0F;
				normales[i*9+j*3+1]=0.0F;
				normales[i*9+j*3+2]=0.0F;
				for(k=0;k<ncaras;k++) {
					if (smooth[k]==smooth[i] &&
						(caras[k*3]==act_vertex || caras[k*3+1]==act_vertex || caras[k*3+2]==act_vertex)) {
						num++;
						normales[i*9+j*3]+=normales_tmp[k].x;
						normales[i*9+j*3+1]+=normales_tmp[k].y;
						normales[i*9+j*3+2]+=normales_tmp[k].z;
					} /* if */ 
				} /* for */ 
				if (num!=0) {
					normales[i*9+j*3]/=num;
					normales[i*9+j*3+1]/=num;
					normales[i*9+j*3+2]/=num;
				} /* if */ 
			} /* if */ 
		} /* for */ 
	} /* for */ 

	delete[] normales_tmp;

} /* C3DObject::CalculaNormales */ 


C3DObject::~C3DObject()
{
	if (points!=NULL) delete []points;
	if (normales!=NULL) delete []normales;
	if (caras!=NULL) delete []caras;
	if (r!=NULL) delete []r;
	if (g!=NULL) delete []g;
	if (b!=NULL) delete []b;
	if (tx!=0) delete tx;
	if (ty!=0) delete ty;
	if (textures!=0) delete textures;
	tx=0;
	ty=0;
	textures=0;
} /* C3DObject::~CObject */ 



bool C3DObject::valid(void)
{
	if (npoints!=0 && ncaras!=0 && points!=NULL && normales!=NULL && caras!=NULL) return true;

	return false;
} /* C3DObject::valid */ 



void C3DObject::draw(void)
{
	int i;

	if (textures!=0) {
		if (display_list==-1) {
			display_list=glGenLists(1);
			glNewList(display_list,GL_COMPILE);
			/* Dibuja el objeto: */ 
			{
				glEnable(GL_TEXTURE_2D);
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3,GL_FLOAT,0,points);

				for(i=0;i<ncaras;i++) {
					glBindTexture(GL_TEXTURE_2D,textures[i]);
					glColor3f(1,1,1);

					glBegin(GL_TRIANGLES);
					glTexCoord2f(tx[i*3],ty[i*3]);
					glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
					glArrayElement(caras[i*3]);

					glTexCoord2f(tx[i*3+1],ty[i*3+1]);
					glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
					glArrayElement(caras[i*3+1]);

					glTexCoord2f(tx[i*3+2],ty[i*3+2]);
					glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
					glArrayElement(caras[i*3+2]);
					glEnd();                
				} /* for */             
				glDisable(GL_TEXTURE_2D);
			} 
			glEndList();

			glCallList(display_list);
		} else {
			glCallList(display_list);
		} /* if */ 

	} else {
		if (display_list==-1) {
			display_list=glGenLists(1);
			glNewList(display_list,GL_COMPILE);
			/* Dibuja el objeto: */ 
			{
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3,GL_FLOAT,0,points);

				glBegin(GL_TRIANGLES);
				for(i=0;i<ncaras;i++) {
					glColor3f(r[i],g[i],b[i]);
					glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
					glArrayElement(caras[i*3]);
					glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
					glArrayElement(caras[i*3+1]);
					glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
					glArrayElement(caras[i*3+2]);
				} /* for */ 
				glEnd();                
			} 
			glEndList();

			glCallList(display_list);
		} else {
			glCallList(display_list);
		} /* if */ 
	} /* if */ 
	
} /* C3DObject::draw */ 


void C3DObject::draw(float r,float g,float b)
{
	int i,off1;

	if (textures!=0) {
		if (display_list==-1) {
			display_list=glGenLists(1);
			glNewList(display_list,GL_COMPILE);
			/* Dibuja el objeto: */ 
			{
				glEnable(GL_TEXTURE_2D);
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3,GL_FLOAT,0,points);

				for(i=0,off1=0;i<ncaras;i++) {
					glBindTexture(GL_TEXTURE_2D,textures[i]);
					glColor3f(1,1,1);

					glBegin(GL_TRIANGLES);
					glTexCoord2f(tx[off1],ty[off1]);
					glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
					glArrayElement(caras[off1++]);

					glTexCoord2f(tx[off1],ty[off1]);
					glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
					glArrayElement(caras[off1++]);

					glTexCoord2f(tx[off1],ty[off1]);
					glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
					glArrayElement(caras[off1++]);
					glEnd();                
				} /* for */             
				glDisable(GL_TEXTURE_2D);
			} 
			glEndList();

			glCallList(display_list);
		} else {
			glCallList(display_list);
		} /* if */ 
	} else {
//              if (display_list==-1) {
//                      display_list=glGenLists(1);
//                      glNewList(display_list,GL_COMPILE_AND_EXECUTE);
			/* Dibuja el objeto: */ 
			{
				/* Dibuja el objeto: */ 
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3,GL_FLOAT,0,points);
				glColor3f(r,g,b);

				for(i=0,off1=0;i<ncaras;i++) {
					glBegin(GL_TRIANGLES);
					glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
					glArrayElement(caras[off1++]);
					glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
					glArrayElement(caras[off1++]);
					glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
					glArrayElement(caras[off1++]);
					glEnd();
				} /* for */ 
			}
//                      glEndList();
//              } else {
//                      glCallList(display_list);
//              } /* if */ 
	} /* if */ 
} /* C3DObject::draw */ 


void C3DObject::draw_notexture(float r,float g,float b)
{
	int i,off1;

	/* Dibuja el objeto: */ 
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,points);
	glColor3f(r,g,b);

	for(i=0,off1=0;i<ncaras;i++) {
		glBegin(GL_TRIANGLES);
		glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
		glArrayElement(caras[off1++]);
		glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
		glArrayElement(caras[off1++]);
		glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
		glArrayElement(caras[off1++]);
		glEnd();
	} /* for */ 
} /* draw_notexture */ 


void C3DObject::draw_notexture(float r,float g,float b,float a)
{
	int i,off1;

	/* Dibuja el objeto: */ 
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,0,points);
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glColor4f(r,g,b,a);

	for(i=0,off1=0;i<ncaras;i++) {
		glBegin(GL_TRIANGLES);
		glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
		glArrayElement(caras[off1++]);
		glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
		glArrayElement(caras[off1++]);
		glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
		glArrayElement(caras[off1++]);
		glEnd();
	} /* for */ 

	glDisable(GL_BLEND);
} /* draw_notexture */ 


void C3DObject::drawcmc(float r,float g,float b)
{
	cmc.draw(r,g,b);
} /* C3DObject::drawcmc */ 





void C3DObject::refresh_display_lists(void)
{
	glDeleteLists(display_list,1);
	display_list=-1;
} /* C3DObject::refresh_display_lists */ 


