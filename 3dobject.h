#ifndef __BRAIN_3DOBJECT
#define __BRAIN_3DOBJECT

#include <string>
#include <fstream>

#include "cmc.h"
#include "color.h"
#include "vector.h"


class C3DObject {
public:
  C3DObject();
  C3DObject(const std::string& filename, const std::string& texturedir);

  ~C3DObject();

  void refresh_display_lists(void);

  bool loadASC(const std::string& filename);
  bool loadASE(const std::string& filename, const std::string& texturedir);

  bool valid();

  void draw();
  void draw(const Color& color);
  void draw_notexture(const Color& color);
  void drawcmc(const Color& color);

	void CalculaNormales(int *smooth);

	float normalize(void);						/* Sets the maximum axis size to 1.0 or -1.0 */ 
	float normalize(float c);					/* Sets the maximum axis size to c or -c */ 
	float normalizexy(float c);					/* Sets the maximum X and Y axis size to c or -c */ 
	void makepositive(void);					/* Sets the minimum coordinates to 0,0,0 */ 
	void makepositivex(void);					/* Sets the minimum coordinates in X axis to 0 */ 
	void makepositivey(void);					/* Sets the minimum coordinates in Y axis to 0 */ 
	void makepositivez(void);					/* Sets the minimum coordinates in Z axis to 0 */ 
	void moveobject(const Vector& distance);


	int npoints;
	int nfaces;
	Vector *points;
	float *normales;
	int *faces;

  Color* faceColors;

  int displayList;
  CMC cmc;

	float *tx, *ty;
	unsigned int *textures;

private:
  bool readVertex(const std::string& data);
  int readFace(const std::string& data, int* smooth);
  bool readSmoothing(const std::string& data, int* smooth, int currentFace);
};

#endif
