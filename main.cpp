#include <memory>
#ifdef _WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include <iostream>
#include <memory.h>
#include <string>

#include <GL/glut.h>

#include "game.h"



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







#ifdef _WIN32
int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)
{
#else
int main(int argc, char** argv)
{
#endif
  glutInit(&argc, argv);

  Game game;
  game.run();

  return 0;
}
