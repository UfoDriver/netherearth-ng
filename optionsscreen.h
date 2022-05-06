#ifndef OPTIONSSCREEN_H
#define OPTIONSSCREEN_H

#include <string>

class Config;
class Light;
class NETHER;


class OptionsScreen
{
public:
  explicit OptionsScreen(NETHER* nether): selectedOption(0), nether(nether) {}

  void draw(int w, int h, const Light& light);
  bool cycle(const Config& config, unsigned char *keyboard);
  void open();

private:
  int selectedOption;
  NETHER* nether;

  std::string generateFilename(int number) const;
};

#endif // OPTIONSSCREEN_H
