#ifndef MAINMENU_H
#define MAINMENU_H

#include <SDL/SDL.h>
#include <string>
#include <vector>

#include "config.h"

class C3DObject;


static std::vector<int> COLOR_DEPTHS {8, 16, 24, 32};
static std::vector<std::pair<int, int>> SCREEN_SIZES {
  {320, 240},
  {400, 300},
  {640, 480},
  {800, 600},
  {1024, 768},
  {1280, 1024},
  {1366, 768},
  {1600, 900},
  {1920, 1080},
  {2560, 1440},
  {3440, 1440},
  {3840, 2160}
};


class MainMenu
{
public:
  enum class ACTION {
    NONE,
    START,
    RESTART_VIDEO,
    QUIT,
    SET_FULLSCREEN,
    SET_SOUND,
  };

  MainMenu(Config& config);
  ACTION cycle(int width, int height);
  void draw(int width, int height);
  void refreshDisplayLists();
  std::string getMapPath();
  void reset()
  {
    status = 0;
    substatus = 0;
  }

private:
  int status {0};
  int substatus {0};
  C3DObject* title = 0;
  unsigned char old_keyboard[SDLK_LAST];
  void populateMaps();
  std::vector<std::string> mapnames {"original"};
  std::vector<std::string>::iterator mapnameIter {mapnames.begin()};

  // CircularIterator<std::vector<std::string>::iterator> mapnameIter {mapnames.begin(), mapnames.end()};
  // CircularIterator<std::vector<int>::iterator> colorDepths {COLOR_DEPTHS.begin(), COLOR_DEPTHS.end()};

  Config& config;
};

#endif // MAINMENU_H
