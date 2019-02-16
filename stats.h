#ifndef STATS_H
#define STATS_H

#include <iostream>
#include <vector>


class Building;

const int BONUSES[][5] = {{0, 5, 6, 6, 8},
                          {0, 5, 8, 9, 12},
                          {0, 5, 10, 12, 16},
                          {0, 5, 10, 15, 20}};


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
