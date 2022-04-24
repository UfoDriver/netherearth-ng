#ifndef ROBOTS_H
#define ROBOTS_H

#include <algorithm>
#include <memory>
#include <vector>

class Robot;


// @TODO PlayerOnly/PlayerOnly::iterator rework needed
class PlayerOnly {
public:
  class iterator {
  public:
    iterator(unsigned short player,
             std::vector<std::shared_ptr<Robot>>::iterator begin_,
             std::vector<std::shared_ptr<Robot>>::iterator end_);
    explicit iterator(std::vector<std::shared_ptr<Robot>>::iterator end_) : player {0}, internal{end_} {}

    iterator& operator++();
    bool operator!=(const iterator& other);
    Robot* operator*();

  private:
    unsigned short player;
    std::vector<std::shared_ptr<Robot>>::iterator internal;
    std::vector<std::shared_ptr<Robot>>::iterator end_;
  };

  explicit PlayerOnly(unsigned short player,
                      std::vector<std::shared_ptr<Robot>>::iterator begin_,
                      std::vector<std::shared_ptr<Robot>>::iterator end_)
    : player{player}, begin_{begin_}, end_{end_} {};

  iterator begin()
  {
    return iterator(player, begin_, end_);
  }
  iterator end()
  {
    return iterator(end_);
  }

private:
  unsigned short player;
  std::vector<std::shared_ptr<Robot>>::iterator begin_;
  std::vector<std::shared_ptr<Robot>>::iterator end_;
};


class Robots : public std::vector<std::shared_ptr<Robot>> {
public:
  int getRobotCount(unsigned short int owner);
  PlayerOnly forPlayer(unsigned short player)
  {
    return PlayerOnly(player, begin(), end());
  }
  int findIndex(std::shared_ptr<Robot> robot);
  void findAndDestroy(std::shared_ptr<Robot> robot);
};


#endif // ROBOTS_H
