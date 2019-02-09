#ifndef COLOR_H
#define COLOR_H

class Color {
 public:
  Color(float red, float green, float blue, int alpha=1): red(red), green(green), blue(blue),
                                                          alpha(alpha) {};
  Color(): red(0), green(0), blue(0), alpha(0) {};
  float red, green, blue;
  int alpha;
};

#endif // COLOR_H
