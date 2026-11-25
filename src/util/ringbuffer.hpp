#pragma once

#include <array>
#include <cstdint>
#include <stdexcept>
#include <utility>

// A fixed-capacity history, read from oldest to newest. Pushing into a full
// buffer replaces the oldest item and invalidates existing iterators.
template<typename T, std::size_t N>
class RingBuffer
{
  static_assert(N > 0, "RingBuffer capacity must be greater than zero");

private:
  std::size_t start = 0;
  std::size_t count = 0;
  std::array<T, N> mem{};

public:
  void push(T item)
  {
    std::size_t i = (start + count) % N;
    mem[i] = std::move(item);
    if (count < N) {
      ++count;
    } else {
      start = (start + 1) % N;
    }
  }

  const T& at(std::size_t i) const
  {
    if (i >= count) {
      throw std::out_of_range("RingBuffer index out of range");
    }
    return mem[(start + i) % N];
  }

  class Iterator
  {
  private:
    const RingBuffer& b;
    std::size_t i;

  public:
    Iterator(const RingBuffer& b_, std::size_t i_)
      : b(b_)
      , i(i_)
    {
    }

    const T& operator*() const { return b.at(i); }

    Iterator& operator++()
    {
      ++i;
      return *this;
    }

    bool operator==(const Iterator& other) const
    {
      return &b == &other.b && i == other.i;
    }

    bool operator!=(const Iterator& other) const { return !(*this == other); }
  };

  Iterator begin() const { return Iterator(*this, 0); }

  Iterator end() const { return Iterator(*this, count); }
};
