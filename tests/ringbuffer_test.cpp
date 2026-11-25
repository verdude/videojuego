#include "util/ringbuffer.hpp"

#include <gtest/gtest.h>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {

template<std::size_t N>
std::vector<int>
contents(const RingBuffer<int, N>& buffer)
{
  std::vector<int> values;
  for (const auto& value : buffer) {
    values.push_back(value);
  }
  return values;
}

TEST(RingBufferTest, StartsEmpty)
{
  const RingBuffer<int, 3> buffer;
  EXPECT_TRUE(buffer.begin() == buffer.end());
  EXPECT_TRUE(contents(buffer).empty());
  EXPECT_THROW(buffer.at(0), std::out_of_range);
}

TEST(RingBufferTest, ReadsOnlyInsertedItemsInInsertionOrder)
{
  RingBuffer<int, 3> buffer;
  buffer.push(10);
  buffer.push(20);

  EXPECT_EQ(contents(buffer), (std::vector<int>{ 10, 20 }));
  EXPECT_EQ(buffer.at(0), 10);
  EXPECT_EQ(buffer.at(1), 20);
  EXPECT_THROW(buffer.at(2), std::out_of_range);
}

TEST(RingBufferTest, FullBufferRetainsAllItemsUntilNextPush)
{
  RingBuffer<int, 3> buffer;
  buffer.push(10);
  buffer.push(20);
  buffer.push(30);

  EXPECT_TRUE(buffer.begin() != buffer.end());
  EXPECT_EQ(contents(buffer), (std::vector<int>{ 10, 20, 30 }));
  EXPECT_THROW(buffer.at(3), std::out_of_range);
}

TEST(RingBufferTest, RepeatedWraparoundRetainsNewestItemsInOrder)
{
  RingBuffer<int, 3> buffer;
  buffer.push(1);
  buffer.push(2);
  buffer.push(3);

  for (int value = 4; value <= 20; ++value) {
    buffer.push(value);
    EXPECT_EQ(contents(buffer),
              (std::vector<int>{ value - 2, value - 1, value }));
    EXPECT_EQ(buffer.at(0), value - 2);
    EXPECT_EQ(buffer.at(1), value - 1);
    EXPECT_EQ(buffer.at(2), value);
    EXPECT_THROW(buffer.at(3), std::out_of_range);
  }
  EXPECT_THROW(buffer.at(std::numeric_limits<std::size_t>::max()),
               std::out_of_range);
}

TEST(RingBufferTest, CapacityOneAlwaysContainsLatestItem)
{
  RingBuffer<int, 1> buffer;
  for (int value : { 10, 20, 30 }) {
    buffer.push(value);
    EXPECT_EQ(contents(buffer), (std::vector<int>{ value }));
    EXPECT_EQ(buffer.at(0), value);
    EXPECT_THROW(buffer.at(1), std::out_of_range);
  }
}

TEST(RingBufferTest, IteratorsCompareBothBufferAndPosition)
{
  RingBuffer<int, 2> first;
  RingBuffer<int, 2> second;
  first.push(10);
  first.push(20);
  second.push(10);
  second.push(20);

  EXPECT_TRUE(first.begin() != second.begin());
  EXPECT_TRUE(first.end() != second.end());

  auto iterator = first.begin();
  const auto copy = iterator;
  EXPECT_TRUE(iterator == copy);
  EXPECT_EQ(*iterator, 10);
  EXPECT_EQ(&++iterator, &iterator);
  EXPECT_TRUE(iterator != copy);
  EXPECT_EQ(*iterator, 20);
  ++iterator;
  EXPECT_TRUE(iterator == first.end());
}

TEST(RingBufferTest, AcceptsMoveOnlyItems)
{
  RingBuffer<std::unique_ptr<int>, 2> buffer;
  buffer.push(std::make_unique<int>(10));
  buffer.push(std::make_unique<int>(20));
  buffer.push(std::make_unique<int>(30));

  ASSERT_NE(buffer.at(0), nullptr);
  ASSERT_NE(buffer.at(1), nullptr);
  EXPECT_EQ(*buffer.at(0), 20);
  EXPECT_EQ(*buffer.at(1), 30);
}

} // namespace
