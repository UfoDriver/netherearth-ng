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

  void DrawShadow(int angle,Vector light,float r,float g,float b,float a);
  void ComputeFixedShadows(Vector light);
  void ComputeDynamicShadow(int angle,Vector light);
  void ComputeShadow(int angle,Vector light,int *np,int *nc,const Vector *p,int **c,CMC *cmc);

  int shdw_npuntos_0,shdw_npuntos_90,shdw_npuntos_180,shdw_npuntos_270, shdw_npuntos_dynamic;
  int shdw_nfaces_0,shdw_nfaces_90,shdw_nfaces_180,shdw_nfaces_270,shdw_nfaces_dynamic;
  Vector *shdw_puntos_0, *shdw_puntos_90,*shdw_puntos_180,*shdw_puntos_270;
  Vector *shdw_puntos_dynamic;
  int *shdw_faces_0,*shdw_faces_90,*shdw_faces_180,*shdw_faces_270,*shdw_faces_dynamic;
  CMC shdw_cmc_0,shdw_cmc_90,shdw_cmc_180,shdw_cmc_270,shdw_cmc_dynamic;
};

#endif
