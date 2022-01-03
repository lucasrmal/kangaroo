#include "model/account-manager.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "model/object-manager.h"
#include "model/proto/account.pb.h"

namespace kangaroo::model {
namespace {

using ::testing::UnorderedElementsAre;

MATCHER_P(AccountId, id, "") { return arg->id() == id; }

std::unique_ptr<Account> MakeValidAssetAccount(const std::string& name,
                                               int64_t parent_id) {
  auto account = std::make_unique<Account>();
  account->set_name(name);
  account->set_parent_id(parent_id);
  account->set_type(Account::ASSET);
  return account;
}

struct AccountHierarchy {
  AccountHierarchy(AccountManager* manager) {
    a_id = manager->Insert(MakeValidAssetAccount("A", manager->Root()->id()))
               .value();
    b_id = manager->Insert(MakeValidAssetAccount("B", manager->Root()->id()))
               .value();
    ba_id = manager->Insert(MakeValidAssetAccount("BA", b_id)).value();
    bb_id = manager->Insert(MakeValidAssetAccount("BB", b_id)).value();
    bba_id = manager->Insert(MakeValidAssetAccount("BBA", bb_id)).value();
    bbb_id = manager->Insert(MakeValidAssetAccount("BBB", bb_id)).value();
    bbc_id = manager->Insert(MakeValidAssetAccount("BBC", bb_id)).value();
    c_id = manager->Insert(MakeValidAssetAccount("C", manager->Root()->id()))
               .value();
    ca_id = manager->Insert(MakeValidAssetAccount("CA", c_id)).value();
  }
  int64_t a_id;
  int64_t b_id;
  int64_t ba_id;
  int64_t bb_id;
  int64_t bba_id;
  int64_t bbb_id;
  int64_t bbc_id;
  int64_t c_id;
  int64_t ca_id;
};

TEST(AccountManagerTest, NewManagerHasRoot) {
  AccountManager manager;
  ASSERT_NE(manager.Root(), nullptr);
  EXPECT_EQ(manager.Root()->type(), Account::ROOT);
}

TEST(IndexedVector, RetrieveChildren) {
  // Hierarchy:
  // TOPLEVEL
  //  -> A
  //  -> B
  //     -> BA
  //     -> BB
  //        -> BBA
  //        -> BBB
  //  -> C
  //     -> CA

  AccountManager manager;
  AccountHierarchy accounts(&manager);
  EXPECT_THAT(
      manager.GetChildren(*(manager.Root())),
      UnorderedElementsAre(AccountId(accounts.a_id), AccountId(accounts.b_id),
                           AccountId(accounts.c_id)));
}

TEST(IndexedVector, IsAncestor) {
  // Hierarchy:
  // TOPLEVEL
  //  -> A
  //  -> B
  //     -> BA
  //     -> BB
  //        -> BBA
  //        -> BBB
  //  -> C
  //     -> CA

  AccountManager manager;
  AccountHierarchy accounts(&manager);

  EXPECT_TRUE(manager.IsAncestorOf(manager.Root()->id(), /*of=*/accounts.a_id));
  EXPECT_TRUE(manager.IsAncestorOf(accounts.b_id, /*of=*/accounts.ba_id));
  EXPECT_TRUE(manager.IsAncestorOf(accounts.b_id, /*of=*/accounts.bbb_id));

  EXPECT_FALSE(
      manager.IsAncestorOf(manager.Root()->id(), /*of=*/manager.Root()->id()));
  EXPECT_FALSE(manager.IsAncestorOf(accounts.b_id, /*of=*/accounts.b_id));
  EXPECT_FALSE(manager.IsAncestorOf(accounts.a_id, /*of=*/accounts.b_id));
  EXPECT_FALSE(manager.IsAncestorOf(accounts.ba_id, /*of=*/accounts.b_id));

  EXPECT_TRUE(manager.IsDescendentOf(accounts.ba_id, /*of=*/accounts.b_id));
  EXPECT_TRUE(manager.IsDescendentOf(accounts.bbc_id, /*of=*/accounts.b_id));
  EXPECT_TRUE(
      manager.IsDescendentOf(accounts.bbc_id, /*of=*/manager.Root()->id()));
  EXPECT_FALSE(manager.IsDescendentOf(accounts.bb_id, /*of=*/accounts.bb_id));
  EXPECT_FALSE(manager.IsDescendentOf(accounts.b_id, /*of=*/accounts.bba_id));
}

TEST(IndexedVector, ColonSeparatedPath) {
  // Hierarchy:
  // TOPLEVEL
  //  -> A
  //  -> B
  //     -> BA
  //     -> BB
  //        -> BBA
  //        -> BBB
  //  -> C
  //     -> CA

  AccountManager manager;
  AccountHierarchy accounts(&manager);

  EXPECT_EQ(manager.ColonSeparatedPath(*manager.Root()), "");
  EXPECT_EQ(manager.ColonSeparatedPath(*manager.Get(accounts.bbc_id), 0), "");
  EXPECT_EQ(manager.ColonSeparatedPath(*manager.Get(accounts.bbc_id)),
            "B:BB:BBC");
  EXPECT_EQ(manager.ColonSeparatedPath(*manager.Get(accounts.bbc_id), -1),
            "B:BB:BBC");
  EXPECT_EQ(
      manager.ColonSeparatedPath(*manager.Get(accounts.bbc_id), /*length=*/2),
      "BB:BBC");
  EXPECT_EQ(
      manager.ColonSeparatedPath(*manager.Get(accounts.bbc_id), /*length=*/23),
      "B:BB:BBC");
}

}  // namespace
}  // namespace kangaroo::model
