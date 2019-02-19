#include "stats.h"

#include "building.h"


void Stats::recompute(const std::vector<Building>& buildings)
{
  if (!needsRecomputing) return;

  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 8; j++)
      stats[i][j] = 0;

  for (const Building& b: buildings) {
    int index = b.owner - 1;
    if (b.type==Building::TYPE::WARBASE) {
      stats[index][0]++;
    }
    if (b.type==Building::TYPE::FACTORY_ELECTRONICS) {
      stats[index][1]++;
    }
    if (b.type==Building::TYPE::FACTORY_NUCLEAR) {
      stats[index][2]++;
    }
    if (b.type==Building::TYPE::FACTORY_PHASERS) {
      stats[index][3]++;
    }
    if (b.type==Building::TYPE::FACTORY_MISSILES) {
      stats[index][4]++;
    }
    if (b.type==Building::TYPE::FACTORY_CANNONS) {
      stats[index][5]++;
    }
    if (b.type==Building::TYPE::FACTORY_CHASSIS) {
      stats[index][6]++;
    }
  }
  needsRecomputing = false;
}


bool Stats::tick(int level)
{
  second += 5;
  if (second >= 60) {
    second = 0;
    minute += 5;
    if (minute >= 60) {
      minute = 0;
      hour++;
      if (hour >= 24) {
        hour = 0;
        day++;
        /* Resource actualization: */
        resources[0][0] += stats[0][0] * 5;

        if (stats[1][0] < 4)
          resources[1][0] += BONUSES[level][stats[1][0]];
        else
          resources[1][0] += stats[1][0] * (level + 2);

        for (int i = 1; i < 7; i++) {
          resources[0][i] += stats[0][i] * 2;
          resources[1][i] += stats[1][i] * 2;
        }
      }
    }
    return true;
  }
  return false;
}


std::istream& operator>>(std::istream& in, Stats& stats)
{
  in >> stats.day >> stats.hour >> stats.minute >> stats.second;
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      in >> stats.resources[i][j];
    }
  }
  return in;
}


std::ostream& operator<<(std::ostream& out, Stats& stats)
{
  out << stats.day << ' ' << stats.hour << ' ' << stats.minute << ' ' << stats.second << '\n';
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 7; j++) {
      out << stats.resources[i][j] << ' ';
    }
    out << '\n';
   }
  return out;
}


std::array<std::pair<int, int>, 7> Stats::getBuildingStats() const
{
  std::array<std::pair<int, int>, 7> data;
  for (int i = 0; i < data.size(); i++) {
    data[i] = std::make_pair(stats[0][i], stats[1][i]);
  }
  return data;
}

std::array<std::pair<int, int>, 7> Stats::getResourceStats() const
{
  std::array<std::pair<int, int>, 7> data;
  for (int i = 0; i < data.size(); i++) {
    data[i] = std::make_pair(resources[0][i], resources[1][i]);
  }
  return data;
}
