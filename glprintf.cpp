#ifdef _WIN32
#include "windows.h"
#endif

#include <GL/gl.h>
#include <GL/glut.h>
#include <cstdio>
#include <numeric>
#include <string>

#include "stdio.h"
#include "stdarg.h"


void glprintf(const char *fmt, ...)
{
  std::string text(256, 0);							// Holds Our String
  va_list ap;										// Pointer To List Of Arguments

  if (fmt == 0) return;

  va_start(ap, fmt);								// Parses The String For Variables
  std::vsprintf(&text.front(), fmt, ap);			// And Converts Symbols To Actual Numbers
  va_end(ap);										// Results Are Stored In Text

  float textWidth = std::accumulate(text.cbegin(), text.cend(), 0,
                                    [](int a, const auto& c) {
                                      return a + glutStrokeWidth(GLUT_STROKE_MONO_ROMAN, c);
                                    });

  glTranslatef(-textWidth / 2, 0.0f, 0.0f);			// Center Our Text On The Screen

  glNormal3f(0.0, 0.0, 1.0);
  for (const auto& c : text) {
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
  }
}


void scaledglprintf(float sx, float sy, const char *fmt, ...)
{
  std::string text(255, 0);							// Holds Our String
  va_list ap;										// Pointer To List Of Arguments

  if (fmt == 0) return;

  va_start(ap, fmt);								// Parses The String For Variables
  std::vsprintf(&text.front(), fmt, ap);			// And Converts Symbols To Actual Numbers
  va_end(ap);										// Results Are Stored In Text

  float textWidth = std::accumulate(text.cbegin(), text.cend(), 0,
                                    [](int a, const auto& c) {
                                      return a + glutStrokeWidth(GLUT_STROKE_MONO_ROMAN, c);
                                    });

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glScalef(sx, sy, 1.0f);

  glTranslatef(-textWidth / 2, 0.0f, 0.0f);					// Center Our Text On The Screen

  glNormal3f(0.0, 0.0, 1.0);
  for (const auto& c : text) {
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
  }
  glPopMatrix();
}


void scaledglprintf2(float sx, float sy, const char *fmt, ...)
{
  std::string text(255, 0);							// Holds Our String
  va_list ap;										// Pointer To List Of Arguments

  if (fmt == 0) return;

  va_start(ap, fmt);								// Parses The String For Variables
  std::vsprintf(&text.front(), fmt, ap);			// And Converts Symbols To Actual Numbers
  va_end(ap);										// Results Are Stored In Text

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glScalef(sx, sy, 1.0f);

  glNormal3f(0.0, 0.0, 1.0);
  for (const auto& c : text) {
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
  }
  glPopMatrix();
}


void fittedglprintf(float sx, float sy, const char *fmt, ...)
{
  std::string text(255, 0);							// Holds Our String
  va_list ap;										// Pointer To List Of Arguments

  if (fmt == 0) return;

  va_start(ap, fmt);								// Parses The String For Variables
  std::vsprintf(&text.front(), fmt, ap);			// And Converts Symbols To Actual Numbers
  va_end(ap);										// Results Are Stored In Text

  float textWidth = std::accumulate(text.cbegin(), text.cend(), 0,
                                    [](int a, const auto& c) {
                                      return a + glutStrokeWidth(GLUT_STROKE_MONO_ROMAN, c);
                                    });

  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glScalef(sx / textWidth, sy / (119.05f + 33.33f), 1.0f);

  glTranslatef(-textWidth / 2, 0.0f, 0.0f);			// Center Our Text On The Screen

  glNormal3f(0.0, 0.0, 1.0);
  for (const auto& c : text) {
    glutStrokeCharacter(GLUT_STROKE_MONO_ROMAN, c);
  }

  glPopMatrix();
}
