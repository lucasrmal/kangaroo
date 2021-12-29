#include "model/price-manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

namespace kangaroo::model {
namespace {

using ::testing::ElementsAre;
using ::testing::Pair;

TEST(PriceManagerTest, EmptyPriceManager) {
  PriceManager prices;
  EXPECT_EQ(prices.Get("AAA", "BBB"), nullptr);
}

TEST(PriceManagerTest, GetOrCreatePair) {
  PriceManager prices;
  EXPECT_NE(prices.GetOrCreate("AAA", "BBB"), nullptr);

  // It can then be accessed with GetPair
  EXPECT_NE(prices.Get("AAA", "BBB"), nullptr);
}

TEST(PriceManagerTest, Remove) {
  PriceManager prices;
  ASSERT_NE(prices.GetOrCreate("AAA", "BBB"), nullptr);

  prices.Remove("AAA", "BBB");

  // It can then be accessed with GetPair
  EXPECT_EQ(prices.Get("AAA", "BBB"), nullptr);
}

TEST(PricePairTest, LastestPriceEmpty) {
  PricePair pair;
  EXPECT_EQ(pair.Latest(), 0.);
}

TEST(PricePairTest, InsertOrUpdate) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  EXPECT_EQ(pair.Latest(), 1.2);
}

TEST(PricePairTest, InsertOrUpdateNormalizes) {
  PricePair pair;
  pair.InsertOrUpdate(20201032, 1.2);
  EXPECT_EQ(pair.Get(20201101), 1.2);
}

TEST(PricePairTest, LatestPrice) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.5);
  EXPECT_EQ(pair.Latest(), 1.5);
}

TEST(PricePairTest, GetPrice) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.5);
  EXPECT_EQ(pair.Get(20201015), 1.5);
}

TEST(PricePairTest, GetNonexistentPrice) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);
  EXPECT_EQ(pair.Get(20201016), 0);
}

TEST(PricePairTest, Remove) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.Remove(20201015);
  EXPECT_EQ(pair.Get(20201015), 0);
}

TEST(PricePairTest, RemoveNormalizes) {
  PricePair pair;
  pair.InsertOrUpdate(20201101, 1.3);
  pair.Remove(20201032);
  EXPECT_EQ(pair.Get(20201101), 0);
}

TEST(PricePairTest, LatestAt) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);
  EXPECT_EQ(pair.LatestAt(20201015), 1.3);
}

TEST(PricePairTest, LatestAt_Before) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);
  EXPECT_EQ(pair.LatestAt(20201012), 0);
}

TEST(PricePairTest, LatestAt_Begin) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);
  EXPECT_EQ(pair.LatestAt(20201014), 1.2);
}

TEST(PricePairTest, LatestAt_End) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);
  EXPECT_EQ(pair.LatestAt(20201020), 1.5);
}

TEST(PricePairTest, LatestAt_PastEnd) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);
  EXPECT_EQ(pair.LatestAt(20201021), 1.5);
}

TEST(PricePairTest, LatestAt_InBetween) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);
  EXPECT_EQ(pair.LatestAt(20201016), 1.3);
}

std::vector<std::pair<date_t, double>> RangeToVector(
    std::pair<PricePair::const_iterator, PricePair::const_iterator>
        range_pair) {
  std::vector<std::pair<date_t, double>> range_vector;
  for (auto it = range_pair.first; it != range_pair.second; ++it) {
    range_vector.push_back(*it);
  }
  return range_vector;
}

TEST(PricePairTest, Between) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);

  EXPECT_THAT(RangeToVector(pair.Between(20201015, 20201019)),
              ElementsAre(Pair(20201018, 1.4), Pair(20201015, 1.3)));
}

TEST(PricePairTest, SingleDate) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);

  EXPECT_THAT(RangeToVector(pair.Between(20201015, 20201015)),
              ElementsAre(Pair(20201015, 1.3)));
}

TEST(PricePairTest, SingleNonexistentDate) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);

  EXPECT_THAT(RangeToVector(pair.Between(20201016, 20201016)),
              ::testing::IsEmpty());
}

TEST(PricePairTest, NonexistentDateToValid) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);

  EXPECT_THAT(RangeToVector(pair.Between(20201016, 20201018)),
              ElementsAre(Pair(20201018, 1.4)));
}

TEST(PricePairTest, DateToPastEnd) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);

  EXPECT_THAT(RangeToVector(pair.Between(20201016, 20201021)),
              ElementsAre(Pair(20201020, 1.5), Pair(20201018, 1.4)));
}

TEST(PricePairTest, PreBeginToDate) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);

  EXPECT_THAT(RangeToVector(pair.Between(20201012, 20201015)),
              ElementsAre(Pair(20201015, 1.3), Pair(20201014, 1.2)));
}

TEST(PricePairTest, BetweenInvalidDates) {
  PricePair pair;
  pair.InsertOrUpdate(20201014, 1.2);
  pair.InsertOrUpdate(20201015, 1.3);
  pair.InsertOrUpdate(20201018, 1.4);
  pair.InsertOrUpdate(20201020, 1.5);

  EXPECT_THAT(RangeToVector(pair.Between(20201015, 20201012)),
              ::testing::IsEmpty());
}

}  // namespace
}  // namespace kangaroo::model