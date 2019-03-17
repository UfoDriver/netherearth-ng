#ifndef STATUSBUTTON_H
#define STATUSBUTTON_H

#include <string>
#include "color.h"


class StatusButton {
public:
  enum class NAME {NONE,
                   TIME,
                   STATUS,
                   RESOURCE,
                   ROBOT1,
                   ROBOT2,
                   ROBOT3,
                   ROBOT4,
                   COMBAT1,
                   COMBAT2,
                   COMBAT3,
                   COMBAT4,
                   COMBAT5,
                   COMBAT6,
                   ORDERS1,
                   ORDERS2,
                   ORDERS3,
                   ORDERS4,
                   ORDERS5,
                   ORDERS,
                   ORDERS11,
                   ORDERS12,
                   ORDERS13,
                   TARGET1,
                   TARGET2,
                   TARGET3,
                   TARGET11,
                   TARGET21,
                   TARGET31};

  StatusButton(NAME id, int x, int y, int sx, int sy, const std::string &text1,
               const std::string &text2, const Color &color, int status = -100)
    : id {id}, x {x}, y {y}, sx {sx}, sy {sy}, text1 {text1}, text2 {text2}, color {color},
      status {status} {}

  bool isInteractive()
  {
    return id != NAME::TIME and id != NAME::STATUS and id != NAME::RESOURCE and id != NAME::ORDERS
      and id != NAME::ORDERS13 and status >= -32;
  }
  bool visible()
  {
    return status == 0;
  }
  void draw();
  void cycle();
  void toggle() {
    color = {1.0f, 0.5f, 0.5f};
  }
  void untoggle() {
    color = {0.5f, 0.5f, 1.0f};
  }

  NAME id;
  int x, y;
  int sx, sy;
  std::string text1;
  std::string text2;
  Color color;
  int status;
};

#endif // STATUSBUTTON_H
