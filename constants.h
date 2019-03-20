#ifndef CONSTANTS_H
#define CONSTANTS_H

#include "color.h"


const int BONUSES[][5] = {{0, 5, 6, 6, 8},
                          {0, 5, 8, 9, 12},
                          {0, 5, 10, 12, 16},
                          {0, 5, 10, 15, 20}};

const float COLISION_TEST_THRESHOLD = 9.0;

const int INTRO_TIME = 60;
const int END_TIME = 260;

const float BULLET_SPEED = 0.1f;
const int CANNON_PERSISTENCE = 40;
const int MISSILE_PERSISTENCE = 60;
const int PHASER_PERSISTENCE = 40;

const int N_BUILDINGS = 9;
const int N_BULLETS = 3;
const int N_OBJECTS = 12;
const int N_PIECES = 11;

const Color PLAYER1_COLOR {0.3f, 0.3f, 0.8f};
const Color PLAYER2_COLOR {0.8f, 0.3f, 0.3f};

const int WE_SEARCH_DEPTH = 12; //Search depth for robots with ELECTRONICS
const int WOE_SEARCH_DEPTH = 4; // Search depth for robots without ELECTRONICS

const float NUCLEAR_RADIUS = 2.5f;

const int CAPTURE_TIME = 1728; // 12 * 12 * 12

#endif // CONSTANTS_H
