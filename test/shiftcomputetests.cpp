#include "shifttest.h"
#include "shift/Properties/sdata.inl"

void ShiftCoreTest::simpleDirtyCompute()
  {
  TestDatabase db;

  // create an entity, everything should be dirty
  TestEntity* a = db.addChild<TestEntity>();
  QCOMPARE(a->evaluationCount, 0U);
  QCOMPARE(a->in.allDirty(), true);
  QCOMPARE(a->reciprocal.allDirty(), true);

  // get a->in, shouldnt change anything but itself
  a->in.preGet();
  QCOMPARE(a->in.countDirty(), 3U);

  // clean components, should only affect themselves
  a->in.x.preGet();
  QCOMPARE(a->in.countDirty(), 2U);
  a->in.y.preGet();
  QCOMPARE(a->in.countDirty(), 1U);
  a->in.z.preGet();
  QCOMPARE(a->in.countDirty(), 0U);


  // get reciprocal, should compute children
  QCOMPARE(a->evaluationCount, 0U);
  a->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 1U);
  QCOMPARE(a->in.anyDirty(), false);
  QCOMPARE(a->reciprocal.anyDirty(), false);



  TestEntity* b = db.addChild<TestEntity>();
  QCOMPARE(b->evaluationCount, 0U);
  QCOMPARE(b->in.allDirty(), true);
  QCOMPARE(b->reciprocal.allDirty(), true);

  b->in.preGet();
  QCOMPARE(b->evaluationCount, 0U);
  b->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 1U);
  QCOMPARE(b->evaluationCount, 1U);
  QCOMPARE(b->in.allDirty(), false);
  QCOMPARE(b->reciprocal.allDirty(), false);

  QCOMPARE(a->evaluationCount, 1U);
  QCOMPARE(b->evaluationCount, 1U);
  b->in.setInput(&a->reciprocal);
  QCOMPARE(b->evaluationCount, 1U);
  QCOMPARE(a->evaluationCount, 1U);
  QCOMPARE(a->in.anyDirty(), false);
  // after a connection, mainly due to efficient, the from and to are dirty, even thought only the to really requires it...
  QCOMPARE(a->reciprocal.allDirty(), true);
  QCOMPARE(b->in.allDirty(), true);

  // get b->in, shouldnt change anything but itself
  QCOMPARE(a->evaluationCount, 1U);
  QCOMPARE(b->evaluationCount, 1U);
  b->in.preGet();
  QCOMPARE(a->evaluationCount, 2U);
  QCOMPARE(b->evaluationCount, 1U);
  QCOMPARE(a->in.countDirty(), 0U);
  QCOMPARE(b->in.countDirty(), 0U);

  QCOMPARE(a->evaluationCount, 2U);
  QCOMPARE(b->evaluationCount, 1U);
  b->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 2U);
  QCOMPARE(b->evaluationCount, 2U);

  a->in.x = 1;
  QCOMPARE(a->in.countDirty(), 0U);
  QCOMPARE(b->in.countDirty(), 4U);
  a->in.y = 2;
  QCOMPARE(a->in.countDirty(), 0U);
  QCOMPARE(b->in.countDirty(), 4U);
  a->in.z = 3;
  QCOMPARE(a->in.countDirty(), 0U);
  QCOMPARE(b->in.countDirty(), 4U);

  QCOMPARE(a->evaluationCount, 2U);
  QCOMPARE(b->evaluationCount, 2U);
  b->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 3U);
  QCOMPARE(b->evaluationCount, 3U);

  a->in.x = 1;
  QCOMPARE(a->in.countDirty(), 0U);
  QCOMPARE(b->in.countDirty(), 4U);
  a->in.y = 2;
  QCOMPARE(a->in.countDirty(), 0U);
  QCOMPARE(b->in.countDirty(), 4U);
  a->in.z = 3;
  QCOMPARE(a->in.countDirty(), 0U);
  QCOMPARE(b->in.countDirty(), 4U);

  QCOMPARE(a->evaluationCount, 3U);

  // clean components, should siblings
  b->in.x.preGet();
  QCOMPARE(a->in.countDirty(), 0U);
  b->in.y.preGet();
  QCOMPARE(a->in.countDirty(), 0U);
  b->in.z.preGet();
  QCOMPARE(a->in.countDirty(), 0U);

  QCOMPARE(a->in.anyDirty(), false);
  QCOMPARE(a->reciprocal.anyDirty(), false);
  QCOMPARE(b->in.anyDirty(), false);

  QCOMPARE(a->evaluationCount, 4U);
  QCOMPARE(b->evaluationCount, 3U);
  b->reciprocal.preGet();
  QCOMPARE(a->evaluationCount, 4U);
  QCOMPARE(b->evaluationCount, 4U);
  }

void ShiftCoreTest::entityCompute()
  {
  TestDatabase db;

  // create an entity, everything should be dirty
  TestEntity *a = db.addChild<TestEntity>();
  TestEntity *b = db.addChild<TestEntity>();

  QCOMPARE(a->evaluationCount, 0U);
  QCOMPARE(b->evaluationCount, 0U);

  b->setInput(a);

  QCOMPARE(a->evaluationCount, 0U);
  QCOMPARE(b->evaluationCount, 0U);

  b->setInput(0);

  QCOMPARE(a->evaluationCount, 0U);
  QCOMPARE(b->evaluationCount, 0U);

  b->setInput(a);

  QCOMPARE(a->evaluationCount, 0U);
  QCOMPARE(b->evaluationCount, 0U);
  }
