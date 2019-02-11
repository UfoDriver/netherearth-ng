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



C3DObject::C3DObject(): points(0), faces(0), displayList(-1), textures(0)
{
}


C3DObject::C3DObject(const std::string& filename, const std::string& texturedir):
  points(0), faces(0), displayList(-1), textures(0)
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
  if (!data.compare(0, 10, "Face list:")) return 0;

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
  faces.emplace_back(p1, p2, p3);
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
        }
        if (buffer.find("Faces:") != std::string::npos) {
          std::istringstream iStr(buffer);
          int nfaces;
          iStr.seekg(buffer.find("Faces:") + 6);
          iStr >> nfaces;
          faces.reserve(nfaces);
          smooth = new int[nfaces];
        }
        if (points.capacity() && faces.capacity()) {
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
    faces.clear();
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
  Vector* normales_tmp = new Vector[faces.size()];

  // @TODO: use iterator
  for (int i = 0; i < faces.size(); i++) {
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
  // @TODO: use iterator
  for (int i = 0; i < faces.size(); i++) {
    if (smooth[i] == 0) {
      faces[i].norm1 = normales_tmp[i];
    } else {
      int num = 0;
      for(int k = 0; k < faces.size(); k++) {
        if (smooth[k] == smooth[i] && faces[k].hasVertex(faces[i].a)) {
          num++;
          faces[i].norm1 = faces[i].norm1 + normales_tmp[k];
        }
      }
      if (num != 0) {
        faces[i].norm1 = faces[i].norm1 / num;
      }
    }
  }
  // @TODO: use iterator
  for (int i = 0; i < faces.size(); i++) {
    if (smooth[i] == 0) {
      faces[i].norm2 = normales_tmp[i];
    } else {
      int num = 0;
      for(int k = 0; k < faces.size(); k++) {
        if (smooth[k]==smooth[i] && faces[k].hasVertex(faces[i].b)) {
          num++;
          faces[i].norm2 = faces[i].norm2 + normales_tmp[k];
        }
      }
      if (num != 0) {
        faces[i].norm2 = faces[i].norm2 / num;
      }
    }
  }
  // @TODO: use iterator
  for (int i = 0; i < faces.size(); i++) {
    if (smooth[i] == 0) {
      faces[i].norm3 = normales_tmp[i];
    } else {
      int num = 0;
      for(int k = 0; k < faces.size(); k++) {
        if (smooth[k]==smooth[i] && faces[k].hasVertex(faces[i].c)) {
          num++;
          faces[i].norm3 = faces[i].norm3 + normales_tmp[k];
        }
      }
      if ( num!=0 ) {
        faces[i].norm3 = faces[i].norm3 / num;
      }
    }
  }

  delete[] normales_tmp;
}


C3DObject::~C3DObject()
{
  points.clear();
}


bool C3DObject::valid(void)
{
  return points.size() && faces.size();
}


void C3DObject::draw()
{
  if (displayList != -1) {
    glCallList(displayList);
    return;
  }

  displayList = glGenLists(1);
  glNewList(displayList, GL_COMPILE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, points.data());

  if (textures.size()) {
    glEnable(GL_TEXTURE_2D);

    for (int i = 0; i < faces.size(); i++) {
      glBindTexture(GL_TEXTURE_2D, textures[i]);
      glColor3f(1, 1, 1);

      glBegin(GL_TRIANGLES);
      glTexCoord2f(textureCoord[i * 3].x, textureCoord[i * 3].y);
      glNormal3f(faces[i].norm1.x, faces[i].norm1.y, faces[i].norm1.z);
      glArrayElement(faces[i].a);

      glTexCoord2f(textureCoord[i * 3 + 1].x, textureCoord[i * 3 + 1].y);
      glNormal3f(faces[i].norm2.x, faces[i].norm2.y, faces[i].norm2.z);
      glArrayElement(faces[i].b);

      glTexCoord2f(textureCoord[i * 3 + 2].x, textureCoord[i * 3 + 2].y);
      glNormal3f(faces[i].norm3.x, faces[i].norm3.y, faces[i].norm3.z);
      glArrayElement(faces[i].c);
      glEnd();
    }
    glDisable(GL_TEXTURE_2D);
  } else {
    glBegin(GL_TRIANGLES);
    for(const Face& face: faces) {
      glColor3f(face.color.red, face.color.green, face.color.blue);
      glNormal3f(face.norm1.x, face.norm1.y, face.norm1.z);
      glArrayElement(face.a);
      glNormal3f(face.norm2.x, face.norm2.y, face.norm2.z);
      glArrayElement(face.b);
      glNormal3f(face.norm3.x, face.norm3.y, face.norm3.z);
      glArrayElement(face.c);
    }
    glEnd();
  }

  glEndList();
  glCallList(displayList);
}


void C3DObject::draw(const Color& color)
{
  if (textures.size()) {
    if (displayList == -1) {
      displayList = glGenLists(1);
      glNewList(displayList,GL_COMPILE);
      glEnable(GL_TEXTURE_2D);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, points.data());

      // @TODO: use iterator
      for (int i = 0; i < faces.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, textures[i]);
        glColor3f(1, 1, 1);

        glBegin(GL_TRIANGLES);
        glTexCoord2f(textureCoord[i * 3].x, textureCoord[i * 3].y);
        glNormal3f(faces[i].norm1.x, faces[i].norm1.y, faces[i].norm1.z);
        glArrayElement(faces[i].a);

        glTexCoord2f(textureCoord[i * 3 + 1].x, textureCoord[i * 3 + 1].y);
        glNormal3f(faces[i].norm2.x, faces[i].norm2.y, faces[i].norm2.z);
        glArrayElement(faces[i].b);

        glTexCoord2f(textureCoord[i * 3 + 2].x, textureCoord[i * 3 + 2].y);
        glNormal3f(faces[i].norm3.x, faces[i].norm3.y, faces[i].norm3.z);
        glArrayElement(faces[i].c);
        glEnd();
      }
      glDisable(GL_TEXTURE_2D);
      glEndList();
    }
    glCallList(displayList);
  } else {
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, points.data());
    glColor3f(color.red, color.green, color.blue);
    for (const Face face: faces) {
      glBegin(GL_TRIANGLES);
      glNormal3f(face.norm1.x, face.norm1.y, face.norm1.z);
      glArrayElement(face.a);
      glNormal3f(face.norm2.x, face.norm2.y, face.norm2.z);
      glArrayElement(face.b);
      glNormal3f(face.norm3.x, face.norm3.y, face.norm3.z);
      glArrayElement(face.c);
      glEnd();
    }
  }
}


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

  for (const Face face: faces) {
    glBegin(GL_TRIANGLES);
    glNormal3f(face.norm1.x, face.norm1.y, face.norm1.z);
    glArrayElement(face.a);
    glNormal3f(face.norm2.x, face.norm2.y, face.norm2.z);
    glArrayElement(face.b);
    glNormal3f(face.norm3.x, face.norm3.y, face.norm3.z);
    glArrayElement(face.c);
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
