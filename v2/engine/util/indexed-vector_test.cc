#include "util/indexed-vector.h"

#include <string>

#include <gtest/gtest.h>

namespace kangaroo {
namespace {

struct TestElement {
  TestElement(const std::string& value) : value_(value) {}
  TestElement(int64_t id, const std::string& value) : id_(id), value_(value) {}

  int64_t id() const { return id_; }
  void set_id(int64_t id) { id_ = id; }
  std::string value() const { return value_; }
  int64_t id_ = 0;
  std::string value_;
};

TEST(IndexedVector, GetNonExisting) {
  IndexedVector<TestElement> index;
  int64_t idx = index.Insert(std::make_unique<TestElement>("A"));
  EXPECT_EQ(nullptr, index.Get(idx+1));
}

TEST(IndexedVector, InsertElement) {
  IndexedVector<TestElement> index;
  int64_t idx = index.Insert(std::make_unique<TestElement>(-1, "A"));
  EXPECT_EQ(idx, index.Get(idx)->id());
  EXPECT_EQ("A", index.Get(idx)->value());
}

TEST(IndexedVector, Size) {
  IndexedVector<TestElement> index;
  EXPECT_EQ(index.size(), 0);
  int64_t idx = index.Insert(std::make_unique<TestElement>("A"));
  EXPECT_EQ(index.size(), 1);
  index.Remove(idx);
  EXPECT_EQ(index.size(), 0);
}

TEST(IndexedVector, NextIdKeepsIncreasing) {
  IndexedVector<TestElement> index;
  EXPECT_EQ(index.next_id(), 0);
  int64_t idx = index.Insert(std::make_unique<TestElement>("A"));
  EXPECT_EQ(index.next_id(), 1);
  index.Remove(idx);
  EXPECT_EQ(index.next_id(), 1);
  idx = index.Insert(std::make_unique<TestElement>("A"));
  EXPECT_EQ(index.next_id(), 2);
}

TEST(IndexedVector, InsertMultipleElements) {
  IndexedVector<TestElement> index;
  int64_t idx_a = index.Insert(std::make_unique<TestElement>("A"));
  int64_t idx_b = index.Insert(std::make_unique<TestElement>("B"));
  EXPECT_NE(idx_a, idx_b);
}

TEST(IndexedVector, GetAndUpdate) {
  IndexedVector<TestElement> index;
  int64_t idx = index.Insert(std::make_unique<TestElement>("A"));
  index.Get(idx)->value_ = "B";
  EXPECT_EQ("B", index.Get(idx)->value());
}

TEST(IndexedVector, Remove) {
  IndexedVector<TestElement> index;
  int64_t idx = index.Insert(std::make_unique<TestElement>("A"));
  EXPECT_NE(index.Get(idx), nullptr);
  EXPECT_TRUE(index.Remove(idx));
  EXPECT_EQ(index.Get(idx), nullptr);
}

TEST(IndexedVector, Load) {
  IndexedVector<TestElement> index;

  std::vector<std::unique_ptr<TestElement>> new_elements;
  new_elements.push_back(std::make_unique<TestElement>(1, "A"));
  new_elements.push_back(std::make_unique<TestElement>(2, "B"));
  index.Load(&new_elements);

  EXPECT_EQ(2, index.size());
  EXPECT_EQ(index.next_id(), 3);
  EXPECT_EQ("A", index.Get(1)->value());
  EXPECT_EQ("B", index.Get(2)->value());
}

TEST(IndexedVector, LoadClearsMap) {
  IndexedVector<TestElement> index;
  index.Insert(std::make_unique<TestElement>("A"));
  index.Insert(std::make_unique<TestElement>("B"));
  index.Insert(std::make_unique<TestElement>("C"));

  std::vector<std::unique_ptr<TestElement>> new_elements;
  new_elements.push_back(std::make_unique<TestElement>(1, "A"));
  index.Load(&new_elements);

  EXPECT_EQ(1, index.size());
  EXPECT_EQ(index.next_id(), 2);
}

}  // namespace
}  // namespace kangaroo

