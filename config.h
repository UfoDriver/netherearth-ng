#ifndef CONFIG_H
#define CONFIG_H

#include <string>


struct Config {
  int screenX;
  int screenY;
  int colorDepth;
  int shadows;
  bool sound;
  int level;
  int keyUp, keyDown, keyLeft, keyRight, keyFire, keyPause;

  bool fullscreenMode;
  bool showRadar;
  std::string mapname;

  void load();
  void save();
};

#endif // CONFIG_H
