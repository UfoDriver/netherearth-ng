#ifndef ROBOT_H
#define ROBOT_H

#include <memory>
#include "cmc.h"
#include "piece3dobject.h"
#include "robots.h"
#include "vector.h"

class Building;
class Bullet;
class NETHER;
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
  bool checkCollision(const std::vector<std::unique_ptr<Building>>& buildings,
                      const Robots& robots, bool complete, Ship* ship) const;

  void cycle(NETHER* nether);
  int getId() const { return id; }

  float movingSpeed(int terrain) const;
  int rotationSpeed(int terrain) const;
  bool walkable(int terrain) const;
  int npieces() const;
  unsigned short getOwner() { return owner; }
  void dispatchOperator(NETHER* nether, unsigned char* keyboard);

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
  void shipDetached() { electronicsState = 6; }
  int getElectronicsState() { return electronicsState; }

  /* Animation variables: */
  int chassisState {0};

private:
  int id {Robot::counter++};
  unsigned short owner;
  int electronicsState {0};

  static int counter;
  static const float MS[4][3];
  static const int RS[4][3];

  void processOperatorForward(NETHER* nether, unsigned char* keyboard);
  void processOperatorLeft(NETHER* nether, unsigned char* keyboard);
  void processOperatorRight(NETHER* nether, unsigned char* keyboard);
  void processOperatorCannons(NETHER* nether, unsigned char* keyboard);
  void processOperatorMissiles(NETHER* nether, unsigned char* keyboard);
  void processOperatorPhasers(NETHER* nether, unsigned char* keyboard);
  void processOperatorNuclear(NETHER* nether, unsigned char* keyboard);
  void processOperatorNone(NETHER* nether, unsigned char* keyboard);
  void processProgram(NETHER* nether, unsigned char* keyboard);
  void processDirectInput(NETHER* nether, unsigned char* keyboard);

  bool checkCollision(const std::vector<std::unique_ptr<Building>>& buildings) const;
  bool checkCollision(const Robots& robots) const;
  bool checkCollision(Ship* ship) const;

  friend std::ostream& operator<<(std::ostream& out, const Robot& robot);
};

std::ostream& operator<<(std::ostream& out, const Robot& robot);

#endif // ROBOT_H
