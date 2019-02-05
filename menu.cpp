#ifdef _WIN32
#include "windows.h"
#endif

#include "stdio.h"
#include <iostream>
#include <algorithm>

#include "GL/gl.h"
#include "GL/glu.h"
#include "SDL/SDL.h"
#include "SDL/SDL_mixer.h"
#include "myglutaux.h"
#include "glprintf.h"

#include "vector.h"
#include "cmc.h"
#include "3dobject.h"
#include "shadow3dobject.h"
#include "piece3dobject.h"
#include "nether.h"
#include "statusbutton.h"
#include "menu.h"
#include "nether.h"


void Menu::draw(int width, int height)
{
  if (needsRedraw != 0) {
    needsRedraw--;
    float lightpos2[4] = {0, 0, 1000, 0};
    int split = int((width * 25.0F) / 32.0F);

    glLightfv(GL_LIGHT0,GL_POSITION, lightpos2);
    glClearColor(0,0,0.2,0);
    glViewport(split,0,width-split,height);
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );
    glOrtho(0,float(width-split),0,height,-100,100);
    glScissor(split,0,width-split,height);
    glScalef(width/640.0,height/480.0,1);

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawButtons();
    drawStatus();
  }
}


void Menu::drawStatus()
{
  glPushMatrix();
  switch (act_menu) {
  case Menu::GENERAL_MENU:
    {
      StatusButton* b = getbutton(StatusButton::STATUS_BUTTON);
      if (b != 0 && b->status == 0) {
        int robots0 =  nether->robots[0].Length();
        int robots1 = nether->robots[1].Length();

        glColor3f(0.5f,0.5f,1.0f);
        glTranslatef(70,356,0);
        scaledglprintf(0.1f,0.1f,"%i WARBASES %i", nether->statistics[1][0], nether->statistics[0][0]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i ELECTR'S %i", nether->statistics[1][1], nether->statistics[0][1]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i NUCLEAR  %i", nether->statistics[1][2], nether->statistics[0][2]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i PHASERS  %i", nether->statistics[1][3], nether->statistics[0][3]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i MISSILES %i", nether->statistics[1][4], nether->statistics[0][4]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i  CANNON  %i", nether->statistics[1][5], nether->statistics[0][5]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i CHASSIS  %i", nether->statistics[1][6], nether->statistics[0][6]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%.2i ROBOTS %.2i", robots1, robots0);
        glColor3f(0.0f,0.8f,0.0f);
        glTranslatef(0,-65,0);
        scaledglprintf(0.1f,0.1f,"GENERAL %.2i", nether->resources[0][0]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"ELECTR' %.2i", nether->resources[0][1]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"NUCLEAR %.2i", nether->resources[0][2]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"PHASERS %.2i", nether->resources[0][3]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"MISSILE %.2i", nether->resources[0][4]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"CANNON  %.2i", nether->resources[0][5]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"CHASSIS %.2i", nether->resources[0][6]);
      }
    }
    break;
  case Menu::ROBOT_MENU:
  case Menu::DIRECTCONTROL_MENU:
    {
      StatusButton* b = getbutton(StatusButton::ROBOT1_BUTTON);
      if (b!=0 && b->status==0) {
        glTranslatef(70,140,0);
        glColor3f(1.0f,1.0f,0.0);
        scaledglprintf(0.1f,0.1f,"-ORDERS-");
        glColor3f(0.5f,0.5f,1.0f);
        switch(nether->controlled->program) {
        case Robot::PROGRAM_STOPDEFEND:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"STOP");
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"AND");
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"DEFEND");
          break;
        case Robot::PROGRAM_ADVANCE:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"ADVANCE");
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"%.2i", nether->controlled->program_parameter.as_int / 2);
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"MILES");
          break;
        case Robot::PROGRAM_RETREAT:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"RETREAT");
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"%.2i",nether->controlled->program_parameter.as_int / 2);
          glTranslatef(0,-18,0);
          scaledglprintf(0.1f,0.1f,"MILES");
          break;
        case Robot::PROGRAM_DESTROY:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"DESTROY");
          switch(nether->controlled->program_parameter.param) {
          case Robot::P_PARAM_ROBOTS:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ROBOTS");
            break;
          case Robot::P_PARAM_WARBASES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"WARBASES");
            break;
          case Robot::P_PARAM_NFACTORIES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"NEUTRAL");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"FACTORIES");
            break;
          case Robot::P_PARAM_EFACTORIES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"FACTORIES");
            break;
          }
          break;
        case Robot::PROGRAM_CAPTURE:
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"CAPTURE");
          switch(nether->controlled->program_parameter.param) {
          case Robot::P_PARAM_ROBOTS:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ROBOTS");
            break;
          case Robot::P_PARAM_WARBASES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"WARBASES");
            break;
          case Robot::P_PARAM_NFACTORIES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"NEUTRAL");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"FACTORIES");
            break;
          case Robot::P_PARAM_EFACTORIES:
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"ENEMY");
            glTranslatef(0,-18,0);
            scaledglprintf(0.1f,0.1f,"FACTORIES");
            break;
          }
          break;
        }

        glTranslatef(0,-44,0);
        glColor3f(1.0f,1.0f,0.0);
        scaledglprintf(0.1f,0.1f,"STRENGTH");
        glTranslatef(0,-18,0);
        glColor3f(1.0f,1.0f,1.0f);
        scaledglprintf(0.1f,0.1f,"%.3i%c", nether->controlled->strength,'%');
      }
    }
    break;

  case Menu::COMBATMODE_MENU:
  case Menu::DIRECTCONTROL2_MENU:
    glTranslatef(70,40,0);
    glColor3f(1.0f,1.0f,0.0);
    scaledglprintf(0.1f,0.1f,"STRENGTH");
    glTranslatef(0,-18,0);
    glColor3f(1.0f,1.0f,1.0f);
    scaledglprintf(0.1f,0.1f,"%.3i%c",nether->controlled->strength,'%');
    break;

  case Menu::ORDERS_MENU:
      {
        StatusButton* b =getbutton(StatusButton::ORDERS1_BUTTON);
        if (b!=0 && b->status==0) {
          glTranslatef(70,400,0);
          glColor3f(1.0f,1.0f,1.0f);
          scaledglprintf(0.1f,0.1f,"SELECT");
          glTranslatef(0,-20,0);
          scaledglprintf(0.1f,0.1f,"ORDERS");

          glTranslatef(0,-340,0);
          glColor3f(1.0f,1.0f,0.0);
          scaledglprintf(0.1f,0.1f,"STRENGTH");
          glTranslatef(0,-18,0);
          glColor3f(1.0f,1.0f,1.0f);
          scaledglprintf(0.1f,0.1f,"%.3i%c", nether->controlled->strength,'%');
        }
      }
      break;

  case Menu::SELECTDISTANCE_MENU:
    {
      StatusButton* b = getbutton(StatusButton::ORDERS_BUTTON);
      if (b!=0 && b->status==0) {
        glTranslatef(70,300,0);
        glColor3f(0.5f,0.5f,1.0f);
        scaledglprintf(0.1f,0.1f,"SELECT");
        glTranslatef(0,-20,0);
        scaledglprintf(0.1f,0.1f,"DISTANCE");

        glColor3f(1.0f,1.0f,0.0);
        glTranslatef(0,-40,0);
        scaledglprintf(0.1f,0.1f,"%.2i MILES", nether->controlled->program_parameter.as_int / 2);

        glTranslatef(0,-200,0);
        glColor3f(1.0f,1.0f,0.0);
        scaledglprintf(0.1f,0.1f,"STRENGTH");
        glTranslatef(0,-18,0);
        glColor3f(1.0f,1.0f,1.0f);
        scaledglprintf(0.1f,0.1f,"%.3i%c", nether->controlled->strength,'%');
      }
    }
    break;

  case Menu::TARGETD_MENU:
  case Menu::TARGETC_MENU:
    {
      StatusButton* b = getbutton(StatusButton::ORDERS_BUTTON);
      if (b!=0 && b->status==0) {
        glTranslatef(70,350,0);
        glColor3f(0.5f,0.5f,1.0f);
        scaledglprintf(0.1f,0.1f,"SELECT");
        glTranslatef(0,-20,0);
        scaledglprintf(0.1f,0.1f,"TARGET");

        glTranslatef(0,-290,0);
        glColor3f(1.0f,1.0f,0.0);
        scaledglprintf(0.1f,0.1f,"STRENGTH");
        glTranslatef(0,-18,0);
        glColor3f(1.0f,1.0f,1.0f);
        scaledglprintf(0.1f,0.1f,"%.3i%c", nether->controlled->strength,'%');
      }
    }
    break;

  }
  glPopMatrix();
}


void Menu::drawButtons()
{
  float angle, cf;

  for (StatusButton* b: buttons) {
    if (b->status >= -16) {
      angle=(float(b->status) * 90.0) / 16.0;
      cf=float((16 - abs(b->status))) / 16.0;
      glPushMatrix();
      glTranslatef(b->x, b->y, 0);
      glRotatef(angle, 0, 1, 0);

      /* Draw button: */
      glColor3f(b->r * cf, b->g * cf, b->b * cf);
      glutSolidBox(b->sx / 2, b->sy / 2, 10.0);
      glTranslatef(0, 0, 11);

      glColor3f(1.0, 1.0, 1.0);
      if (!b->text1.empty()) {
        if (!b->text2.empty()) {
          glTranslatef(0, -12, 0);
          scaledglprintf(0.1f, 0.1f, b->text2.c_str());
          glTranslatef(0, 17, 0);
          scaledglprintf(0.1f, 0.1f, b->text1.c_str());
        } else {
          glTranslatef(0, -3, 0);
          scaledglprintf(0.1f, 0.1f, b->text1.c_str());
        }
      }
      glPopMatrix();
    }
  }
}


void Menu::cycle()
{
  auto i = std::begin(buttons);
  auto end = std::end(buttons);
  while (i != end) {
    auto b = *i;
    if (b->status != 0) {
      b->status++;
      needsRedraw = 2;
      if (b->status >= 16) {
        delete b;
        i = buttons.erase(i);
      } else {
        i++;
      }
    } else {
      i++;
    }
  }
}


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
    if (nether->controlled->program==Robot::PROGRAM_ADVANCE)
      newbuttondelayed(StatusButton::ORDERS_BUTTON,70,400,130,40,"ADVANCE  "," ?? MILES",0,0,0.8f);
    if (nether->controlled->program==Robot::PROGRAM_RETREAT)
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
  needsRedraw = true;
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
  needsRedraw = true;
}


void Menu::newbutton(StatusButton::BUTTON_NAMES ID, int x, int y, int sx, int sy,
                     const std::string& t1, const std::string& t2, float r, float g, float b)
{
  buttons.push_back(new StatusButton(ID, x, y, sx, sy, t1, t2, r, g, b, -16));
  needsRedraw = true;
}


void Menu::newbuttondelayed(StatusButton::BUTTON_NAMES ID, int x, int y, int sx, int sy,
                            const std::string& t1, const std::string& t2, float r, float g, float b)
{
  buttons.push_back(new StatusButton(ID, x, y, sx, sy, t1, t2, r, g, b, -32));
  needsRedraw = true;
}


void Menu::killbutton(StatusButton::BUTTON_NAMES ID)
{
  std::for_each(std::cbegin(buttons), std::cend(buttons),
                [ID](StatusButton* b) {if (b->ID == ID) b->status = 1; });
  needsRedraw = true;
}


StatusButton* Menu::getbutton(StatusButton::BUTTON_NAMES ID)
{
  auto result = find_if(std::cbegin(buttons), std::cend(buttons),
                        [ID](StatusButton* b) {return b->ID == ID;});
  if (result != end(buttons)) {
    return *result;
  } else {
    return 0;
  }
}


void Menu::replaceMenu(MENU_TYPES oldMenu, MENU_TYPES newMenu,
                       StatusButton::BUTTON_NAMES activeButton)
{
  killmenu(oldMenu);
  newmenu(newMenu);
  act_button = activeButton;
}
