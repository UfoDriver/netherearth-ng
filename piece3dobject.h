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
  void computeDynamicShadow(int angle, Vector light);
  void computeShadow(int angle, const Vector& light, std::vector<Vector>& points,
                     std::vector<Face>& shadowFaces, CMC& cmc);

  std::vector<Vector> shdw_puntos_0, shdw_puntos_90, shdw_puntos_180, shdw_puntos_270;
  std::vector<Vector> shdw_puntos_dynamic;
  std::vector<Face> shdw_faces_0, shdw_faces_90, shdw_faces_180, shdw_faces_270, shdw_faces_dynamic;
  CMC shdw_cmc_0, shdw_cmc_90, shdw_cmc_180, shdw_cmc_270, shdw_cmc_dynamic;
};

#endif
