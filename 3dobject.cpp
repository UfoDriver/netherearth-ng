#ifdef _WIN32
#include "windows.h"
#endif
#include "GL/gl.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <limits>
#include <sstream>

#include "3dobject.h"
#include "cmc.h"
#include "myglutaux.h"
#include "vector.h"


C3DObject::C3DObject(const std::string& filename, const std::string& texturedir, const Color& color):
         color(color)
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


int C3DObject::readFace(const std::string& data) {
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
  return act_face;
}


bool C3DObject::readSmoothing(const std::string& data, int currentFace) {
  std::istringstream inStr(data);
  inStr.ignore(100, ':');
  inStr >> faces[currentFace].smooth;
  return true;
}


bool C3DObject::loadASC(const std::string& filename)
{
  const int ST_INIT = 0;
  const int ST_DATA = 1;

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
        }
        if (points.capacity() && faces.capacity()) {
          state = ST_DATA;
        }
        break;
      case ST_DATA:
        if (!buffer.compare(0, 6, "Mapped")) {}
        if (!buffer.compare(0, 6, "Vertex")) { readVertex(buffer); }
        if (!buffer.compare(0, 4, "Face")) { act_face = readFace(buffer); }
        if (!buffer.compare(0, 9, "Smoothing")) { readSmoothing(buffer, act_face); }

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

  calculateNormales();
  cmc.set(points);

  return true;
}


float C3DObject::normalize(void)
{
  return normalize(1.0);
}


float C3DObject::normalize(float c)
{
  cmc.set(points);
  float fx = (cmc.x[1] - cmc.x[0]) / 2;
  float fy = (cmc.y[1] - cmc.y[0]) / 2;
  float fz = (cmc.z[1] - cmc.z[0]) / 2;
  float cx = (cmc.x[1] + cmc.x[0]) / 2;
  float cy = (cmc.y[1] + cmc.y[0]) / 2;
  float cz = (cmc.z[1] + cmc.z[0]) / 2;

  float factor = fx;
  factor = std::max(fy, factor);
  factor = std::max(fz, factor);

  factor /= c;

  for (Vector& point: points) {
    point.x = (point.x - cx) / factor;
    point.y = (point.y - cy) / factor;
    point.z = (point.z - cz) / factor;
  }

  cmc.set(points);
  return factor;
}


float C3DObject::normalizexy(float c)
{
  cmc.set(points);
  float fx = (cmc.x[1] - cmc.x[0]) / 2;
  float fy = (cmc.y[1] - cmc.y[0]) / 2;
  float cx = (cmc.x[1] + cmc.x[0]) / 2;
  float cy = (cmc.y[1] + cmc.y[0]) / 2;
  float cz = (cmc.z[1] + cmc.z[0]) / 2;

  float factor = std::max(fx, fy);

  factor /= c;

  for (Vector& point: points) {
    point.x = (point.x - cx) / factor;
    point.y = (point.y - cy) / factor;
    point.z = (point.z - cz) / factor;
  }
  cmc.set(points);
  return factor;
}


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


void C3DObject::calculateNormales()
{
  std::vector<Vector> normales_tmp;

  for (const Face& face: faces) {
    Vector vector1 = points[face.b] - points[face.a];
    Vector vector2 = points[face.c] - points[face.a];
    normales_tmp.push_back(vector1.normal(vector2));
  }

  // @TODO: temporary copypaste
  // @TODO: use iterator
  for (unsigned int i = 0; i < faces.size(); i++) {
    if (faces[i].smooth == 0) {
      faces[i].norm1 = normales_tmp[i];
      faces[i].norm2 = normales_tmp[i];
      faces[i].norm3 = normales_tmp[i];
    } else {
      int num = 0;
      // Ineffective search
      for(unsigned int k = 0; k < faces.size(); k++) {
        if (faces[k].smooth == faces[i].smooth && faces[k].hasVertex(faces[i].a)) {
          num++;
          faces[i].norm1 = faces[i].norm1 + normales_tmp[k];
        }
      }
      if (num != 0) {
        faces[i].norm1 = faces[i].norm1 / num;
      }

      num = 0;
      // Ineffective search
      for(unsigned int k = 0; k < faces.size(); k++) {
        if (faces[k].smooth == faces[i].smooth && faces[k].hasVertex(faces[i].b)) {
          num++;
          faces[i].norm2 = faces[i].norm2 + normales_tmp[k];
        }
      }
      if (num != 0) {
        faces[i].norm2 = faces[i].norm2 / num;
      }

      num = 0;
      // Ineffective search
      for(unsigned int k = 0; k < faces.size(); k++) {
        if (faces[k].smooth == faces[i].smooth && faces[k].hasVertex(faces[i].c)) {
          num++;
          faces[i].norm3 = faces[i].norm3 + normales_tmp[k];
        }
      }
      if ( num!=0 ) {
        faces[i].norm3 = faces[i].norm3 / num;
      }
    }
  }
}


C3DObject::~C3DObject()
{
  points.clear();
}


bool C3DObject::valid(void)
{
  return points.size() && faces.size();
}


void C3DObject::draw() const
{
  if (displayList != -1) {
    glCallList(displayList);
    return;
  }

  displayList = glGenLists(1);
  glNewList(displayList, GL_COMPILE);
  glEnableClientState(GL_VERTEX_ARRAY);
  glVertexPointer(3, GL_FLOAT, 0, points.data());

  if (textured) {
    glEnable(GL_TEXTURE_2D);

    for (unsigned int i = 0; i < faces.size(); i++) {
      glBindTexture(GL_TEXTURE_2D, faces[i].texture);
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


void C3DObject::draw(const Color& color) const
{
  if (textured) {
    if (displayList == -1) {
      displayList = glGenLists(1);
      glNewList(displayList,GL_COMPILE);
      glEnable(GL_TEXTURE_2D);
      glEnableClientState(GL_VERTEX_ARRAY);
      glVertexPointer(3, GL_FLOAT, 0, points.data());

      // @TODO: use iterator
      for (unsigned int i = 0; i < faces.size(); i++) {
        glBindTexture(GL_TEXTURE_2D, faces[i].texture);
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
    for (const Face& face: faces) {
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
  // drawcmc({1, 0, 0});
}


void C3DObject::drawcmc(const Color& color) const
{
  cmc.draw(color);
}


void C3DObject::refresh_display_lists()
{
  glDeleteLists(displayList, 1);
  displayList = -1;
}


bool nexttag(std::string& tag, std::ifstream& iFile)
{
  iFile.ignore(std::numeric_limits<std::streamsize>::max(), '*');
  if (iFile.eof()) return false;
  iFile >> tag;
  return true;
}


bool nexttaginside(std::string& tag, std::ifstream& iFile)
{
  int parentheses = 0;
  bool instring = false;
  int c;

  do {
    c = iFile.get();
    if (c == '\"') {
      if (instring)
        instring = false;
      else
        instring = true;
    }
    if (!instring && c == '{') parentheses++;
    if (!instring && c == '}') parentheses--;
  } while(instring || (c != '*' && !iFile.eof() && parentheses >= 0));
  if (iFile.eof() || parentheses < 0) return false;

  iFile >> tag;

  return true;
}


bool skipinfo(std::ifstream& iFile, int parentheses)
{
  bool instring = false;
  int c;

  do {
    c = iFile.get();
    if (c == '\r') continue;
    if (c == '\"') {
      if (instring)
        instring = false;
      else
        instring = true;
    }
    if (!instring && c == '{') parentheses++;
    if (!instring && c == '}') parentheses--;
  } while ((c != '\n' || parentheses != 0 || instring == true) && !iFile.eof());
  if (iFile.eof()) return false;
  return true;
}


bool lookfor(const std::string& tag, std::ifstream& iFile)
{
  std::string tagname;

  for(;;) {
    if (!nexttag(tagname, iFile)) return false;
    if (tag == tagname) return true;
    skipinfo(iFile, 0);
  }
  return false;
}


int lookfor2(const std::string& tag, const std::string& tag2, std::ifstream& iFile)
{
  std::string tagname;

  for (;;) {
    if (!nexttag(tagname, iFile)) return 0;
    if (tag == tagname) return 1;
    if (tag2 == tagname) return 2;
    skipinfo(iFile, 0);
  }

  return 0;
}


bool lookforinside(const std::string& tag, std::ifstream& iFile)
{
  std::string tagname;

  for (;;) {
    if (!nexttaginside(tagname, iFile)) return false;
    if (tag == tagname) return true;
    skipinfo(iFile, 0);
  }

  return false;
}


bool readcomment(std::string& data, std::ifstream& iFile)
{
  iFile.ignore(std::numeric_limits<std::streamsize>::max(), '\"');
  std::getline(iFile, data, '\"');
  if (iFile.eof()) return false;
  return true;
}


std::string convertPath(const std::string& texturedir, std::string& bmpname)
{
  std::replace(bmpname.begin(), bmpname.end(), '\\', '/');
  std::string realBitmapPath {texturedir};
  if (bmpname.rfind('/') == std::string::npos) {
    realBitmapPath.append(bmpname);
  } else {
    realBitmapPath.append(bmpname.substr(bmpname.rfind('/') + 1));
  }
  return realBitmapPath;
}


bool C3DObject::loadASE(const std::string& filename, const std::string& texturedir)
{
  std::ifstream iFile(filename, std::ios::binary);
  std::string buffer;

  if (!lookfor("MATERIAL_LIST", iFile) ||
      !lookfor("MATERIAL_COUNT", iFile)) {
    return false;
  }

  int nmaterials;
  iFile >> nmaterials;

  std::vector<std::vector<int>> materials;
  std::vector<int> submaterials;
  std::vector<std::vector<std::string>> material_bitmaps;
  std::vector<int> nsubmaterials;


  materials.resize(nmaterials);
  material_bitmaps.resize(nmaterials);
  for (int j = 0; j < nmaterials; j++) {
    if (!lookfor("MATERIAL", iFile) ||
        !lookfor("MATERIAL_CLASS", iFile) ||
        !readcomment(buffer, iFile)) {
      return false;
    }

    if (buffer == "Standard") {
      // Standard Material, has no Submaterials:
      nsubmaterials.push_back(0);
      materials[j].push_back(0);

      if (lookforinside("MAP_DIFFUSE", iFile) && lookfor("BITMAP", iFile)) {

        std::string bmpname;
        if (!readcomment(bmpname, iFile)) {
          return false;
        }
        material_bitmaps[j].push_back(convertPath(texturedir, bmpname));
      }
    } else {
      // Composed material, has submaterias:
      if (!lookfor("NUMSUBMTLS", iFile)) {
        return false;
      }
      int nsubs;
      iFile >> nsubs;
      nsubmaterials.push_back(nsubs);

      materials[j].resize(nsubs);
      // for(int i = 0; i < nsubmaterials[j]; i++) materials[j][i] = 0;
      material_bitmaps[j].resize(nsubmaterials[j]);

      for(int i = 0; i < nsubmaterials[j]; i++) {
        if (!lookfor("SUBMATERIAL", iFile)) {
          return false;
        }
        if (lookforinside("MAP_DIFFUSE", iFile) && lookfor("BITMAP", iFile)) {

          std::string bmpname;
          if (!readcomment(bmpname, iFile)) {
            return false;
          }
          material_bitmaps[j][i] = convertPath(texturedir, bmpname);
        }
      }
    }
  }

  if (!lookfor("GEOMOBJECT", iFile) ||
      !lookfor("MESH", iFile)) {
    return false;
  }

  int npoints;
  if (!lookfor("MESH_NUMVERTEX", iFile))
    return false;
  iFile >> npoints;

  points.reserve(npoints);

  int nfaces;
  if (!lookfor("MESH_NUMFACES", iFile))
    return false;
  iFile >> nfaces;

  faces.reserve(nfaces);

  std::vector<int> facematerial;

  if (!lookfor("MESH_VERTEX_LIST", iFile))
    return false;

  for(int i = 0; i < npoints; i++) {
    int p;
    float x, y, z;

    if (!lookfor("MESH_VERTEX", iFile))
      return false;

    iFile >> p >> x >> y >> z;
    points.emplace_back(x, y, z);
  }

  if (!lookfor("MESH_FACE_LIST", iFile))
    return false;

  for (int i = 0; i < nfaces; i++) {
    std::string ignored;
    Face face(0, 0, 0, Color(0.5, 0.5, 0.5));

    if (!lookfor("MESH_FACE", iFile))
      return false;

    iFile >> ignored >> ignored >> face.a >> ignored >> face.b >> ignored >> face.c;

    if (!lookfor("MESH_SMOOTHING", iFile))
      return false;
    iFile >> face.smooth;
    faces.push_back(face);

    if (!lookfor("MESH_MTLID", iFile))
      return false;

    int mtlId;
    iFile >> mtlId;
    facematerial.push_back(mtlId);
  }

  textureCoord.clear();
  if (nmaterials != 0) {
    int v = lookfor2("MESH_FACEMAPLIST", "MESH_NUMTVERTEX", iFile);

    if (v == 1) {
      // MESH_FACEMAPLIST:
      int f;
      float x, y, z;

      for(int i = 0; i < nfaces; i++) {
        if (!lookfor("MESH_FACEMAP", iFile))
          return false;

        iFile >> f;

        if (!lookfor("MESH_FACEMAPVERT", iFile))
          return false;

        iFile >> x >> y >> z;
        textureCoord.emplace_back(x, 1 - y);
        if (!lookfor("MESH_FACEMAPVERT", iFile))
          return false;

        iFile >> x >> y >> z;
        textureCoord.emplace_back(x, 1 - y);
        if (!lookfor("MESH_FACEMAPVERT", iFile))
          return false;
        iFile >> x >> y >> z;
        textureCoord.emplace_back(x, 1 - y);
      }
    }

    if (v == 2) {
      // MESH_NUMTVERTEX:
      int ntv, ntf, n, p1, p2, p3;
      float x, y;
      std::vector<float> tv;

      iFile >> ntv;
      if (!lookfor("MESH_TVERTLIST", iFile))
        return false;

      for (int i = 0; i < ntv; i++) {
        if (!lookfor("MESH_TVERT", iFile))
          return false;

        iFile >> n >> x >> y;
        tv.push_back(x);
        tv.push_back(y);
      }

      if (!lookfor("MESH_NUMTVFACES", iFile))
        return false;

      iFile >> ntf;
      if (!lookfor("MESH_TFACELIST", iFile))
        return false;

      for(int i = 0; i < ntf; i++) {
        if (!lookfor("MESH_TFACE", iFile))
          return false;
        iFile >> n >> p1 >> p2 >> p3;
        textureCoord.emplace_back(tv[p1 * 2], 1 - tv[p1 * 2 + 1]);
        textureCoord.emplace_back(tv[p2 * 2], 1 - tv[p2 * 2 + 1]);
        textureCoord.emplace_back(tv[p3 * 2], 1 - tv[p3 * 2 + 1]);
      }
    }

    /* Create all the materials used by the faces: */
    {
      int materialRef;

      if (!lookfor("MATERIAL_REF", iFile))
        return false;
      iFile >> materialRef;
      textured = true;

      for (int i = 0; i < nfaces; i++) {
        if (facematerial[i] > nsubmaterials[materialRef])
          facematerial[i] = 0;

        if (materials[materialRef][facematerial[i]] == 0 &&
            !material_bitmaps[materialRef][facematerial[i]].empty()) {

          materials[materialRef][facematerial[i]] =
            createTexture(material_bitmaps[materialRef][facematerial[i]].c_str());
        }
        faces[i].texture = materials[materialRef][facematerial[i]];
      }
    }
  }

  calculateNormales();
  cmc.set(points);
  return true;
}
