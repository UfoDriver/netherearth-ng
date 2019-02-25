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
                   TARGET1,
                   TARGET2,
                   TARGET3};

  StatusButton(NAME ID, int x, int y, int sx, int sy, const std::string& text1,
               const std::string& text2, const Color& color, int status):
    ID(ID), x(x), y(y), sx(sx), sy(sy), text1(text1), text2(text2), color{color}, status(status)
  {}

  bool isInteractive()
  {
    return ID != NAME::TIME && ID != NAME::STATUS and ID != NAME::RESOURCE;
  }

  NAME ID;
  int x, y;
  int sx, sy;
  std::string text1;
  std::string text2;
  Color color;
  int status;
};

#endif // STATUSBUTTON_H
