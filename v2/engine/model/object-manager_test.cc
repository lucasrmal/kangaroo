#include "model/object-manager.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "model/proto/institution.pb.h"

namespace kangaroo::model {
namespace {

using InstitutionNameIndex = ObjectIndex<Institution, &Institution::name>;

class TestObjectManager : public ObjectManager<Institution> {
 public:
  absl::Status ValidateInsert(const Institution& inst) const override {
    if (inst.name() == "NOINSERT") {
      return absl::InvalidArgumentError("Can't insert");
    }

    return absl::OkStatus();
  }

  absl::Status ValidateUpdate(const Institution& existing,
                              const Institution& updated) const override {
    if (updated.name() == "NOUPDATE") {
      return absl::InvalidArgumentError("Can't update");
    }

    return absl::OkStatus();
  }

  absl::Status ValidateRemove(const Institution& inst) const override {
    if (inst.name() == "DONOTREMOVE") {
      return absl::InvalidArgumentError("Can't remove DONOTREMOVE");
    }

    return absl::OkStatus();
  }

  std::vector<ObjectIndexT*> Indexes() override { return {&name_index_}; }
  std::vector<const ObjectValidatorT*> Validators() const override {
    return {};
  }

  InstitutionNameIndex name_index_;
};

TEST(IndexedVector, Insert) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("A");
  auto retval = manager.Insert(std::move(inst));
  ASSERT_TRUE(retval.ok());
  EXPECT_EQ(*retval, 0);
  EXPECT_EQ(manager.size(), 1);
}

TEST(IndexedVector, NoInsertInvalid) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("NOINSERT");
  auto retval = manager.Insert(std::move(inst));
  EXPECT_FALSE(retval.ok());
  EXPECT_EQ(manager.size(), 0);
}

TEST(IndexedVector, Get) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  auto retval = manager.Insert(std::move(inst_a));
  ASSERT_TRUE(retval.ok());
  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  retval = manager.Insert(std::move(inst_b));
  ASSERT_TRUE(retval.ok());

  auto fetched = manager.Get(0);
  ASSERT_NE(fetched, nullptr);
  EXPECT_EQ(0, fetched->id());
  EXPECT_EQ("A", fetched->name());

  fetched = manager.Get(1);
  ASSERT_NE(fetched, nullptr);
  EXPECT_EQ(1, fetched->id());
  EXPECT_EQ("B", fetched->name());
}

TEST(IndexedVector, GetNonexistentFails) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("A");
  auto retval = manager.Insert(std::move(inst));
  ASSERT_TRUE(retval.ok());
  EXPECT_EQ(manager.Get(1), nullptr);
}

TEST(IndexedVector, Iterate) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  auto retval = manager.Insert(std::move(inst_a));
  ASSERT_TRUE(retval.ok());
  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  retval = manager.Insert(std::move(inst_b));
  ASSERT_TRUE(retval.ok());

  auto [iterator, end] = manager.Objects();
  int num = 0;
  for (; iterator != end; ++iterator) ++num;
  EXPECT_EQ(num, 2);
}

TEST(IndexedVector, Size) {
  TestObjectManager manager;
  EXPECT_EQ(manager.size(), 0);
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  auto retval = manager.Insert(std::move(inst_a));
  ASSERT_TRUE(retval.ok());
  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  retval = manager.Insert(std::move(inst_b));
  ASSERT_TRUE(retval.ok());
  EXPECT_EQ(manager.size(), 2);
}

TEST(IndexedVector, UpdateOk) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("A");
  auto retval = manager.Insert(std::move(inst));
  ASSERT_TRUE(retval.ok());

  Institution updated;
  updated.set_id(0);
  updated.set_name("B");
  ASSERT_TRUE(manager.Update(updated).ok());

  auto fetched = manager.Get(0);
  ASSERT_NE(fetched, nullptr);
  EXPECT_EQ(0, fetched->id());
  EXPECT_EQ("B", fetched->name());
}

TEST(IndexedVector, UpdateNonexistentFails) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("A");
  auto retval = manager.Insert(std::move(inst));
  ASSERT_TRUE(retval.ok());

  Institution updated;
  updated.set_id(1);
  updated.set_name("B");
  EXPECT_FALSE(manager.Update(updated).ok());
}

TEST(IndexedVector, UpdateInvalidFails) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("A");
  auto retval = manager.Insert(std::move(inst));
  ASSERT_TRUE(retval.ok());

  Institution updated;
  updated.set_id(0);
  updated.set_name("NOUPDATE");
  EXPECT_FALSE(manager.Update(updated).ok());
}

TEST(IndexedVector, RemoveOk) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("A");
  auto retval = manager.Insert(std::move(inst));
  ASSERT_TRUE(retval.ok());

  EXPECT_TRUE(manager.Remove(0).ok());
  EXPECT_EQ(manager.size(), 0);
}

TEST(IndexedVector, RemoveNonexistentFails) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("A");
  auto retval = manager.Insert(std::move(inst));
  ASSERT_TRUE(retval.ok());

  EXPECT_FALSE(manager.Remove(1).ok());
  EXPECT_EQ(manager.size(), 1);
}

TEST(IndexedVector, RemoveForbiddenFails) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("DONOTREMOVE");
  auto retval = manager.Insert(std::move(inst));
  ASSERT_TRUE(retval.ok());

  EXPECT_FALSE(manager.Remove(0).ok());
  EXPECT_EQ(manager.size(), 1);
}

TEST(IndexedVector, IndexAdds) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  ASSERT_TRUE(manager.Insert(std::move(inst_a)).ok());
  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  ASSERT_TRUE(manager.Insert(std::move(inst_b)).ok());
  auto inst_c = std::make_unique<Institution>();
  inst_c->set_name("B");
  ASSERT_TRUE(manager.Insert(std::move(inst_c)).ok());

  auto all_a = manager.name_index_.FindAll("A");
  EXPECT_EQ(all_a.size(), 1);

  auto all_b = manager.name_index_.FindAll("B");
  EXPECT_EQ(all_b.size(), 2);
}

TEST(IndexedVector, IndexUpdates) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  ASSERT_TRUE(manager.Insert(std::move(inst_a)).ok());
  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  ASSERT_TRUE(manager.Insert(std::move(inst_b)).ok());
  auto inst_c = std::make_unique<Institution>();
  inst_c->set_name("B");
  ASSERT_TRUE(manager.Insert(std::move(inst_c)).ok());

  Institution updated;
  updated.set_id(1);
  updated.set_name("A");
  ASSERT_TRUE(manager.Update(updated).ok());

  auto all_a = manager.name_index_.FindAll("A");
  EXPECT_EQ(all_a.size(), 2);

  auto all_b = manager.name_index_.FindAll("B");
  EXPECT_EQ(all_b.size(), 1);
}

TEST(IndexedVector, IndexRemoves) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  ASSERT_TRUE(manager.Insert(std::move(inst_a)).ok());
  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  ASSERT_TRUE(manager.Insert(std::move(inst_b)).ok());
  auto inst_c = std::make_unique<Institution>();
  inst_c->set_name("B");
  ASSERT_TRUE(manager.Insert(std::move(inst_c)).ok());

  ASSERT_TRUE(manager.Remove(0).ok());

  auto all_a = manager.name_index_.FindAll("A");
  EXPECT_EQ(all_a.size(), 0);
}

TEST(IndexedVector, IndexFindOneReturnsAny) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  ASSERT_TRUE(manager.Insert(std::move(inst_a)).ok());
  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("A");

  EXPECT_NE(manager.name_index_.FindOne("A"), nullptr);
  EXPECT_EQ(manager.name_index_.FindOne("A")->name(), "A");
}

TEST(IndexedVector, IndexFindNonexistentReturnsNullptr) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");

  EXPECT_EQ(manager.name_index_.FindOne("B"), nullptr);
}

TEST(IndexedVector, IndexContains) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  ASSERT_TRUE(manager.Insert(std::move(inst_a)).ok());
  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("A");

  EXPECT_TRUE(manager.name_index_.Contains("A"));
}

TEST(IndexedVector, EmptyIndex) {
  TestObjectManager manager;
  EXPECT_FALSE(manager.name_index_.Contains("A"));
}

}  // namespace
}  // namespace kangaroo::model
