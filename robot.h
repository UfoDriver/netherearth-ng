#ifndef ROBOT_H
#define ROBOT_H

#include <bitset>
#include <memory>

#include <sexp/value.hpp>

#include "cmc.h"
#include "piece3dobject.h"
#include "robots.h"
#include "vector.h"


class Building;
class Bullet;
class NETHER;
class Ship;


struct AnimationState {
  int chassis = 0;
  int electronics = 0;
};


struct RobotProgram {
  enum TYPE {
    NONE = -1,
    FORWARD,
    STOPDEFEND,
    ADVANCE,
    RETREAT,
    DESTROY,
    CAPTURE
  };

  enum PARAM {
    ROBOTS = 1,
    WARBASES,
    NEUTRAL_FACTORIES,
    ENEMY_FACTORIES
  };

  TYPE type;
  // @TODO: program param as variant instead of union
  union {
    PARAM param;
    int as_int;
  } parameter;
  Vector goal;
};

class Robot {
public:
  enum class OPERATOR {
    NONE = -1,
    FORWARD,
    LEFT,
    RIGHT,
    CANNONS,
    MISSILES,
    PHASERS,
    NUCLEAR
  };

  explicit Robot(unsigned short owner);
  Robot(const sexp::Value& sexp) { Robot(-1); fromSexp(sexp); }

  bool valid() const;
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

  int npieces() const { return pieces.count(); }
  unsigned short getOwner() { return owner; }
  void dispatchOperator(NETHER* nether, unsigned char* keyboard);

  void setPieces(std::bitset<5> newPieces) { pieces = newPieces; }
  std::bitset<5> getPieces() const { return pieces; }
  void setAngle(int newAngle) { angle = newAngle; }
  int getAngle() const { return angle; }
  void setTraction(int newTraction) { traction = newTraction; }
  int getTraction() const { return traction; }
  void detachShip() {
    animation.electronics = 6;
    shipover = false;
  }
  void attachShip() { shipover = true; }
  bool isShipOver() const { return shipover; }

  RobotProgram program;
  OPERATOR op;

  int firetimer {0};
  int strength {100};

  Vector pos;
  CMC cmc;

  void copyDesign(const Robot& robot);
  void calculateCMC(std::vector<Piece3DObject>& pieceTiles);

  sexp::Value toSexp() const;
  bool fromSexp(const sexp::Value&);

private:
  std::bitset<5> pieces = false;
  int traction {-1};
  int angle;
  bool shipover {false};

  int id { Robot::counter++ };
  unsigned short owner;

  AnimationState animation = {0};

  static int counter;
  static const float MOVING_SPEED[4][3];
  static const int ROTATION_SPEED[4][3];

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

  float piecesHeight(int piece);
};

#endif // ROBOT_H
