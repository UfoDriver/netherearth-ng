#ifndef CONFIG_H
#define CONFIG_H

#include <SDL/SDL_keysym.h>
#include <string>

#include <SDL/SDL.h>


struct Config {
  int screenX {320};
  int screenY {240};
  int colorDepth {8};
  int shadows {0};
  bool sound {false};
  int level {1};
  int keyUp {SDLK_q},
    keyDown {SDLK_a},
    keyLeft {SDLK_o},
    keyRight {SDLK_p},
    keyFire {SDLK_SPACE},
    keyPause {SDLK_ESCAPE};

  bool fullscreenMode {false};
  bool showRadar {true};
  std::string mapname {"original"};

  void load();
  void save();
};

#endif // CONFIG_H
