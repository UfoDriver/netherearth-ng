#ifdef _WIN32
#include "windows.h"
#endif

#include <algorithm>
#include <limits>

#include "3dobject.h"
#include "myglutaux.h"
#include "vector.h"


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

  // int *facematerial=0;
  // int **materials=0,nmaterials=0,*nsubmaterials=0;
  // char ***material_bitmaps=0;
  // char buffer[256];
  // FILE *fp;

  if (!lookfor("MATERIAL_LIST", iFile) ||
      !lookfor("MATERIAL_COUNT", iFile)) {
    return false;
  }

  int nmaterials;
  iFile >> nmaterials;

  std::vector<std::vector<int>> materials;
  std::vector<int> submaterials;
  // material_bitmaps=new char **[nmaterials];
  std::vector<std::vector<std::string>> material_bitmaps;
  // materials=new int *[nmaterials];
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
            //  material_bitmaps[materialRef][facematerial[i]] != 0) {
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
