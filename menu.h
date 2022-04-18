#ifndef MENU_H
#define MENU_H

#include <iostream>
#include <memory>
#include <vector>
#include <unordered_set>

#include <sexp/value.hpp>

#include "statusbutton.h"


class NETHER;
class Stats;

const std::initializer_list<StatusButton> MENU
  {
     {StatusButton::NAME::TIME,     70, 455, 130, 40, "Day: 0",    "Time: 00:00", Color(0.8f, 0, 0)},
     {StatusButton::NAME::STATUS,   70, 400, 130, 50, "STATUS",    "INSG  HUMN",  Color(0, 0, 0.0f)},
     {StatusButton::NAME::RESOURCE, 70, 200, 130, 30, "RESOURCES", "",            Color(0, 0, 0.0f)},
     {StatusButton::NAME::ROBOT1,   70, 350, 130, 40, "DIRECT   ", "  CONTROL",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ROBOT2,   70, 300, 130, 40, "GIVE     ", "   ORDERS",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ROBOT3,   70, 250, 130, 40, "COMBAT   ", "     MODE",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ROBOT4,   70, 200, 130, 40, "LEAVE    ", "    ROBOT",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::COMBAT1,  70, 350, 130, 40, "NUCLEAR  ", "     BOMB",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::COMBAT2,  70, 300, 130, 40, "FIRE     ", "  PHASERS",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::COMBAT3,  70, 250, 130, 40, "FIRE     ", " MISSILES",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::COMBAT4,  70, 200, 130, 40, "FIRE     ", "   CANNON",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::COMBAT5,  70, 150, 130, 40, "MOVE     ", "    ROBOT",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::COMBAT6,  70, 100, 130, 40, "STOP     ", "   COMBAT",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ORDERS1,  70, 350, 130, 40, "STOP AND ", "   DEFEND",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ORDERS2,  70, 300, 130, 40, "ADVANCE  ", " ?? MILES",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ORDERS3,  70, 250, 130, 40, "RETREAT  ", " ?? MILES",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ORDERS4,  70, 200, 130, 40, "SEARCH & ", "  DESTROY",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ORDERS5,  70, 150, 130, 40, "SEARCH & ", "  CAPTURE",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ORDERS,   70, 400, 130, 40, "SEARCH & ", "  CAPTURE",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ORDERS11, 70, 400, 130, 40, "ADVANCE  ", " ?? MILES",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ORDERS12, 70, 400, 130, 40, "RETREAT  ", " ?? MILES",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::ORDERS13, 70, 400, 130, 40, "SEARCH & ", "  DESTROY",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::TARGET1,  70, 300, 130, 40, "ENEMY    ", "   ROBOTS",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::TARGET2,  70, 250, 130, 40, "ENEMY    ", "FACTORIES",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::TARGET3,  70, 200, 130, 40, "ENEMY    ", " WARBASES",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::TARGET11, 70, 300, 130, 40, "NEUTRAL  ", "FACTORIES",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::TARGET21, 70, 250, 130, 40, "ENEMY    ", "FACTORIES",   Color(0, 0, 0.8f)},
     {StatusButton::NAME::TARGET31, 70, 200, 130, 40, "ENEMY    ", " WARBASES",   Color(0, 0, 0.8f)}
  };


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

  explicit Menu(NETHER *nether) : nether{nether} {};

  void draw(int width, int height);
  void cycle(unsigned char *keyboard);
  bool handleKeys(unsigned char *keyboard);
  void updateTime(const Stats &stats);

  void activateMenu(TYPE newMenu, StatusButton::NAME activeButton);
  StatusButton::NAME getActiveButton() const { return activeButton; }
  TYPE getActiveMenu() const { return activeMenu; }

  sexp::Value toSexp() const;
  bool fromSexp(const sexp::Value&);

private:
  void drawStatus();
  void showMenu(TYPE menu);
  void hideMenu();
  void hideButtons(const std::unordered_set<StatusButton::NAME>& ids);
  void showButtons(const std::unordered_set<StatusButton::NAME>& ids);
  StatusButton& findButton(StatusButton::NAME id);

  TYPE activeMenu {TYPE::GENERAL};
  StatusButton::NAME activeButton {StatusButton::NAME::COMBAT1};
  NETHER *nether;
  std::vector<StatusButton> buttons {MENU};

  friend std::ostream &operator<<(std::ostream &out, const Menu &menu);
  friend std::istream &operator>>(std::istream &in, Menu &menu);
};

#endif // MENU_H
