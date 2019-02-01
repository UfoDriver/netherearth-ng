#ifndef STATUSBUTTON_H
#define STATUSBUTTON_H

#include <string>


class StatusButton {
public:
  enum BUTTON_NAMES {TIME_BUTTON = 1,
                   STATUS_BUTTON,
                   RESOURCE_BUTTON,
                   ROBOT1_BUTTON,
                   ROBOT2_BUTTON,
                   ROBOT3_BUTTON,
                   ROBOT4_BUTTON,
                   COMBAT1_BUTTON,
                   COMBAT2_BUTTON,
                   COMBAT3_BUTTON,
                   COMBAT4_BUTTON,
                   COMBAT5_BUTTON,
                   COMBAT6_BUTTON,
                   ORDERS1_BUTTON,
                   ORDERS2_BUTTON,
                   ORDERS3_BUTTON,
                   ORDERS4_BUTTON,
                   ORDERS5_BUTTON,
                   ORDERS_BUTTON,
                   TARGET1_BUTTON,
                   TARGET2_BUTTON,
                   TARGET3_BUTTON};

  StatusButton(BUTTON_NAMES ID, int x, int y, int sx, int sy, const std::string& text1,
               const std::string& text2, float r, float g, float b, int status):
    ID(ID), x(x), y(y), sx(sx), sy(sy), text1(text1), text2(text2), r(r), g(g), b(b), status(status)
  {}

  BUTTON_NAMES ID;
  int x, y;
  int sx, sy;
  std::string text1;
  std::string text2;
  float r, g, b;
  int status;
};

#endif // STATUSBUTTON_H
