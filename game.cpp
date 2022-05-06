#include <iostream>
#include <iterator>

#include <GL/gl.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include <sexp/io.hpp>
#include <sexp/parser.hpp>
#include <sexp/value.hpp>
#include <sexp/util.hpp>

#include "game.h"
#include "mainmenu.h"


Game::Game()
  : mainMenu{config}
{
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cerr << "Video initialization failed: " << SDL_GetError() << std::endl;
    initFailed = true;
    return;
  }

  const SDL_VideoInfo* info {SDL_GetVideoInfo()};

  if (!info) {
    std::cerr << "Video query failed: " << SDL_GetError() << std::endl;
    initFailed = true;
    return;
  }

  int bpp {config.fullscreenMode ? config.colorDepth : info->vfmt->BitsPerPixel};

  // modified
  // SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  // SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  // SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  // SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
  // SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  // SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 0);


  int flags = config.fullscreenMode ? SDL_FULLSCREEN : 0;
  flags = SDL_OPENGL | flags;

  sdl_surface = SDL_SetVideoMode(config.screenX, config.screenY, bpp, flags);
  if (!sdl_surface) {
    std::cerr << "Video mode set failed: " << SDL_GetError() << std::endl;
    initFailed = true;
    return;
  }

  pause(400);
  if (Mix_OpenAudio(22050, AUDIO_S16, 2, 1024)) {
    std::cerr << "Cannot initialize audio" << std::endl;
    initFailed = true;
    return;
  }

  SDL_WM_SetCaption("Nether Earth REMAKE v0.51", 0);
  SDL_ShowCursor(SDL_DISABLE);
}


Game::~Game()
{
  if (nether) delete nether;
  Mix_CloseAudio();
  SDL_Quit();
}


void Game::run()
{
  if (initFailed) return;

  int time = initTime = SDL_GetTicks();
  bool quit = false;

  while (!quit) {
    quit = processSDLEvents();

    int act_time = SDL_GetTicks();
    if (act_time-time >= REDRAWING_PERIOD) {
      framesPerSecTmp += 1;
      if ((act_time-initTime) >= 1000) {
        framesPerSec = framesPerSecTmp;
        framesPerSecTmp = 0;
        initTime = act_time;
      }

      do {
        time += REDRAWING_PERIOD;
        if ((act_time-time) > 50 * REDRAWING_PERIOD)time = act_time;

        if (nether) {
          if (!nether->gamecycle()) {
            delete nether;
            nether = nullptr;
            mainMenu.reset();
          }
        } else {
          quit = !processMenu();
        }
        act_time = SDL_GetTicks();
      } while (act_time-time >= REDRAWING_PERIOD);

      if (nether) {
        nether->gameredraw(config.screenX, config.screenY);
      } else {
        mainMenu.draw(config.screenX, config.screenY);
      }
    }
  }
}


void Game::pause(unsigned int time)
{
  unsigned int initt = SDL_GetTicks();
  while((SDL_GetTicks() - initt) < time);
}


bool Game::processSDLEvents()
{
  SDL_Event event {};
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_KEYDOWN:
      if (event.key.keysym.sym == SDLK_F12) {
        return true;
      }
      if (event.key.keysym.sym == SDLK_RETURN && (SDL_GetModState() & KMOD_ALT) != 0) {
        toggleFullscreen();
      }
      break;

    case SDL_QUIT:
      return true;
    }
  }
  return false;
}


void Game::toggleFullscreen()
{
  if (nether) {
    Resources::instance()->refreshDisplayLists();
    nether->scene.ship.refresh_display_lists();
  }

  mainMenu.refreshDisplayLists();

  if (nether) Resources::instance()->deleteObjects();

  config.fullscreenMode = !config.fullscreenMode;
  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_InitSubSystem(SDL_INIT_VIDEO);

  if (SDL_WasInit(SDL_INIT_VIDEO)) {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

    sdl_surface = SDL_SetVideoMode(config.screenX, config.screenY, config.colorDepth,
                                  SDL_OPENGL | (config.fullscreenMode ? SDL_FULLSCREEN : 0));
    if (nether) Resources::instance()->loadObjects(config.shadows);
    SDL_WM_SetCaption("Nether Earth REMAKE v0.5", 0);
    SDL_ShowCursor(SDL_DISABLE);
  }
}


bool Game::restartVideo()
{
  // @TODO: chances that there is a lot of code duplication with fullScreen routine
  if (nether) {
    Resources::instance()->refreshDisplayLists();
    nether->scene.ship.refresh_display_lists();
  }

  mainMenu.refreshDisplayLists();
  if (nether) Resources::instance()->deleteObjects();

  SDL_QuitSubSystem(SDL_INIT_VIDEO);
  SDL_InitSubSystem(SDL_INIT_VIDEO);
  if (SDL_WasInit(SDL_INIT_VIDEO)) {
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

    sdl_surface = SDL_SetVideoMode(config.screenX, config.screenY, config.colorDepth,
                                   SDL_OPENGL | (config.fullscreenMode ? SDL_FULLSCREEN : 0));
    if (nether) Resources::instance()->loadObjects(config.shadows);
    SDL_WM_SetCaption("Nether Earth REMAKE v0.5", 0);
    SDL_ShowCursor(SDL_DISABLE);
    return true;
  } else {
    return false;
  }
}


bool Game::processMenu()
{
  switch (mainMenu.cycle(config.screenX, config.screenY)) {
  case MainMenu::ACTION::START:
    if (nether) delete nether;
    nether = new NETHER(mainMenu.getMapPath(), config);
    return true;
  case MainMenu::ACTION::QUIT:
    return false;
  case MainMenu::ACTION::RESTART_VIDEO:
    if (!restartVideo()) {
      std::cerr << "Failed to restart video" << std::endl;
      return false;
    }
  default:
    return true;
  }
}
