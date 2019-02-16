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

  void DrawShadow(const Color& color);
  void ComputeShadow(Vector light);

  std::vector<Vector> shdw_puntos;
	int shdw_nfaces;
	int *shdw_faces;
	CMC shdw_cmc;
};

#endif
