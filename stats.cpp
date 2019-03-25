#include "buildingblock.h"
#include "robot.h"
#include "stats.h"


void Stats::recompute(const std::vector<BuildingBlock>& buildings)
{
  if (!needsRecomputing) return;

  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 8; j++)
      stats[i][j] = 0;

  for (const BuildingBlock& b: buildings) {
    int index = b.owner - 1;
    if (b.type==BuildingBlock::TYPE::WARBASE) {
      stats[index][0]++;
    }
    if (b.type==BuildingBlock::TYPE::FACTORY_ELECTRONICS) {
      stats[index][1]++;
    }
    if (b.type==BuildingBlock::TYPE::FACTORY_NUCLEAR) {
      stats[index][2]++;
    }
    if (b.type==BuildingBlock::TYPE::FACTORY_PHASERS) {
      stats[index][3]++;
    }
    if (b.type==BuildingBlock::TYPE::FACTORY_MISSILES) {
      stats[index][4]++;
    }
    if (b.type==BuildingBlock::TYPE::FACTORY_CANNONS) {
      stats[index][5]++;
    }
    if (b.type==BuildingBlock::TYPE::FACTORY_CHASSIS) {
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


bool Stats::canBuildRobot(int player, const Robot& robot)
{
  std::array<int, 7> invoice = calculateCost(robot);
  int generalRequired = 0;
  for (int i = 1; i < 7; i++) {
    generalRequired -= std::min(resources[player][i] - invoice[i], 0);
  }
  return abs(generalRequired) <= resources[player][0];
}


void Stats::spendRobotResources(int player, const Robot& robot)
{
  std::array<int, 7> normalized {normalizeCost(player, calculateCost(robot))};
  for (int i = 0; i < 7; i++) {
    resources[player][i] -= normalized[i];
  }
}


std::array<int, 7> Stats::calculateCost(const Robot& robot)
{
  int prices[6] = {0, 3, 20, 4, 4, 2};

  std::array<int, 7> invoice {0};
  // Calculate specific resources first, except chasis
  for (int robotIndex = 0, priceIndex = 5; robotIndex < 5; robotIndex++, priceIndex--) {
    if (robot.pieces[robotIndex]) {
      invoice[priceIndex] = prices[priceIndex];
    }
  }

  switch (robot.traction) {
  case 0:
    invoice[6] = 3;
    break;
  case 1:
    invoice[6] = 5;
    break;
  case 2:
    invoice[6] = 10;
    break;
  }
  return invoice;
}


std::array<int, 7> Stats::normalizeCost(int player, const std::array<int, 7>& invoice)
{
  std::array<int, 7> normalized {0};
  for (int i = 1; i < 7; i++) {
    normalized[0] -= std::min(resources[player][i] - invoice[i], 0);
    normalized[i] = std::min(resources[player][i], invoice[i]);
  }
  return normalized;
}


bool Stats::noWarbasesLeft()
{
  return !(stats[0][0] && stats[1][0]);
}
