#include "model/types/date.h"

#include <gtest/gtest.h>

#include "absl/time/civil_time.h"

namespace kangaroo {
namespace {

TEST(DateTest, ToCivilTime) {
  absl::CivilDay day = date::ToCivilDay(20211215);

  EXPECT_EQ(day.year(), 2021);
  EXPECT_EQ(day.month(), 12);
  EXPECT_EQ(day.day(), 15);
}

TEST(DateTest, ToCivilTimeOutOfBoundSkipped) {
  absl::CivilDay day = date::ToCivilDay(1120211215);

  EXPECT_EQ(day.year(), 2021);
  EXPECT_EQ(day.month(), 12);
  EXPECT_EQ(day.day(), 15);
}

TEST(DateTest, ToCivilTimeTooShortSkipped) {
  absl::CivilDay day = date::ToCivilDay(11215);

  EXPECT_EQ(day.year(), 1);
  EXPECT_EQ(day.month(), 12);
  EXPECT_EQ(day.day(), 15);
}

TEST(DateTest, ToCivilTimeAllZero) {
  absl::CivilDay day = date::ToCivilDay(0);

  EXPECT_EQ(day.year(), -1);
  EXPECT_EQ(day.month(), 11);
  EXPECT_EQ(day.day(), 30);
}

TEST(DateTest, FromCivilDay) {
  absl::CivilDay day(2021, 12, 15);
  EXPECT_EQ(date::FromCivilDay(day), 20211215);
}

TEST(DateTest, FromCivilDayZeroYear) {
  absl::CivilDay day(0, 12, 15);
  EXPECT_EQ(date::FromCivilDay(day), 1215);
}

TEST(DateTest, FromCivilDayZeroMonth) {
  absl::CivilDay day(2020, 0, 15);
  EXPECT_EQ(date::FromCivilDay(day), 20191215);
}

TEST(DateTest, NormalizeTooLong) {
  EXPECT_EQ(date::Normalize(1120211215), 20211215);
}

TEST(DateTest, NormalizeZeroMonth) {
  EXPECT_EQ(date::Normalize(20200015), 20191215);
}

}  // namespace
}  // namespace kangaroo