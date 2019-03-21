#ifndef __BRAIN_PIECE3DOBJECT
#define __BRAIN_PIECE3DOBJECT

#include <string>
#include "vector.h"
#include "3dobject.h"


class Piece3DObject : public C3DObject {
public:
  Piece3DObject();
  Piece3DObject(const std::string& file, const std::string& texturedir);
  ~Piece3DObject();

  void drawShadow(int angle, Vector light, const Color& color);
  void computeFixedShadows(Vector light);
  void computeDynamicShadow(int angle, const Vector& light);
  void computeShadow(int angle, const Vector& light, std::vector<Vector>& points,
                     std::vector<Face>& shadowFaces, CMC& cmc);

  std::vector<Vector> shadowPoints0, shadowPoints90, shadowPoints180, shadowPoints270;
  std::vector<Vector> shadowPointsDynamic;
  std::vector<Face> shadowFaces0, shadowFaces90, shadowFaces180, shadowFaces270, shadowFacesDynamic;
  CMC shadowCMC0, shadowCMC90, shadowCMC180, shadowCMC270, shadowCMCDynamic;
};

#endif
