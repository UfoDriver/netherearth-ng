#ifndef STATS_H
#define STATS_H

#include <array>
#include <iostream>
#include <memory>
#include <vector>

#include "constants.h"

class Building;
class Robot;


class Stats
{
public:
  Stats(): day(0), hour(0), minute(0), second(0), resources{{20, 0}, {20, 0}},
           stats{{0}, {0}}, needsRecomputing(true) {}
  void recompute(const std::vector<std::unique_ptr<Building>>& buildings);
  void requestRecomputing() { needsRecomputing = true; }
  bool tick(int level);
  std::array<std::pair<int, int>, 7> getBuildingStats() const;
  std::array<std::pair<int, int>, 7> getResourceStats() const;
  bool canBuildRobot(int player, const Robot& robot);
  void spendRobotResources(int player, const Robot& robot);
  std::array<int, 7> calculateCost(const Robot& robot);
  std::array<int, 7> normalizeCost(int player, const std::array<int, 7>& invoice);
  bool noWarbasesLeft();

  int day;
  int hour;
  int minute;
  int second;
  int resources[2][7];
  int stats[2][8];

private:
  bool needsRecomputing;
};

std::istream& operator>>(std::istream& in, Stats& stats);
std::ostream& operator<<(std::ostream& out, Stats& stats);

#endif // STATS
