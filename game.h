#ifndef GAME_H
#define GAME_H

#include <SDL/SDL.h>
#include <SDL/SDL_video.h>

#include "mainmenu.h"
#include "nether.h"


const int REDRAWING_PERIOD = 20;


class Game
{
public:
  Game();
  ~Game();

  void run();

private:
  // Game config variables
  Config config;

  MainMenu mainMenu;
  SDL_Surface* sdl_surface {nullptr};
  NETHER* nether {nullptr};
  bool initFailed {false};

  bool processSDLEvents();
  void pause(unsigned int time);
  void toggleFullscreen();
  bool restartVideo();
  bool processMenu();

  void loadConfiguration();
  void saveConfiguration();

  // Drawing housekeeping variables
  int framesPerSec = 0;
  int framesPerSecTmp = 0;
  int initTime = 0;
};

#endif // GAME_H
