#ifndef MAINMENU_H
#define MAINMENU_H

#include <SDL/SDL.h>
#include <string>
#include <vector>

#include "config.h"

class C3DObject;


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
  std::vector<std::string> mapnames;
  std::vector<std::string>::iterator mapnameIter {mapnames.begin()};

  Config config;
};

#endif // MAINMENU_H
