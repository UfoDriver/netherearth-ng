#ifndef CONSTANTS_H
#define CONSTANTS_H


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

#endif // CONSTANTS_H
