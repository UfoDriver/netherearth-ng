#include <algorithm>


template <class T, class V>
int find_index(const T& container, const V& value)
{
  // @TODO: comparing addresses is wrong, should be fixed
  auto pos = std::find(container.cbegin(), container.cend(), value);
  if (pos != container.cend()) {
    return std::distance(container.cbegin(), pos);
  } else {
    return -1;
  }
}
