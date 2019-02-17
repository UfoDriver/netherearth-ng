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
  case Menu::TYPE::GENERAL:
    {
      StatusButton* b = getbutton(StatusButton::NAME::STATUS);
      if (b != 0 && b->status == 0) {
        int robots0 =  nether->robots[0].size();
        int robots1 = nether->robots[1].size();

        glColor3f(0.5f,0.5f,1.0f);
        glTranslatef(70,356,0);
        scaledglprintf(0.1f,0.1f,"%i WARBASES %i", nether->stats.stats[1][0], nether->stats.stats[0][0]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i ELECTR'S %i", nether->stats.stats[1][1], nether->stats.stats[0][1]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i NUCLEAR  %i", nether->stats.stats[1][2], nether->stats.stats[0][2]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i PHASERS  %i", nether->stats.stats[1][3], nether->stats.stats[0][3]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i MISSILES %i", nether->stats.stats[1][4], nether->stats.stats[0][4]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i  CANNON  %i", nether->stats.stats[1][5], nether->stats.stats[0][5]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%i CHASSIS  %i", nether->stats.stats[1][6], nether->stats.stats[0][6]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"%.2i ROBOTS %.2i", robots1, robots0);
        glColor3f(0.0f,0.8f,0.0f);
        glTranslatef(0,-65,0);
        scaledglprintf(0.1f,0.1f,"GENERAL %.2i", nether->stats.resources[0][0]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"ELECTR' %.2i", nether->stats.resources[0][1]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"NUCLEAR %.2i", nether->stats.resources[0][2]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"PHASERS %.2i", nether->stats.resources[0][3]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"MISSILE %.2i", nether->stats.resources[0][4]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"CANNON  %.2i", nether->stats.resources[0][5]);
        glTranslatef(0,-18,0);
        scaledglprintf(0.1f,0.1f,"CHASSIS %.2i", nether->stats.resources[0][6]);
      }
    }
    break;
  case Menu::TYPE::ROBOT:
  case Menu::TYPE::DIRECTCONTROL:
    {
      StatusButton* b = getbutton(StatusButton::NAME::ROBOT1);
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

  case Menu::TYPE::COMBATMODE:
  case Menu::TYPE::DIRECTCONTROL2:
    glTranslatef(70,40,0);
    glColor3f(1.0f,1.0f,0.0);
    scaledglprintf(0.1f,0.1f,"STRENGTH");
    glTranslatef(0,-18,0);
    glColor3f(1.0f,1.0f,1.0f);
    scaledglprintf(0.1f,0.1f,"%.3i%c",nether->controlled->strength,'%');
    break;

  case Menu::TYPE::ORDERS:
      {
        StatusButton* b =getbutton(StatusButton::NAME::ORDERS1);
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

  case Menu::TYPE::SELECTDISTANCE:
    {
      StatusButton* b = getbutton(StatusButton::NAME::ORDERS);
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

  case Menu::TYPE::TARGET_DESTROY:
  case Menu::TYPE::TARGET_CAPTURE:
    {
      StatusButton* b = getbutton(StatusButton::NAME::ORDERS);
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


void Menu::newmenu(TYPE menu)
{
  switch(menu) {
  case TYPE::GENERAL:
    if (getbutton(StatusButton::NAME::TIME)==0) {
      newbutton(StatusButton::NAME::TIME,70,455,130,40,"Day: 0","Time: 00:00",0.8f,0,0);
    }
    newbuttondelayed(StatusButton::NAME::STATUS,70,400,130,50,"STATUS","INSG  HUMN",0.0,0,0);
    newbuttondelayed(StatusButton::NAME::RESOURCE,70,200,130,30,"RESOURCES", "",0.0,0,0);
    act_menu=TYPE::GENERAL;
    break;

  case TYPE::ROBOT:
    newbuttondelayed(StatusButton::NAME::ROBOT1,70,350,130,40,"DIRECT   ","  CONTROL",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::ROBOT2,70,300,130,40,"GIVE     ","   ORDERS",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::ROBOT3,70,250,130,40,"COMBAT   ","     MODE",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::ROBOT4,70,200,130,40,"LEAVE    ","    ROBOT",0,0,0.8f);
    act_menu=TYPE::ROBOT;
    break;

  case TYPE::COMBATMODE:
    newbuttondelayed(StatusButton::NAME::COMBAT1,70,350,130,40,"NUCLEAR  ","     BOMB",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::COMBAT2,70,300,130,40,"FIRE     ","  PHASERS",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::COMBAT3,70,250,130,40,"FIRE     "," MISSILES",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::COMBAT4,70,200,130,40,"FIRE     ","   CANNON",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::COMBAT5,70,150,130,40,"MOVE     ","    ROBOT",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::COMBAT6,70,100,130,40,"STOP     ","   COMBAT",0,0,0.8f);
    act_menu=TYPE::COMBATMODE;
    break;

  case TYPE::ORDERS:
    newbuttondelayed(StatusButton::NAME::ORDERS1,70,350,130,40,"STOP AND ","   DEFEND",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::ORDERS2,70,300,130,40,"ADVANCE  "," ?? MILES",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::ORDERS3,70,250,130,40,"RETREAT  "," ?? MILES",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::ORDERS4,70,200,130,40,"SEARCH & ","  DESTROY",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::ORDERS5,70,150,130,40,"SEARCH & ","  CAPTURE",0,0,0.8f);
    act_menu=TYPE::ORDERS;
    break;

  case TYPE::SELECTDISTANCE:
    if (nether->controlled->program==Robot::PROGRAM_ADVANCE)
      newbuttondelayed(StatusButton::NAME::ORDERS,70,400,130,40,"ADVANCE  "," ?? MILES",0,0,0.8f);
    if (nether->controlled->program==Robot::PROGRAM_RETREAT)
      newbuttondelayed(StatusButton::NAME::ORDERS,70,400,130,40,"RETREAT  "," ?? MILES",0,0,0.8f);
    act_menu=TYPE::SELECTDISTANCE;
    break;

  case TYPE::TARGET_DESTROY:
    newbuttondelayed(StatusButton::NAME::ORDERS,70,400,130,40,"SEARCH & ","  DESTROY",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::TARGET1,70,300,130,40,"ENEMY    ","   ROBOTS",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::TARGET2,70,250,130,40,"ENEMY    ","FACTORIES",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::TARGET3,70,200,130,40,"ENEMY    "," WARBASES",0,0,0.8f);
    act_menu=TYPE::TARGET_DESTROY;
    break;

  case TYPE::TARGET_CAPTURE:
    newbuttondelayed(StatusButton::NAME::ORDERS,70,400,130,40,"SEARCH & ","  CAPTURE",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::TARGET1,70,300,130,40,"NEUTRAL  ","FACTORIES",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::TARGET2,70,250,130,40,"ENEMY    ","FACTORIES",0,0,0.8f);
    newbuttondelayed(StatusButton::NAME::TARGET3,70,200,130,40,"ENEMY    "," WARBASES",0,0,0.8f);
    act_menu=TYPE::TARGET_CAPTURE;
    break;
  }
  needsRedraw = true;
}


void Menu::killmenu(TYPE menu)
{
  switch(menu) {
  case TYPE::GENERAL:
    killbutton(StatusButton::NAME::STATUS);
    killbutton(StatusButton::NAME::RESOURCE);
    break;

  case TYPE::ROBOT:
    killbutton(StatusButton::NAME::ROBOT1);
    killbutton(StatusButton::NAME::ROBOT2);
    killbutton(StatusButton::NAME::ROBOT3);
    killbutton(StatusButton::NAME::ROBOT4);
    break;

  case TYPE::COMBATMODE:
    killbutton(StatusButton::NAME::COMBAT1);
    killbutton(StatusButton::NAME::COMBAT2);
    killbutton(StatusButton::NAME::COMBAT3);
    killbutton(StatusButton::NAME::COMBAT4);
    killbutton(StatusButton::NAME::COMBAT5);
    killbutton(StatusButton::NAME::COMBAT6);
    break;

  case TYPE::ORDERS:
    killbutton(StatusButton::NAME::ORDERS1);
    killbutton(StatusButton::NAME::ORDERS2);
    killbutton(StatusButton::NAME::ORDERS3);
    killbutton(StatusButton::NAME::ORDERS4);
    killbutton(StatusButton::NAME::ORDERS5);
    break;

  case TYPE::SELECTDISTANCE:
    killbutton(StatusButton::NAME::ORDERS);
    break;

  case TYPE::TARGET_DESTROY:
  case TYPE::TARGET_CAPTURE:
    killbutton(StatusButton::NAME::ORDERS);
    killbutton(StatusButton::NAME::TARGET1);
    killbutton(StatusButton::NAME::TARGET2);
    killbutton(StatusButton::NAME::TARGET3);
    break;

  case TYPE::ALL:
    killbutton(StatusButton::NAME::STATUS);
    killbutton(StatusButton::NAME::RESOURCE);
    killbutton(StatusButton::NAME::ROBOT1);
    killbutton(StatusButton::NAME::ROBOT2);
    killbutton(StatusButton::NAME::ROBOT3);
    killbutton(StatusButton::NAME::ROBOT4);
    killbutton(StatusButton::NAME::COMBAT1);
    killbutton(StatusButton::NAME::COMBAT2);
    killbutton(StatusButton::NAME::COMBAT3);
    killbutton(StatusButton::NAME::COMBAT4);
    killbutton(StatusButton::NAME::COMBAT5);
    killbutton(StatusButton::NAME::COMBAT6);
    killbutton(StatusButton::NAME::ORDERS1);
    killbutton(StatusButton::NAME::ORDERS2);
    killbutton(StatusButton::NAME::ORDERS3);
    killbutton(StatusButton::NAME::ORDERS4);
    killbutton(StatusButton::NAME::ORDERS5);
    killbutton(StatusButton::NAME::ORDERS);
    killbutton(StatusButton::NAME::ORDERS);
    killbutton(StatusButton::NAME::TARGET1);
    killbutton(StatusButton::NAME::TARGET2);
    killbutton(StatusButton::NAME::TARGET3);
    break;
  }
  needsRedraw = true;
}


void Menu::newbutton(StatusButton::NAME ID, int x, int y, int sx, int sy,
                     const std::string& t1, const std::string& t2, float r, float g, float b)
{
  buttons.push_back(new StatusButton(ID, x, y, sx, sy, t1, t2, r, g, b, -16));
  needsRedraw = true;
}


void Menu::newbuttondelayed(StatusButton::NAME ID, int x, int y, int sx, int sy,
                            const std::string& t1, const std::string& t2, float r, float g, float b)
{
  buttons.push_back(new StatusButton(ID, x, y, sx, sy, t1, t2, r, g, b, -32));
  needsRedraw = true;
}


void Menu::killbutton(StatusButton::NAME ID)
{
  std::for_each(std::cbegin(buttons), std::cend(buttons),
                [ID](StatusButton* b) {if (b->ID == ID) b->status = 1; });
  needsRedraw = true;
}


StatusButton* Menu::getbutton(StatusButton::NAME ID)
{
  auto result = find_if(std::cbegin(buttons), std::cend(buttons),
                        [ID](StatusButton* b) {return b->ID == ID;});
  if (result != end(buttons)) {
    return *result;
  } else {
    return 0;
  }
}


void Menu::replaceMenu(TYPE oldMenu, TYPE newMenu,
                       StatusButton::NAME activeButton)
{
  killmenu(oldMenu);
  newmenu(newMenu);
  act_button = activeButton;
}
