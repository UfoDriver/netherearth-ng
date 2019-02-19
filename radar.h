#ifndef RADAR_H
#define RADAR_H

class NETHER;


class Radar {
public:
  explicit Radar(const NETHER* nether): needsRedraw(1), nether(nether) {};
  void draw(const int width, const int height, const int split, const int splity);
  int needsRedraw;
  void requestRedraw() { needsRedraw = 2; }

private:
  const NETHER* nether;
};

#endif // RADAR_H
