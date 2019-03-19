#ifndef ROBOTS_H
#define ROBOTS_H

#include <algorithm>
#include <vector>

class Robot;


// @TODO PlayerOnly/PlayerOnly::iterator rework needed
class PlayerOnly {
public:
  class iterator {
  public:
    iterator(unsigned short player,
             std::vector<Robot *>::iterator begin_,
             std::vector<Robot *>::iterator end_);
    explicit iterator(std::vector<Robot *>::iterator end_) : player {0}, internal{end_} {}

    iterator& operator++();
    bool operator!=(const iterator& other);
    Robot* operator*();

  private:
    unsigned short player;
    std::vector<Robot *>::iterator internal;
    std::vector<Robot *>::iterator end_;
  };

  explicit PlayerOnly(unsigned short player,
                      std::vector<Robot *>::iterator begin_,
                      std::vector<Robot *>::iterator end_)
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
  std::vector<Robot *>::iterator begin_;
  std::vector<Robot *>::iterator end_;
};


// @TODO std::unique_ptr?
class Robots : public std::vector<Robot *> {
public:
  int getRobotCount(unsigned short int owner);
  PlayerOnly forPlayer(unsigned short player)
  {
    return PlayerOnly(player, begin(), end());
  }
  int findIndex(const Robot* robot);
  void findAndDestroy(Robot* robot);
};


#endif // ROBOTS_H
