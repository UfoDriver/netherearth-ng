#ifndef RADAR_H
#define RADAR_H

class NETHER;


class Radar {
public:
  explicit Radar(const NETHER *nether) : nether {nether} {};
  void draw(const int width, const int height, const int split, const int splity);

private:
  const NETHER* nether;
};

#endif // RADAR_H
