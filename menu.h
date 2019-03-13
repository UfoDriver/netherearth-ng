#ifndef MENU_H
#define MENU_H

#include <iostream>
#include <memory>
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

  explicit Menu(const NETHER* nether): nether(nether) {};

  void draw(int width, int height);
  void requestRedraw() { needsRedraw = 2; }
  void cycle();
  void activateMenu(TYPE newMenu, StatusButton::NAME activeButton);
  void setActiveButtonColor(const Color& color);
  bool handleKeys(unsigned char* keyboard);
  void updateTime(const Stats& stats);

  StatusButton::NAME getActiveButton() const { return activeButton; }
  TYPE getActiveMenu() const { return activeMenu; }
  void setActiveMenu(TYPE newActiveMenu) { activeMenu = newActiveMenu; }

private:
  StatusButton *getbutton(StatusButton::NAME ID);
  void drawStatus();
  void newbutton(StatusButton::NAME ID, int x, int y, int sx, int sy,
                 const std::string& t1, const std::string& t2, const Color& color);
  void newbuttondelayed(StatusButton::NAME ID, int x, int y, int sx, int sy,
                        const std::string& t1, const std::string& t2, const Color& color);
  void killbutton(StatusButton::NAME ID);
  void newmenu(TYPE menu);
  void killmenu(TYPE menu);

  TYPE activeMenu {TYPE::GENERAL};
  StatusButton::NAME activeButton {StatusButton::NAME::COMBAT1};
  int needsRedraw {0};
  const NETHER* nether;
  std::vector<std::unique_ptr<StatusButton>> buttons;

  friend std::ostream& operator<<(std::ostream& out, const Menu& menu);
  friend std::istream& operator>>(std::istream& in, Menu& menu);
};

#endif // MENU_H
