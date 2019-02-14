#ifndef ROBOT_H
#define ROBOT_H

#include "bullet.h"
#include "cmc.h"
#include "vector.h"


class Robot {
public:
  enum ROBOT_PROGRAMS {PROGRAM_NONE = -1,
                       PROGRAM_FORWARD,
                       PROGRAM_STOPDEFEND,
                       PROGRAM_ADVANCE,
                       PROGRAM_RETREAT,
                       PROGRAM_DESTROY,
                       PROGRAM_CAPTURE};

  enum PROGRAM_PARAM {P_PARAM_ROBOTS = 1,
                      P_PARAM_WARBASES,
                      P_PARAM_NFACTORIES,
                      P_PARAM_EFACTORIES};
  Robot();
  explicit Robot(std::istream& in);
  bool valid();
  float piecez(int piece);
  bool bulletHit(Bullet::BULLET_TYPE type);
  void draw(int owner, bool shadows, std::vector<Piece3DObject> piece_tiles[2], Vector lightposv);
  int cost();
  void cost(int player, int* res, int resources[2][7]);
  bool operator==(const Robot& other) const { return id == other.id; };
  bool hasCannons() { return pieces[0]; }
  bool hasMissiles() { return pieces[1]; }
  bool hasPhasers() { return pieces[2]; }
  bool hasElectronics() { return pieces[4]; }

  int getId() const { return id; }

  int traction;
  bool pieces[5];

  ROBOT_PROGRAMS program;
  union {
    PROGRAM_PARAM param;
    int as_int;
  } program_parameter;
  Vector program_goal;

  int op;
  bool shipover;
  int firetimer;
  int strength;

  Vector pos;
  int angle;
  CMC cmc;

  /* Animation variables: */
  int electronics_state;
  int chassis_state;
private:
  int id;
  static int counter;
};

#endif // ROBOT_H
