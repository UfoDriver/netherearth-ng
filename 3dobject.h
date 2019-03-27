#ifndef __BRAIN_3DOBJECT
#define __BRAIN_3DOBJECT

#include <fstream>
#include <string>
#include <vector>

#include "cmc.h"
#include "color.h"
#include "vector.h"


class Face {
public:
  Face(int a, int b, int c): a(a), b(b), c(c), texture(0), smooth(0) {}
  Face(int a, int b, int c, const Color& color): a(a), b(b), c(c), color(color), texture(0), smooth(0) {}
  Face(): a(0), b(0), c(0), texture(0), smooth(0) {}
  bool hasVertex(int vertex) {return a == vertex || b == vertex || c == vertex;}
  int a;
  int b;
  int c;

  Vector norm1;
  Vector norm2;
  Vector norm3;

  Color color;
  int texture;
  int smooth;
};


class TextureCoordinate {
public:
  TextureCoordinate(float x, float y) : x {x}, y {y} {}
  float x;
  float y;
};


class C3DObject {
public:
  C3DObject() {}

  C3DObject(const std::string& filename, const std::string& texturedir,
            const Color& color=Color(1, 1, 1, 1));

  ~C3DObject();

  void refresh_display_lists(void);

  bool loadASC(const std::string& filename);
  bool loadASE(const std::string& filename, const std::string& texturedir);

  bool valid();

  void draw() const;
  void draw(const Color& color) const;
  void draw_notexture(const Color& color) const;
  void drawcmc(const Color& color) const;

  void calculateNormales();

	float normalize(void);						/* Sets the maximum axis size to 1.0 or -1.0 */ 
	float normalize(float c);					/* Sets the maximum axis size to c or -c */ 
	float normalizexy(float c);					/* Sets the maximum X and Y axis size to c or -c */ 
	void makepositive(void);					/* Sets the minimum coordinates to 0,0,0 */ 
	void makepositivex(void);					/* Sets the minimum coordinates in X axis to 0 */ 
	void makepositivey(void);					/* Sets the minimum coordinates in Y axis to 0 */ 
	void makepositivez(void);					/* Sets the minimum coordinates in Z axis to 0 */ 
	void moveobject(const Vector& distance);

  std::vector<Vector> points;
  std::vector<Face> faces;

  CMC cmc;
  Color color;

  std::vector<TextureCoordinate> textureCoord;

private:
  mutable int displayList {-1};
  bool readVertex(const std::string& data);
  int readFace(const std::string& data);
  bool readSmoothing(const std::string& data, int currentFace);
  bool textured {false};
};

#endif
