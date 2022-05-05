#ifndef UTILS_H
#define UTILS_H

#include <algorithm>


// https://codereview.stackexchange.com/questions/165905/c-circular-buffer-through-circular-iterator
template <class BaseIter>
class CircularIterator: public BaseIter
{
public:
  CircularIterator(BaseIter b, BaseIter e)
    : BaseIter(b), begin{b}, end {e} {}

  CircularIterator& operator++() {
    BaseIter::operator++();
    if (*this == end)
      BaseIter::operator=(begin);
    return *this;
  }

  const CircularIterator operator++(int)
  {
    const auto oldValue = *this;
    this->operator++();
    return oldValue;
  }
  CircularIterator& operator--(void) = delete;
  const CircularIterator operator--(int) = delete;

private:
  BaseIter begin;
  BaseIter end;
};


// Function name is really awful
template <typename Container, typename Value>
typename Container::const_iterator
find_next_pair_looped(const Container& container, Value value)
{
  typename Container::const_iterator found_value =
    std::find_if(container.cbegin(), container.cend(),
                 [value](const auto& pair) {
                   return pair.first == value;
                 });
  return found_value == container.cend() ? container.begin() : found_value;
}

#endif // UTILS_H
