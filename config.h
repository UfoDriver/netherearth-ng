#ifndef CONFIG_H
#define CONFIG_H

#include <string>

#include <SDL/SDL.h>


struct Config {
  int screenX {320};
  int screenY {240};
  int colorDepth {8};
  int shadows {0};
  bool sound {false};
  int level {1};
  int keyUp {SDLK_UP},
    keyDown {SDLK_DOWN},
    keyLeft {SDLK_LEFT},
    keyRight {SDLK_RIGHT},
    keyFire {SDLK_z},
    keyPause {SDLK_h};

  bool fullscreenMode {false};
  bool showRadar {true};
  std::string mapname {"original"};

  void load();
  void save();
};

#endif // CONFIG_H