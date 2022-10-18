#pragma once
#include <functional>

class Hash
{
public:
  template <class T>
  static inline void combine(std::size_t &seed, const T &value)
  {
    std::hash<T> hasher;
    seed ^= hasher(value) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
  }
};
