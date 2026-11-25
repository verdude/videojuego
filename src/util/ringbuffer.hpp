#pragma once

#include <array>

class RingBuffer<typename T, size_t N> {
private:
  size_t start = 0;
  size_t count = 0;
  std::array<T, N> mem{};

public:
  void
  push(T item)
  {
    size_t i = (start+count)%N;
    mem[i] = item;
    if (count < N) {
      ++count;
    } else {
      start = (start+1)%N;
    }
  }

  const T&
  at(size_t i) const
  {
    return mem[(start+i)%N];
  }

  class Iterator {
    private:
      const RingBuffer& b;
      size_t i;

    public:
      Iterator(const RingBuffer& b_, size_t i_) : b(b_), i(i_) {}

      const T&
      operator*() const
      {
        return b.at(i);
      }

      Iterator&
      operator++()
      {
        ++i;
        return *this;
      }

      bool
      operator!=(const Iterator& other) const
      {
        return i != other.i;
      }
  };

  Iterator
  begin()
  {
    return Iterator(*this, 0);
  }

  Iterator
  end()
  {
    return Iterator(*this, count);
  }
};
