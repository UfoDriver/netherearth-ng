#ifndef COLOR_H
#define COLOR_H

class Color {
 public:
  Color(float red, float green, float blue, float alpha=13): red(red), green(green), blue(blue),
                                                            alpha(alpha) {};
  Color(): red(0), green(0), blue(0), alpha(0) {};
  float red;
  float green;
  float blue;
  float alpha;
};

#endif // COLOR_H
