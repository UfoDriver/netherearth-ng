#ifndef AI_H
#define AI_H

#include <vector>

#include "map.h"
#include "robot.h"
#include "vector.h"

class AIOperator;
class NETHER;
class Scene;


class AI
{
public:
  enum class STATE {EXPANDING,
                    FIGHTING,
                    DEFENDING,
                    CONQUERING,
                    DESTROYING};

  AI(NETHER* nether, Scene* scene): nether{nether}, scene{scene} {};
  void makePrecomputations();
  void deletePrecomputations();
  void enemy();
  int worseMapTerrain(const int x, const int y, const int dx, const int dy);
  void newRobot(const Vector& pos, const int owner);
  void moveRobot(const Vector& oldpos, const Vector& newpos, const int owner);
  int killRobot(const Vector& pos);
  int robotHere(const Vector& pos);
  Robot::OPERATOR programAdvance(const Robot& robot, const int player);
  Robot::OPERATOR programRetreat(const Robot& robot, const int player);
  Robot::OPERATOR programCapture(const Robot& robot, Vector *program_goal, const int player);
  Robot::OPERATOR programDestroy(const Robot& robot, Vector *program_goal, const int player);
  Robot::OPERATOR programStopDefend(const Robot& robot, Vector *program_goal, const int player);
  void removeBuilding(const Vector& pos);
  int discreetmapSize() const { return discreetmap.size(); }
  int discreetmapValue(int index) const { return discreetmap[index]; }

private:
  Robot* enemyNewRobot(const STATE, const Vector& pos);
  void release();
  void availableOperators(const Robot& robot, std::vector<AIOperator>& l);
  bool expandOperators(const int x, const int y, const int angle, const Robot& robot,
                       const int previous, const int oldcost, const int depth);
  Robot::OPERATOR searchEngine(const Robot& robot, const int goaltype, const Vector& goalpos, const int depth);
  void resetSearch(const Vector& pos, const int depth);
  void rankOperatorsAdvance(std::vector<AIOperator>& l);
  void rankOperatorsRetreat(std::vector<AIOperator>& l);
  void rankOperatorsCapture(std::vector<AIOperator>&l, Vector goal);
  const AIOperator chooseOperator(std::vector<AIOperator>& l, const int factor);
  int  realShotPaths(const int x, const int y, const int player, const int persistence);
  void fillZone(std::vector<int>& map, int w, int val, int x, int y, int dx, int dy);
  void robotZone(const Vector& pos,int *x,int *y,int *dx,int *dy);

  std::vector<int> discreetmap;
  std::vector<int> bk_discreetmap;
  std::vector<AIOperator> searchmap;
  std::vector<int> attackmap;

  NETHER* nether;
  Scene* scene;
};


#endif // AI_H
