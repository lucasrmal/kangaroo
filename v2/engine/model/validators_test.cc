#include "model/validators.h"

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "model/object-manager.h"
#include "model/proto/institution.pb.h"

namespace kangaroo::model {
namespace {

using InstitutionNameIndex = ObjectIndex<Institution, &Institution::name>;

class TestObjectManager : public ObjectManager<Institution> {
 public:
  std::vector<ObjectIndexT*> Indexes() { return {&name_index_}; }
  std::vector<const ObjectValidatorT*> Validators() const {
    return {&nonempty_name_validator_, &unique_name_validator_};
  }

  RequiredValidator<Institution, &Institution::name> nonempty_name_validator_ =
      {"name"};
  UniqueValidator<Institution, &Institution::name> unique_name_validator_ = {
      "name", &name_index_};
  InstitutionNameIndex name_index_;
};

TEST(IndexedVector, RequiredValidatorFails) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("");
  EXPECT_FALSE(manager.Insert(std::move(inst)).ok());
}

TEST(IndexedVector, RequiredValidatorSuccess) {
  TestObjectManager manager;
  auto inst = std::make_unique<Institution>();
  inst->set_name("A");
  EXPECT_TRUE(manager.Insert(std::move(inst)).ok());
}

TEST(IndexedVector, UniqueValidatorFails) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  EXPECT_TRUE(manager.Insert(std::move(inst_a)).ok());

  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("A");
  EXPECT_FALSE(manager.Insert(std::move(inst_b)).ok());
}

TEST(IndexedVector, UniqueValidatorSuccess) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  EXPECT_TRUE(manager.Insert(std::move(inst_a)).ok());

  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  EXPECT_TRUE(manager.Insert(std::move(inst_b)).ok());
}

TEST(IndexedVector, UniqueValidatorUpdateFails) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  EXPECT_TRUE(manager.Insert(std::move(inst_a)).ok());

  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  EXPECT_TRUE(manager.Insert(std::move(inst_b)).ok());

  Institution update;
  update.set_id(1);
  update.set_name("A");
  EXPECT_FALSE(manager.Update(update).ok());
}

TEST(IndexedVector, UniqueValidatorUpdateSuccess) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  EXPECT_TRUE(manager.Insert(std::move(inst_a)).ok());

  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  EXPECT_TRUE(manager.Insert(std::move(inst_b)).ok());

  Institution update;
  update.set_id(1);
  update.set_name("C");
  EXPECT_TRUE(manager.Update(update).ok());
}

TEST(IndexedVector, UniqueValidatorUpdateToSameSuccess) {
  TestObjectManager manager;
  auto inst_a = std::make_unique<Institution>();
  inst_a->set_name("A");
  EXPECT_TRUE(manager.Insert(std::move(inst_a)).ok());

  auto inst_b = std::make_unique<Institution>();
  inst_b->set_name("B");
  EXPECT_TRUE(manager.Insert(std::move(inst_b)).ok());

  Institution update;
  update.set_id(1);
  update.set_name("B");
  EXPECT_TRUE(manager.Update(update).ok());
}

}  // namespace
}  // namespace kangaroo::model