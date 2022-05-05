#include "config.h"

#include <iostream>
#include <fstream>

#include <sexp/io.hpp>
#include <sexp/parser.hpp>
#include <sexp/util.hpp>
#include <sexp/value.hpp>


void Config::load()
{
  std::ifstream configFile("nether.cfg", std::ios::binary);
  sexp::Value config = sexp::Parser::from_stream(configFile);
  if (config.get_car().as_string() != "game-config") {
    std::cerr << "nether.cfg does not look like a game-config" << std::endl;
    return;
  }

  for (const sexp::Value& section: sexp::ListAdapter(config.get_cdr())) {
    if (section.get_car().as_string() == "screen") {
      screenX = sexp::list_ref(section, 1).as_int();
      screenY = sexp::list_ref(section, 2).as_int();
      colorDepth = sexp::list_ref(section, 3).as_int();
      fullscreenMode = sexp::list_ref(section, 4).as_bool();
    } else if (section.get_car().as_string() == "options") {
      sound = sexp::list_ref(section, 1).as_bool();
      level = sexp::list_ref(section, 2).as_int();
      shadows = sexp::list_ref(section, 3).as_int();
      fullscreenMode = sexp::list_ref(section, 4).as_bool();
      mapname = sexp::list_ref(section, 5).as_string();
    } else if (section.get_car().as_string() == "keys") {
      keyUp = sexp::list_ref(section, 1).as_int();
      keyDown = sexp::list_ref(section, 2).as_int();
      keyLeft = sexp::list_ref(section, 3).as_int();
      keyRight = sexp::list_ref(section, 4).as_int();
      keyFire = sexp::list_ref(section, 5).as_int();
      keyPause = sexp::list_ref(section, 6).as_int();
    } else {
      std::cerr << "Unknown section section " << section.get_car().as_string() << std::endl;
    }
  }
}


void Config::save()
{
  std::ofstream("nether.cfg")
    << sexp::Value::list(
      sexp::Value::symbol("game-config"),
      sexp::Value::list(
        sexp::Value::symbol("screen"),
        sexp::Value::integer(screenX),
        sexp::Value::integer(screenY),
        sexp::Value::integer(colorDepth),
        sexp::Value::boolean(fullscreenMode)
      ),
      sexp::Value::list(
        sexp::Value::symbol("options"),
        sexp::Value::boolean(sound),
        sexp::Value::integer(level),
        sexp::Value::integer(shadows),
        sexp::Value::boolean(fullscreenMode),
        sexp::Value::string(mapname)
      ),
      sexp::Value::list(
        sexp::Value::symbol("keys"),
        sexp::Value::integer(keyUp),
        sexp::Value::integer(keyDown),
        sexp::Value::integer(keyLeft),
        sexp::Value::integer(keyRight),
        sexp::Value::integer(keyFire),
        sexp::Value::integer(keyPause)
      )
    );
}
