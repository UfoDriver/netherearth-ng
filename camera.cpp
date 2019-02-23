#include <SDL/SDL.h>
#include <algorithm>
#include "camera.h"


void Camera::cycle(unsigned char* keyboard)
{
  if (keyboard[SDLK_PAGEUP]) {
    zoom = std::min(zoom * 1.1, 4.0);
  }
  if (keyboard[SDLK_PAGEDOWN]) {
    zoom = std::max(zoom / 1.1, 0.5);
  }
}
