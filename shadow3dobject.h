#ifndef __BRAIN_SHADOW3DOBJECT
#define __BRAIN_SHADOW3DOBJECT

#include <string>
#include <vector>

#include "3dobject.h"


class Shadow3DObject : public C3DObject {
public:
  Shadow3DObject();
  Shadow3DObject(const std::string& file, const std::string& texturedir);
  ~Shadow3DObject();

  void drawShadow(const Color& color);
  void computeShadow(Vector light);

  std::vector<Vector> shadowPoints;
  std::vector<Face> shadowFaces;
  CMC shadowCMC;
};

#endif
