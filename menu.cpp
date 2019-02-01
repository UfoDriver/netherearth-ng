#ifdef _WIN32
#include "windows.h"
#endif

#include "stdio.h"
#include "string.h"
#include <iostream>

#include "GL/gl.h"
#include "GL/glu.h"
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"

#include "list.h"
#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "nether.h"
#include "statusbutton.h"
#include "menu.h"
#include "nether.h"

void Menu::newmenu(MENU_TYPES menu)
{
  switch(menu) {
  case GENERAL_MENU:
    if (getbutton(StatusButton::TIME_BUTTON)==0) {
      newbutton(StatusButton::TIME_BUTTON,70,455,130,40,"Day: 0","Time: 00:00",0.8f,0,0);
    }
    newbuttondelayed(StatusButton::STATUS_BUTTON,70,400,130,50,"STATUS","INSG  HUMN",0.0,0,0);
    newbuttondelayed(StatusButton::RESOURCE_BUTTON,70,200,130,30,"RESOURCES", "",0.0,0,0);
    act_menu=GENERAL_MENU;
    break;

  case ROBOT_MENU:
    newbuttondelayed(StatusButton::ROBOT1_BUTTON,70,350,130,40,"DIRECT   ","  CONTROL",0,0,0.8f);
    newbuttondelayed(StatusButton::ROBOT2_BUTTON,70,300,130,40,"GIVE     ","   ORDERS",0,0,0.8f);
    newbuttondelayed(StatusButton::ROBOT3_BUTTON,70,250,130,40,"COMBAT   ","     MODE",0,0,0.8f);
    newbuttondelayed(StatusButton::ROBOT4_BUTTON,70,200,130,40,"LEAVE    ","    ROBOT",0,0,0.8f);
    act_menu=ROBOT_MENU;
    break;

  case COMBATMODE_MENU:
    newbuttondelayed(StatusButton::COMBAT1_BUTTON,70,350,130,40,"NUCLEAR  ","     BOMB",0,0,0.8f);
    newbuttondelayed(StatusButton::COMBAT2_BUTTON,70,300,130,40,"FIRE     ","  PHASERS",0,0,0.8f);
    newbuttondelayed(StatusButton::COMBAT3_BUTTON,70,250,130,40,"FIRE     "," MISSILES",0,0,0.8f);
    newbuttondelayed(StatusButton::COMBAT4_BUTTON,70,200,130,40,"FIRE     ","   CANNON",0,0,0.8f);
    newbuttondelayed(StatusButton::COMBAT5_BUTTON,70,150,130,40,"MOVE     ","    ROBOT",0,0,0.8f);
    newbuttondelayed(StatusButton::COMBAT6_BUTTON,70,100,130,40,"STOP     ","   COMBAT",0,0,0.8f);
    act_menu=COMBATMODE_MENU;
    break;

  case ORDERS_MENU:
    newbuttondelayed(StatusButton::ORDERS1_BUTTON,70,350,130,40,"STOP AND ","   DEFEND",0,0,0.8f);
    newbuttondelayed(StatusButton::ORDERS2_BUTTON,70,300,130,40,"ADVANCE  "," ?? MILES",0,0,0.8f);
    newbuttondelayed(StatusButton::ORDERS3_BUTTON,70,250,130,40,"RETREAT  "," ?? MILES",0,0,0.8f);
    newbuttondelayed(StatusButton::ORDERS4_BUTTON,70,200,130,40,"SEARCH & ","  DESTROY",0,0,0.8f);
    newbuttondelayed(StatusButton::ORDERS5_BUTTON,70,150,130,40,"SEARCH & ","  CAPTURE",0,0,0.8f);
    act_menu=ORDERS_MENU;
    break;

  case SELECTDISTANCE_MENU:
    if (nether->controlled->program==PROGRAM_ADVANCE)
      newbuttondelayed(StatusButton::ORDERS_BUTTON,70,400,130,40,"ADVANCE  "," ?? MILES",0,0,0.8f);
    if (nether->controlled->program==PROGRAM_RETREAT)
      newbuttondelayed(StatusButton::ORDERS_BUTTON,70,400,130,40,"RETREAT  "," ?? MILES",0,0,0.8f);
    act_menu=SELECTDISTANCE_MENU;
    break;

  case TARGETD_MENU:
    newbuttondelayed(StatusButton::ORDERS_BUTTON,70,400,130,40,"SEARCH & ","  DESTROY",0,0,0.8f);
    newbuttondelayed(StatusButton::TARGET1_BUTTON,70,300,130,40,"ENEMY    ","   ROBOTS",0,0,0.8f);
    newbuttondelayed(StatusButton::TARGET2_BUTTON,70,250,130,40,"ENEMY    ","FACTORIES",0,0,0.8f);
    newbuttondelayed(StatusButton::TARGET3_BUTTON,70,200,130,40,"ENEMY    "," WARBASES",0,0,0.8f);
    act_menu=TARGETD_MENU;
    break;

  case TARGETC_MENU:
    newbuttondelayed(StatusButton::ORDERS_BUTTON,70,400,130,40,"SEARCH & ","  CAPTURE",0,0,0.8f);
    newbuttondelayed(StatusButton::TARGET1_BUTTON,70,300,130,40,"NEUTRAL  ","FACTORIES",0,0,0.8f);
    newbuttondelayed(StatusButton::TARGET2_BUTTON,70,250,130,40,"ENEMY    ","FACTORIES",0,0,0.8f);
    newbuttondelayed(StatusButton::TARGET3_BUTTON,70,200,130,40,"ENEMY    "," WARBASES",0,0,0.8f);
    act_menu=TARGETC_MENU;
    break;
  }
  redrawmenu=true;
}


void Menu::killmenu(MENU_TYPES menu)
{
  switch(menu) {
  case GENERAL_MENU:
    killbutton(StatusButton::STATUS_BUTTON);
    killbutton(StatusButton::RESOURCE_BUTTON);
    break;

  case ROBOT_MENU:
    killbutton(StatusButton::ROBOT1_BUTTON);
    killbutton(StatusButton::ROBOT2_BUTTON);
    killbutton(StatusButton::ROBOT3_BUTTON);
    killbutton(StatusButton::ROBOT4_BUTTON);
    break;

  case COMBATMODE_MENU:
    killbutton(StatusButton::COMBAT1_BUTTON);
    killbutton(StatusButton::COMBAT2_BUTTON);
    killbutton(StatusButton::COMBAT3_BUTTON);
    killbutton(StatusButton::COMBAT4_BUTTON);
    killbutton(StatusButton::COMBAT5_BUTTON);
    killbutton(StatusButton::COMBAT6_BUTTON);
    break;

  case ORDERS_MENU:
    killbutton(StatusButton::ORDERS1_BUTTON);
    killbutton(StatusButton::ORDERS2_BUTTON);
    killbutton(StatusButton::ORDERS3_BUTTON);
    killbutton(StatusButton::ORDERS4_BUTTON);
    killbutton(StatusButton::ORDERS5_BUTTON);
    break;

  case SELECTDISTANCE_MENU:
    killbutton(StatusButton::ORDERS_BUTTON);
    break;

  case TARGETD_MENU:
  case TARGETC_MENU:
    killbutton(StatusButton::ORDERS_BUTTON);
    killbutton(StatusButton::TARGET1_BUTTON);
    killbutton(StatusButton::TARGET2_BUTTON);
    killbutton(StatusButton::TARGET3_BUTTON);
    break;

  case ALL_MENUS:
    killbutton(StatusButton::STATUS_BUTTON);
    killbutton(StatusButton::RESOURCE_BUTTON);
    killbutton(StatusButton::ROBOT1_BUTTON);
    killbutton(StatusButton::ROBOT2_BUTTON);
    killbutton(StatusButton::ROBOT3_BUTTON);
    killbutton(StatusButton::ROBOT4_BUTTON);
    killbutton(StatusButton::COMBAT1_BUTTON);
    killbutton(StatusButton::COMBAT2_BUTTON);
    killbutton(StatusButton::COMBAT3_BUTTON);
    killbutton(StatusButton::COMBAT4_BUTTON);
    killbutton(StatusButton::COMBAT5_BUTTON);
    killbutton(StatusButton::COMBAT6_BUTTON);
    killbutton(StatusButton::ORDERS1_BUTTON);
    killbutton(StatusButton::ORDERS2_BUTTON);
    killbutton(StatusButton::ORDERS3_BUTTON);
    killbutton(StatusButton::ORDERS4_BUTTON);
    killbutton(StatusButton::ORDERS5_BUTTON);
    killbutton(StatusButton::ORDERS_BUTTON);
    killbutton(StatusButton::ORDERS_BUTTON);
    killbutton(StatusButton::TARGET1_BUTTON);
    killbutton(StatusButton::TARGET2_BUTTON);
    killbutton(StatusButton::TARGET3_BUTTON);
    break;
  }
  redrawmenu=true;
}


void Menu::newbutton(StatusButton::BUTTON_NAMES ID, int x, int y, int sx, int sy,
const std::string& t1, const std::string& t2, float r, float g, float b)
{
  buttons.Add(new StatusButton(ID, x, y, sx, sy, t1, t2, r, g, b, -16));
  redrawmenu = true;
}


void Menu::newbuttondelayed(StatusButton::BUTTON_NAMES ID, int x, int y, int sx, int sy,
const std::string& t1, const std::string& t2, float r, float g, float b)
{
  buttons.Add(new StatusButton(ID, x, y, sx, sy, t1, t2, r, g, b, -32));
  redrawmenu = true;
}


void Menu::killbutton(StatusButton::BUTTON_NAMES ID)
{
  List<StatusButton> l;
  StatusButton *b;

  l.Instance(buttons);
  l.Rewind();
  while(l.Iterate(b)) {
    if (b->ID == ID) b->status = 1;
  }
  redrawmenu=true;
}


StatusButton* Menu::getbutton(StatusButton::BUTTON_NAMES ID)
{
  List<StatusButton> l;
  StatusButton *b;

  l.Instance(buttons);
  l.Rewind();
  while(l.Iterate(b)) {
    if (b->ID == ID) return b;
  }

  return 0;
}


void Menu::replaceMenu(MENU_TYPES oldMenu, MENU_TYPES newMenu,
                       StatusButton::BUTTON_NAMES activeButton)
{
  killmenu(oldMenu);
  newmenu(newMenu);
  act_button = activeButton;
}
