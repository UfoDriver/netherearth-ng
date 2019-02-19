#ifndef OPTIONSSCREEN_H
#define OPTIONSSCREEN_H

class Light;
class NETHER;


class OptionsScreen
{
public:
  explicit OptionsScreen(NETHER* nether): selectedOption(0), nether(nether) {}

  void draw(int w, int h, const Light& light);
  bool cycle(unsigned char *keyboard);
  void open();


private:
  int selectedOption;
  NETHER* nether;
};

#endif // OPTIONSSCREEN_H
