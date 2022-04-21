#include <algorithm>
#include <sexp/util.hpp>
#include <sexp/value.hpp>

#include "building.h"
#include "robot.h"
#include "stats.h"


void Stats::recompute(const std::vector<std::unique_ptr<Building>>& buildings)
{
  if (!needsRecomputing) return;

  for (int i = 0; i < 2; i++)
    for (int j = 0; j < 8; j++)
      stats[i][j] = 0;

  for (const auto& b: buildings) {
    int index = b->owner - 1;
    if (b->type == Building::TYPE::WARBASE) {
      stats[index][0]++;
    } else if (b->type == Building::TYPE::FACTORY) {
      switch (b->subtype) {
      case Building::SUBTYPE::ELECTRONICS:
        stats[index][1]++;
        break;
      case Building::SUBTYPE::NUCLEAR:
        stats[index][2]++;
        break;
      case Building::SUBTYPE::PHASERS:
        stats[index][3]++;
        break;
      case Building::SUBTYPE::MISSILES:
        stats[index][4]++;
        break;
      case Building::SUBTYPE::CANNONS:
        stats[index][5]++;
        break;
      case Building::SUBTYPE::CHASSIS:
        stats[index][6]++;
        break;
      case Building::SUBTYPE::UNKNOWN:
        break;
      }
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


std::array<std::pair<int, int>, 7> Stats::getBuildingStats() const
{
  std::array<std::pair<int, int>, 7> data;
  for (unsigned int i = 0; i < data.size(); i++) {
    data[i] = std::make_pair(stats[0][i], stats[1][i]);
  }
  return data;
}

std::array<std::pair<int, int>, 7> Stats::getResourceStats() const
{
  std::array<std::pair<int, int>, 7> data;
  for (unsigned int i = 0; i < data.size(); i++) {
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
  std::bitset<5> pieces = robot.getPieces();
  for (int robotIndex = 0, priceIndex = 5; robotIndex < 5; robotIndex++, priceIndex--) {
    if (pieces[robotIndex]) {
      invoice[priceIndex] = prices[priceIndex];
    }
  }

  switch (robot.getTraction()) {
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


sexp::Value Stats::toSexp() const
{
  sexp::Value player1Stat = sexp::Value::array();
  for (int j = 0; j <= 7; j++) {
    player1Stat.append(sexp::Value::integer(stats[0][j]));
  }
  sexp::Value player2Stat = sexp::Value::array();
  for (int j = 0; j <= 7; j++) {
    player2Stat.append(sexp::Value::integer(stats[1][j]));
  }

  sexp::Value stat = sexp::Value::list(
    sexp::Value::symbol("stats"),
    sexp::Value::integer(day),
    sexp::Value::integer(hour),
    sexp::Value::integer(minute),
    sexp::Value::integer(second),
    player1Stat,
    player2Stat
  );

  return stat;
}


bool Stats::fromSexp(const sexp::Value& value)
{
  day = sexp::cdar(value).as_int();
  hour = sexp::cddar(value).as_int();
  minute = sexp::cdddar(value).as_int();
  second = sexp::cdddar(value.get_cdr()).as_int();

  std::vector<sexp::Value> p1stat = sexp::cdddar(value.get_cdr().get_cdr()).as_array();
  std::transform(p1stat.cbegin(), p1stat.cend(), stats[0],
                 [](const sexp::Value& value) { return value.as_int(); });
  std::vector<sexp::Value> p2stat = sexp::cdddar(value.get_cdr().get_cdr().get_cdr()).as_array();
  std::transform(p2stat.cbegin(), p2stat.cend(), stats[1],
                 [](const sexp::Value& value) { return value.as_int(); });

  return true;
}
