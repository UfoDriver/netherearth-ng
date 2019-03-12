#include <GL/gl.h>

#include "glprintf.h"
#include "myglutaux.h"
#include "statusbutton.h"


void StatusButton::draw()
{
  if (status >= -16) {
    float angle = (float(status) * 90.0) / 16.0;
    float cf = float((16 - abs(status))) / 16.0;
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(angle, 0, 1, 0);

    glColor3f(color.red * cf, color.green * cf, color.blue * cf);
    glutSolidBox(sx / 2, sy / 2, 10.0);
    glTranslatef(0, 0, 11);

    glColor3f(1.0, 1.0, 1.0);
    if (!text1.empty()) {
      if (!text2.empty()) {
        glTranslatef(0, -12, 0);
        scaledglprintf(0.1f, 0.1f, text2.c_str());
        glTranslatef(0, 17, 0);
        scaledglprintf(0.1f, 0.1f, text1.c_str());
      } else {
        glTranslatef(0, -3, 0);
        scaledglprintf(0.1f, 0.1f, text1.c_str());
      }
    }
    glPopMatrix();
  }
}
