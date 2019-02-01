#ifndef RADAR_H
#define RADAR_H

class NETHER;


class Radar {
public:
  explicit Radar(const NETHER* nether): needsRedraw(1), nether(nether) {};
  void draw();
  int needsRedraw;

private:
  const NETHER* nether;
};

#endif // RADAR_H
