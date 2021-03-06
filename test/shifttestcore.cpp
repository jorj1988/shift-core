#include "shifttest.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/TypeInformation/smodule.h"
#include "shift/Properties/sdata.inl"

xCompileTimeAssert(sizeof(Shift::Attribute) == sizeof(void*));
xCompileTimeAssert(sizeof(Shift::Property) <= (sizeof(Shift::Attribute) + sizeof(void*) * 4));

S_IMPLEMENT_PROPERTY(TestVector, Test)

void TestVector::createTypeInformation(
    Shift::PropertyInformationTyped<TestVector> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  auto childBlock = info->createChildrenBlock(data);

  childBlock.add(&TestVector::x, "x");
  childBlock.add(&TestVector::y, "y");
  childBlock.add(&TestVector::z, "z");
  }

S_IMPLEMENT_PROPERTY(TestEntity, Test)

void TestEntity::createTypeInformation(
    Shift::PropertyInformationTyped<TestEntity> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  auto childBlock = info->createChildrenBlock(data);

  auto a = childBlock.add(&TestEntity::in, "in");
  auto b = childBlock.add(&TestEntity::reciprocal, "reciprocal");

  auto affects = childBlock.createAffects(&b, 1);

  a->setAffects(affects, true);
  b->setCompute([](TestEntity *ent)
    {
    ent->reciprocal.x.computeLock() = 1.0f / ent->in.x();
    QCOMPARE(ent->reciprocal.x.isDirty(), false);
    ent->reciprocal.y.computeLock() = 1.0f / ent->in.y();
    QCOMPARE(ent->reciprocal.y.isDirty(), false);
    ent->reciprocal.z.computeLock() = 1.0f / ent->in.z();
    QCOMPARE(ent->reciprocal.z.isDirty(), false);

    QCOMPARE(ent->reciprocal.x.isDirty(), false);
    QCOMPARE(ent->reciprocal.y.isDirty(), false);
    QCOMPARE(ent->reciprocal.z.isDirty(), false);

    ++ent->evaluationCount;
    });
  }

S_IMPLEMENT_PROPERTY(TestIndexedEntity, Test)

void TestIndexedEntity::createTypeInformation(
    Shift::PropertyInformationTyped<TestIndexedEntity> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  auto childBlock = info->createChildrenBlock(data);

  childBlock.add(&TestIndexedEntity::testArray, "testArray");
  }

S_IMPLEMENT_TEST
QTEST_APPLESS_MAIN(ShiftCoreTest)
