#ifndef ROBOT_H
#define ROBOT_H

#include <memory>
#include "cmc.h"
#include "piece3dobject.h"
#include "robots.h"
#include "vector.h"

class Building;
class Bullet;
class Ship;


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

  enum class OPERATOR {NONE = -1,
                       FORWARD,
                       LEFT,
                       RIGHT,
                       CANNONS,
                       MISSILES,
                       PHASERS,
                       NUCLEAR};

  explicit Robot(unsigned short owner);
  Robot(unsigned short owner, std::istream& in);
  bool valid() const;
  float piecez(int piece);
  bool bulletHit(const std::unique_ptr<Bullet>& bullet);
  void draw(Vector lightposv, bool shadows) const;
  int cost() const;
  bool operator==(const Robot& other) const { return id == other.id; };
  bool hasCannons() const { return pieces[0]; }
  bool hasMissiles() const { return pieces[1]; }
  bool hasPhasers() const { return pieces[2]; }
  bool hasNuclear() const { return pieces[3]; }
  bool hasElectronics() const { return pieces[4]; }
  bool checkCollision(const std::vector<Building>& buildings,
                      const Robots& robots, bool complete, Ship* ship);

  void cycle();
  int getId() const { return id; }

  float robotSpeed(int terrain) const;
  int robotRotationSpeed(int terrain) const;
  bool walkable(int terrain) const;
  int npieces() const;
  unsigned short getOwner() { return owner; }

  int traction {-1};
  bool pieces[5] = {};

  // @TODO: program setter along with program param
  ROBOT_PROGRAMS program;
  // @TODO: program param as variant instead of union
  union {
    PROGRAM_PARAM param;
    int as_int;
  } program_parameter;
  Vector program_goal;

  OPERATOR op;
  bool shipover;
  int firetimer {0};
  int strength {100};

  Vector pos;
  int angle;
  CMC cmc;
  void calculateCMC(std::vector<Piece3DObject>& pieceTiles);
  void copyDesign(const Robot& robot);

  /* Animation variables: */
  int electronics_state {0};
  int chassis_state {0};

private:
  int id {Robot::counter++};
  unsigned short owner;
  static int counter;
  static const float MS[4][3];
  static const int RS[4][3];
};

std::ostream& operator<<(std::ostream& out, const Robot& robot);

#endif // ROBOT_H
