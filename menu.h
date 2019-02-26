#ifndef MENU_H
#define MENU_H

#include <vector>

#include "statusbutton.h"


class NETHER;
class Stats;


class Menu
{
public:
  enum class TYPE {GENERAL,
                   ROBOT,
                   DIRECTCONTROL,
                   COMBATMODE,
                   DIRECTCONTROL2,
                   ORDERS,
                   SELECTDISTANCE,
                   TARGET_DESTROY,
                   TARGET_CAPTURE,
                   ALL};
  explicit Menu(const NETHER* nether): needsRedraw(0), act_menu(TYPE::GENERAL),
                                       act_button(StatusButton::NAME::COMBAT1),
                                       nether(nether) {};
  void draw(int width, int height);
  void requestRedraw() { needsRedraw = 2; }
  void cycle();
  void replaceMenu(TYPE oldMenu, TYPE newMenu, StatusButton::NAME activeButton);
  void newmenu(TYPE menu);
  void killmenu(TYPE menu);

  void newbutton(StatusButton::NAME ID, int x, int y, int sx, int sy,
                 const std::string& t1, const std::string& t2, const Color& color);
  void newbuttondelayed(StatusButton::NAME ID, int x, int y, int sx, int sy,
                        const std::string& t1, const std::string& t2, const Color& color);
  void killbutton(StatusButton::NAME ID);
  StatusButton *getbutton(StatusButton::NAME ID);

  bool handleKeys(unsigned char* keyboard);
  void updateTime(const Stats& stats);

  int needsRedraw;
  TYPE act_menu;
  StatusButton::NAME act_button;

private:
  void drawButtons();
  void drawStatus();
  const NETHER* nether;
  std::vector<StatusButton *> buttons;
};

#endif // MENU_H
