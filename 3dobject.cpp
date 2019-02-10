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



C3DObject::C3DObject(): nfaces(0), normales(0), faces(0),
                        displayList(-1), tx(0), ty(0), textures(0)
{
}


C3DObject::C3DObject(const std::string& filename, const std::string& texturedir):
  nfaces(0), normales(0), faces(0), displayList(-1),
  tx(0), ty(0), textures(0)
{
  int l = filename.length();

  if (filename[l - 1] == 'c' || filename[l - 1] == 'C') {
    loadASC(filename);
  } else {
    loadASE(filename, texturedir);
  }
}


bool C3DObject::readVertex(const std::string& data)
{
  if (!data.compare(0, 12, "Vertex list:")) return false;

  std::istringstream inStr(data);
  int vertexNumber = 0;
  float x, y, z;
  inStr.ignore(100, ' ');
  inStr >> vertexNumber;
  inStr.ignore(100, ':');
  inStr.ignore(100, ':');
  inStr >> x;
  inStr.ignore(100, ':');
  inStr >> y;
  inStr.ignore(100, ':');
  inStr >> z;
  points.emplace_back(x, y, z);
 
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

  faces[act_face].a = p1;
  faces[act_face].b = p2;
  faces[act_face].c = p3;

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

    switch (state) {
      case ST_INIT:
        if (buffer.find("Vertices:") != std::string::npos) {
          std::istringstream iStr(buffer);
          int npoints;
          iStr.seekg(buffer.find("Vertices:") + 9);
          iStr >> npoints;
          points.reserve(npoints);
          if (nfaces != 0) state = ST_DATA;
        }
        if (buffer.find("Faces:") != std::string::npos) {
          std::istringstream iStr(buffer);
          iStr.seekg(buffer.find("Faces:") + 6);
          iStr >> nfaces;
          faces = new Face[nfaces];
          smooth = new int[nfaces];
          for(int i = 0;i < nfaces; i++) {
            faceColors.emplace_back(0.5, 0.5, 0.5);
          }
          state = ST_DATA;
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
    points.clear();
    if (faces != NULL) delete[] faces;
    nfaces = 0;
    faces = NULL;
    return false;
  }

  CalculaNormales(smooth);
  cmc.set(points);
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

	cmc.set(points);
	fx=(cmc.x[1]-cmc.x[0])/2;
	fy=(cmc.y[1]-cmc.y[0])/2;
	fz=(cmc.z[1]-cmc.z[0])/2;
	cx=(cmc.x[1]+cmc.x[0])/2;
	cy=(cmc.y[1]+cmc.y[0])/2;
	cz=(cmc.z[1]+cmc.z[0])/2;

	factor=fx;
	if (fy>factor) factor=fy;
	if (fz>factor) factor=fz;

	factor /= c;

    for (Vector& point: points) {
      point.x = (point.x - cx) / factor;
      point.y = (point.y - cy) / factor;
      point.z = (point.z - cz) / factor;
    }

    cmc.set(points);

    return factor;
} /* normalize */ 


float C3DObject::normalizexy(float c)
{
	int i;
	float cx,fx,cy,fy,cz,factor;

	cmc.set(points);
	fx=(cmc.x[1]-cmc.x[0])/2;
	fy=(cmc.y[1]-cmc.y[0])/2;
	cx=(cmc.x[1]+cmc.x[0])/2;
	cy=(cmc.y[1]+cmc.y[0])/2;
	cz=(cmc.z[1]+cmc.z[0])/2;

	factor=fx;
	if (fy>factor) factor=fy;

	factor/=c;

    for (Vector& point: points) {
      point.x = (point.x - cx) / factor;
      point.y = (point.y - cy) / factor;
      point.z = (point.z - cz) / factor;
    }
    cmc.set(points);

	return factor;
} /* normalizexy */ 


void C3DObject::makepositive(void)
{
  cmc.set(points);

  for (Vector& point: points) {
    point.x -= cmc.x[0];
    point.y -= cmc.y[0];
    point.z -= cmc.z[0];
  }
  cmc.set(points);
}


void C3DObject::makepositivex(void)
{
  cmc.set(points);

  for (Vector& point: points) {
    point.x -= cmc.x[0];
  }
  cmc.set(points);
}


void C3DObject::makepositivey(void)
{
  cmc.set(points);

  for (Vector& point: points) {
    point.y -= cmc.y[0];
  }
  cmc.set(points);
}


void C3DObject::makepositivez(void)
{
  cmc.set(points);

  for (Vector& point: points) {
    point.z -= cmc.z[0];
  }
  cmc.set(points);
}


void C3DObject::moveobject(const Vector& distance)
{
  for(Vector& point: points) {
    point = point + distance;
  }
  cmc.set(points);
}


void C3DObject::CalculaNormales(int *smooth)
{
  normales = new float[nfaces*3*3];

  int num;
  Vector* normales_tmp = new Vector[nfaces];

  for (int i = 0; i < nfaces; i++) {
    double vector1[3], vector2[3], normal[3];
    vector1[0]=points[faces[i].b].x - points[faces[i].a].x;
    vector1[1]=points[faces[i].b].y - points[faces[i].a].y;
    vector1[2]=points[faces[i].b].z - points[faces[i].a].z;
    vector2[0]=points[faces[i].c].x - points[faces[i].a].x;
    vector2[1]=points[faces[i].c].y - points[faces[i].a].y;
    vector2[2]=points[faces[i].c].z - points[faces[i].a].z;
    Normal(vector1,vector2,normal);
    normales_tmp[i].x = float(normal[0]);
    normales_tmp[i].y = float(normal[1]);
    normales_tmp[i].z = float(normal[2]);
  }

  // @TODO: temporary copypaste
  for (int i = 0; i < nfaces; i++) {
    int act_vertex = faces[i].a;
    int j = 0;
    if (smooth[i]==0) {
      normales[i*9+j*3]=normales_tmp[i].x;
      normales[i*9+j*3+1]=normales_tmp[i].y;
      normales[i*9+j*3+2]=normales_tmp[i].z;
    } else {
      num=0;
      normales[i*9+j*3]=0.0F;
      normales[i*9+j*3+1]=0.0F;
      normales[i*9+j*3+2]=0.0F;
      for(int k = 0; k < nfaces; k++) {
        if (smooth[k]==smooth[i] && faces[k].hasVertex(act_vertex)) {
          std::cerr << "Oh" << std::endl;
          num++;
          normales[i*9+j*3]+=normales_tmp[k].x;
          normales[i*9+j*3+1]+=normales_tmp[k].y;
          normales[i*9+j*3+2]+=normales_tmp[k].z;
        }
      }
      if (num!=0) {
        std::cerr << "Num is not zero" << std::endl;
        normales[i*9+j*3]/=num;
        normales[i*9+j*3+1]/=num;
        normales[i*9+j*3+2]/=num;
      }
    }
  }
  for (int i = 0; i < nfaces; i++) {
    int act_vertex = faces[i].b;
    int j = 1;
    if (smooth[i]==0) {
      normales[i*9+j*3]=normales_tmp[i].x;
      normales[i*9+j*3+1]=normales_tmp[i].y;
      normales[i*9+j*3+2]=normales_tmp[i].z;
    } else {
      num=0;
      normales[i*9+j*3]=0.0F;
      normales[i*9+j*3+1]=0.0F;
      normales[i*9+j*3+2]=0.0F;
      for(int k = 0; k < nfaces; k++) {
        if (smooth[k]==smooth[i] && faces[k].hasVertex(act_vertex)) {
          num++;
          normales[i*9+j*3]+=normales_tmp[k].x;
          normales[i*9+j*3+1]+=normales_tmp[k].y;
          normales[i*9+j*3+2]+=normales_tmp[k].z;
        }
      }
      if (num!=0) {
        normales[i*9+j*3]/=num;
        normales[i*9+j*3+1]/=num;
        normales[i*9+j*3+2]/=num;
      }
    }
  }
  for (int i = 0; i < nfaces; i++) {
    int act_vertex = faces[i].c;
    int j = 2;
    if (smooth[i]==0) {
      normales[i*9+j*3]=normales_tmp[i].x;
      normales[i*9+j*3+1]=normales_tmp[i].y;
      normales[i*9+j*3+2]=normales_tmp[i].z;
    } else {
      num=0;
      normales[i*9+j*3]=0.0F;
      normales[i*9+j*3+1]=0.0F;
      normales[i*9+j*3+2]=0.0F;
      for(int k = 0; k < nfaces; k++) {
        if (smooth[k]==smooth[i] && faces[k].hasVertex(act_vertex)) {
          num++;
          normales[i*9+j*3]+=normales_tmp[k].x;
          normales[i*9+j*3+1]+=normales_tmp[k].y;
          normales[i*9+j*3+2]+=normales_tmp[k].z;
        }
      }
      if (num!=0) {
        normales[i*9+j*3]/=num;
        normales[i*9+j*3+1]/=num;
        normales[i*9+j*3+2]/=num;
      }
    }
  }

  delete[] normales_tmp;
}


C3DObject::~C3DObject()
{
  points.clear();
  if (normales != NULL) delete[] normales;
  if (faces != NULL) delete[] faces;
  if (tx != 0) delete tx;
  if (ty != 0) delete ty;
  if (textures != 0) delete textures;
  tx = 0;
  ty = 0;
  textures = 0;
}


bool C3DObject::valid(void)
{
  return points.size() && nfaces && normales && faces;
}


void C3DObject::draw(void)
{
	if (textures!=0) {
		if (displayList==-1) {
			displayList=glGenLists(1);
			glNewList(displayList,GL_COMPILE);
			/* Dibuja el objeto: */ 
			{
				glEnable(GL_TEXTURE_2D);
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3,GL_FLOAT,0,points.data());

				for(int i = 0; i < nfaces; i++) {
					glBindTexture(GL_TEXTURE_2D,textures[i]);
					glColor3f(1,1,1);

					glBegin(GL_TRIANGLES);
					glTexCoord2f(tx[i*3],ty[i*3]);
					glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
					glArrayElement(faces[i].a);

					glTexCoord2f(tx[i*3+1],ty[i*3+1]);
					glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
					glArrayElement(faces[i].b);

					glTexCoord2f(tx[i*3+2],ty[i*3+2]);
					glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
					glArrayElement(faces[i].c);
					glEnd();                
				} /* for */             
				glDisable(GL_TEXTURE_2D);
			} 
			glEndList();

			glCallList(displayList);
		} else {
			glCallList(displayList);
		} /* if */ 

	} else {
		if (displayList==-1) {
			displayList=glGenLists(1);
			glNewList(displayList,GL_COMPILE);
			/* Dibuja el objeto: */ 
			{
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3,GL_FLOAT,0,points.data());

				glBegin(GL_TRIANGLES);
				for(int i = 0; i < nfaces; i++) {
					glColor3f(faceColors[i].red, faceColors[i].green, faceColors[i].blue);
					glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
					glArrayElement(faces[i].a);
					glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
					glArrayElement(faces[i].b);
					glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
					glArrayElement(faces[i].c);
				} /* for */ 
				glEnd();                
			} 
			glEndList();

			glCallList(displayList);
		} else {
			glCallList(displayList);
		} /* if */ 
	} /* if */ 
	
} /* C3DObject::draw */ 


void C3DObject::draw(const Color& color)
{
	if (textures!=0) {
		if (displayList==-1) {
			displayList=glGenLists(1);
			glNewList(displayList,GL_COMPILE);
			/* Dibuja el objeto: */ 
			{
				glEnable(GL_TEXTURE_2D);
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3,GL_FLOAT,0,points.data());

				for(int i=0,off1=0;i<nfaces;i++) {
					glBindTexture(GL_TEXTURE_2D,textures[i]);
					glColor3f(1,1,1);

					glBegin(GL_TRIANGLES);
					glTexCoord2f(tx[off1],ty[off1]);
					glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
					glArrayElement(faces[i].a);
                    off1++;

					glTexCoord2f(tx[off1],ty[off1]);
					glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
					glArrayElement(faces[i].b);
                    off1++;

					glTexCoord2f(tx[off1],ty[off1]);
					glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
					glArrayElement(faces[i].c);
                    off1++;

					glEnd();
				} /* for */
				glDisable(GL_TEXTURE_2D);
			}
			glEndList();

			glCallList(displayList);
		} else {
			glCallList(displayList);
		} /* if */ 
	} else {
//              if (display_list==-1) {
//                      display_list=glGenLists(1);
//                      glNewList(display_list,GL_COMPILE_AND_EXECUTE);
			/* Dibuja el objeto: */ 
			{
				/* Dibuja el objeto: */
              
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, 0, points.data());
				glColor3f(color.red, color.green, color.blue);

				for(int i = 0; i < nfaces; i++) {
					glBegin(GL_TRIANGLES);
					glNormal3f(normales[i*9+0],normales[i*9+1],normales[i*9+2]);
					glArrayElement(faces[i].a);
					glNormal3f(normales[i*9+3],normales[i*9+4],normales[i*9+5]);
					glArrayElement(faces[i].b);
					glNormal3f(normales[i*9+6],normales[i*9+7],normales[i*9+8]);
					glArrayElement(faces[i].c);
					glEnd();
				} /* for */ 
			}
//                      glEndList();
//              } else {
//                      glCallList(display_list);
//              } /* if */ 
	} /* if */ 
} /* C3DObject::draw */ 


void C3DObject::draw_notexture(const Color& color)
{
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, points.data());

  if (color.alpha) {
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glColor4f(color.red, color.green, color.blue, color.alpha);
  } else {
    glColor3f(color.red, color.green, color.blue);
  }

  for (int i = 0; i < nfaces; i++) {
    glBegin(GL_TRIANGLES);
    glNormal3f(normales[i * 9 + 0], normales[i * 9 + 1],normales[i * 9 + 2]);
    glArrayElement(faces[i].a);
    glNormal3f(normales[i * 9 + 3], normales[i * 9 + 4], normales[i * 9 + 5]);
    glArrayElement(faces[i].b);
    glNormal3f(normales[i * 9 + 6], normales[i * 9 + 7], normales[i * 9 + 8]);
    glArrayElement(faces[i].c);
    glEnd();
  }
}


void C3DObject::drawcmc(const Color& color)
{
  cmc.draw(color.red, color.green, color.blue);
}


void C3DObject::refresh_display_lists()
{
  glDeleteLists(displayList, 1);
  displayList = -1;
}
