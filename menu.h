#ifndef MENU_H
#define MENU_H

#include <list>

#include "statusbutton.h"


class NETHER;


class Menu
{
public:
  enum MENU_TYPES {GENERAL_MENU,
                   ROBOT_MENU,
                   DIRECTCONTROL_MENU,
                   COMBATMODE_MENU,
                   DIRECTCONTROL2_MENU,
                   ORDERS_MENU,
                   SELECTDISTANCE_MENU,
                   TARGETD_MENU,
                   TARGETC_MENU,
                   ALL_MENUS};
  explicit Menu(const NETHER* nether): needsRedraw(0), act_menu(GENERAL_MENU),
                                       act_button(StatusButton::COMBAT1_BUTTON),
                                       nether(nether) {};
  void draw(int width, int height);
  void cycle();
  void replaceMenu(MENU_TYPES oldMenu, MENU_TYPES newMenu, StatusButton::BUTTON_NAMES activeButton);
  void newmenu(MENU_TYPES menu);
  void killmenu(MENU_TYPES menu);

  void newbutton(StatusButton::BUTTON_NAMES ID, int x, int y, int sx, int sy,
                 const std::string& t1, const std::string& t2, float r, float g, float b);
  void newbuttondelayed(StatusButton::BUTTON_NAMES ID, int x, int y, int sx, int sy,
                        const std::string& t1, const std::string& t2, float r, float g, float b);
  void killbutton(StatusButton::BUTTON_NAMES ID);
  StatusButton *getbutton(StatusButton::BUTTON_NAMES ID);

  int needsRedraw;
  MENU_TYPES act_menu;
  StatusButton::BUTTON_NAMES act_button;

private:
  void drawButtons();
  void drawStatus();
  const NETHER* nether;
  std::list<StatusButton *> buttons;
};

#endif // MENU_H
