#ifndef STATS_H
#define STATS_H

#include <iostream>
#include <vector>

#include "constants.h"

class Building;


class Stats
{
public:
  Stats(): day(0), hour(0), minute(0), second(0), resources{{20, 0}, {20, 9}},
           stats{{0}, {0}}, needsRecomputing(true) {}
  void recompute(const std::vector<Building>& buildings);
  void requestRecomputing() { needsRecomputing = true; }
  bool tick(int level);

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
