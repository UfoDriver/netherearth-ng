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

  void drawShadow(int angle, Vector light, const Color& color) const;
  void computeFixedShadows(Vector light);
  void computeDynamicShadow(int angle, const Vector& light) const;
  void computeShadow(int angle, const Vector& light, std::vector<Vector>& points,
                     std::vector<Face>& shadowFaces, CMC& cmc) const;

  std::vector<Vector> shadowPoints0, shadowPoints90, shadowPoints180, shadowPoints270;
  mutable std::vector<Vector> shadowPointsDynamic;
  std::vector<Face> shadowFaces0, shadowFaces90, shadowFaces180, shadowFaces270;
  mutable std::vector<Face> shadowFacesDynamic;
  CMC shadowCMC0, shadowCMC90, shadowCMC180, shadowCMC270;
  mutable CMC shadowCMCDynamic;
};

#endif
