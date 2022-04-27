#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include <iostream>
#include <memory.h>
#include <string>

#include <GL/gl.h>
#include <GL/glut.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>

#include "mainmenu.h"
#include "nether.h"


int SCREEN_X = 640;
int SCREEN_Y = 480;

int COLOUR_DEPTH = 32;
int shadows = 0;
bool sound = true;
int up_key = SDLK_q, down_key = SDLK_a, left_key = SDLK_o, right_key = SDLK_p,
  fire_key = SDLK_SPACE, pause_key = SDLK_F1;
int level = 1;
bool fullscreen = false;
bool show_radar = true;

/* Redrawing constant: */
const int REDRAWING_PERIOD = 20;

/* Frames per second counter: */
int frames_per_sec = 0;
int frames_per_sec_tmp = 0;
int init_time = 0;

void loadConfiguration(void);

/* AUXILIAR FUNCTION DEFINITION: */


void pause(unsigned int time)
{
  unsigned int initt = SDL_GetTicks();

  while((SDL_GetTicks() - initt) < time);
} /* pause */


SDL_Surface *initialization(int flags)
{
  const SDL_VideoInfo* info = 0;
  int bpp = 0;
  SDL_Surface *screen;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
    std::cerr << "Video initialization failed: " << SDL_GetError() << std::endl;
    return 0;
  }

  info = SDL_GetVideoInfo();

  if (!info) {
    std::cerr << "Video query failed: " << SDL_GetError() << std::endl;
    return 0;
  }

  if (fullscreen) {
    bpp = COLOUR_DEPTH;
  } else {
    bpp = info->vfmt->BitsPerPixel;
  }

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

  flags = SDL_OPENGL | flags;

  screen = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, bpp, flags);
  if (screen == 0) {
    std::cerr << "Video mode set failed: " << SDL_GetError() << std::endl;
    return 0;
  }

  pause(400);
  if (Mix_OpenAudio(22050, AUDIO_S16, 2, 1024)) {
    return 0;
  }

  SDL_WM_SetCaption("Nether Earth REMAKE v0.51", 0);
  SDL_ShowCursor(SDL_DISABLE);

  return screen;
}


void finalize()
{
  Mix_CloseAudio();
  SDL_Quit();
}


#ifdef _WIN32
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
#else
int main(int argc, char** argv)
{
#endif
  MainMenu mainMenu;
  SDL_Surface* screen_sfc = initialization((fullscreen ? SDL_FULLSCREEN : 0));
  if (!screen_sfc) {
    std::cerr << "Cannot create SDL surface" << std::endl;
    return 0;
  }

  NETHER* game {nullptr};

  glutInit(&argc, argv);

  int time = init_time = SDL_GetTicks();
  bool quit = false;

  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN:
        if (event.key.keysym.sym == SDLK_F12) {
          quit = true;
          break;
        }
        if (event.key.keysym.sym == SDLK_RETURN && (SDL_GetModState() & KMOD_ALT) != 0) {
          /* Toggle FULLSCREEN mode: */
          if (game) {
            Resources::instance()->refreshDisplayLists();
            game->scene.ship.refresh_display_lists();
          }
          mainMenu.refreshDisplayLists();
          if (game) Resources::instance()->deleteObjects();
          fullscreen = !fullscreen;
          SDL_QuitSubSystem(SDL_INIT_VIDEO);
          SDL_InitSubSystem(SDL_INIT_VIDEO);
          if (SDL_WasInit(SDL_INIT_VIDEO)) {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

            screen_sfc = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, COLOUR_DEPTH,
                                          SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0));
            if (game) Resources::instance()->loadObjects();
            SDL_WM_SetCaption("Nether Earth REMAKE v0.5", 0);
            SDL_ShowCursor(SDL_DISABLE);
          } else {
            quit = true;
          }
        }
        break;

      case SDL_QUIT:
        quit = true;
        break;
      }
    }

    int act_time = SDL_GetTicks();
    if (act_time-time >= REDRAWING_PERIOD) {
      frames_per_sec_tmp += 1;
      if ((act_time-init_time) >= 1000) {
        frames_per_sec = frames_per_sec_tmp;
        frames_per_sec_tmp = 0;
        init_time = act_time;
      }

      do {
        time += REDRAWING_PERIOD;
        if ((act_time-time) > 50 * REDRAWING_PERIOD)time = act_time;

        if (game) {
          if (!game->gamecycle()) {
            delete game;
            game = nullptr;
            mainMenu.reset();
          }
        } else {
          MainMenu::ACTION val = mainMenu.cycle(SCREEN_X, SCREEN_Y);
          if (val == MainMenu::ACTION::START) {
            std::cerr << "Creating instance of NETHER" << std::endl;
            if (game) delete game;
            game = new NETHER(mainMenu.getMapPath());
          }
          if (val == MainMenu::ACTION::QUIT) quit = true;
          if (val == MainMenu::ACTION::RESTARTVIDEO) {
            if (game) {
              Resources::instance()->refreshDisplayLists();
              game->scene.ship.refresh_display_lists();
            }
            mainMenu.refreshDisplayLists();
            if (game) Resources::instance()->deleteObjects();
            SDL_QuitSubSystem(SDL_INIT_VIDEO);
            SDL_InitSubSystem(SDL_INIT_VIDEO);
            if (SDL_WasInit(SDL_INIT_VIDEO)) {
              SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
              SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
              SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
              SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
              SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
              SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 1);

              screen_sfc = SDL_SetVideoMode(SCREEN_X, SCREEN_Y, COLOUR_DEPTH,
                                            SDL_OPENGL | (fullscreen ? SDL_FULLSCREEN : 0));
              if (game) Resources::instance()->loadObjects();
              SDL_WM_SetCaption("Nether Earth REMAKE v0.5", 0);
              SDL_ShowCursor(SDL_DISABLE);
            } else {
              quit = true;
            }
          }
        }
        act_time = SDL_GetTicks();
      } while (act_time-time >= REDRAWING_PERIOD);

      if (game) {
        game->gameredraw(SCREEN_X, SCREEN_Y);
      } else {
        mainMenu.draw(SCREEN_X, SCREEN_Y);
      }
    }
  }

  if (game) delete game;
  finalize();

  return 0;
}
