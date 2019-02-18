#ifndef OPTIONSSCREEN_H
#define OPTIONSSCREEN_H

class NETHER;


class OptionsScreen
{
public:
  explicit OptionsScreen(NETHER* nether): nether(nether) {}

  void draw(int w, int h, const float lightpos[4]);
  bool cycle(unsigned char *keyboard);
  void open();


private:
  int selectedOption;
  NETHER* nether;
};

#endif // OPTIONSSCREEN_H
