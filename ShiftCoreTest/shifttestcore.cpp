#include "shifttest.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/TypeInformation/spropertygroup.h"

namespace Test
{
Shift::PropertyGroup &propertyGroup()
  {
  static Shift::PropertyGroup grp;
  return grp;
  }
}

S_IMPLEMENT_PROPERTY(TestVector, Test)

void TestVector::createTypeInformation(
    Shift::PropertyInformationTyped<TestVector> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto childBlock = info->createChildrenBlock(data);

    childBlock.add(&TestVector::x, "x");
    childBlock.add(&TestVector::y, "y");
    childBlock.add(&TestVector::z, "z");
    }
  }

S_IMPLEMENT_PROPERTY(TestEntity, Test)

void TestEntity::createTypeInformation(
    Shift::PropertyInformationTyped<TestEntity> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  if(data.registerAttributes)
    {
    auto childBlock = info->createChildrenBlock(data);

    auto a = childBlock.add(&TestEntity::in, "in");
    auto b = childBlock.add(&TestEntity::reciprocal, "reciprocal");

    auto affects = childBlock.createAffects(&b, 1);

    a->setAffects(affects, true);
    b->setCompute([](TestEntity *ent)
      {
      Shift::FloatProperty::ComputeLock(&ent->reciprocal.x) = 1.0f / ent->in.x();
      QCOMPARE(ent->reciprocal.x.isDirty(), false);
      Shift::FloatProperty::ComputeLock(&ent->reciprocal.y) = 1.0f / ent->in.y();
      QCOMPARE(ent->reciprocal.y.isDirty(), false);
      Shift::FloatProperty::ComputeLock(&ent->reciprocal.z) = 1.0f / ent->in.z();
      QCOMPARE(ent->reciprocal.z.isDirty(), false);

      QCOMPARE(ent->reciprocal.x.isDirty(), false);
      QCOMPARE(ent->reciprocal.y.isDirty(), false);
      QCOMPARE(ent->reciprocal.z.isDirty(), false);
      });
    }
  }

