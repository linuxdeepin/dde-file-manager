// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_threadcontainer.cpp
 * @brief Unit tests for DThreadList / DThreadMap / DThreadHash (threadcontainer.h)
 *
 * The three templates in utils/threadcontainer.h wrap QList/QMap/QHash with a
 * QMutex so every mutating / reading operation is serialized. They are pure
 * value types with no DBus, filesystem or GUI dependency, so each public
 * method can be exercised directly and its observable behaviour asserted.
 *
 * @note Source defects (not fixed, only flagged — Iron Law #9):
 *   - DThreadList::size() const / at(int) const lock a *non-mutable* QMutex
 *     from a const method, which does not compile when instantiated. They are
 *     therefore intentionally NOT called here (a non-compiling instantiation
 *     would break the whole ut-dfm-base binary). DThreadMap/DThreadHash use a
 *     `mutable` mutex and are unaffected.
 *   - DThreadHash::begin()/end()/erase() declare a QMap<...>::iterator return
 *     / parameter type but operate on the underlying QHash (`myHash`); the
 *     iterator types are unrelated, so these methods do not compile when
 *     instantiated. They are intentionally NOT called here.
 *   - DThreadList declares an implicit (shallow) copy constructor while
 *     defining a deep-copy operator=. Copy-*constructing* would share the
 *     QList pointer and double-free on destruction; the tests therefore use
 *     assignment (deep copy) only.
 *   - DThreadMap::erase(QMap::iterator) compiles, but the only public way to
 *     obtain an iterator is map() which returns a *copy* of the underlying
 *     QMap; calling erase() with an iterator that belongs to a different map
 *     is undefined behaviour (no public accessor yields a live in-map
 *     iterator). It is therefore intentionally NOT exercised here.
 * All other public methods are covered below.
 */

#include <gtest/gtest.h>
#include <dfm-base/utils/threadcontainer.h>

#include <QHash>
#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

using namespace dfmbase;

// ---------------------------------------------------------------------------
// DThreadList<T>
// ---------------------------------------------------------------------------
TEST(DThreadListTest, DefaultConstructIsEmpty)
{
    DThreadList<int> l;
    EXPECT_EQ(l.count(), 0);
    EXPECT_TRUE(l.listByLock().isEmpty());
}

TEST(DThreadListTest, DestructorRunsCleanForOwnedHeapList)
{
    {
        DThreadList<int> l;
        l.push_backByLock(1);
        l.push_backByLock(2);
    }
    SUCCEED();   // dtor locks, clears and deletes the internal QList
}

TEST(DThreadListTest, PushBackByLockAppendsToEnd)
{
    DThreadList<int> l;
    l.push_backByLock(1);
    l.push_backByLock(2);
    l.push_backByLock(3);
    EXPECT_EQ(l.listByLock(), (QList<int> { 1, 2, 3 }));
    EXPECT_EQ(l.count(), 3);
}

TEST(DThreadListTest, AppendByLockAppendsToEnd)
{
    DThreadList<int> l;
    l.appendByLock(10);
    l.appendByLock(20);
    EXPECT_EQ(l.listByLock(), (QList<int> { 10, 20 }));
}

TEST(DThreadListTest, InsertByLockPrependsToFront)
{
    DThreadList<int> l;
    l.push_backByLock(1);
    l.push_backByLock(2);   // [1, 2]
    l.insertByLock(3);      // inserts at index 0 -> [3, 1, 2]
    EXPECT_EQ(l.listByLock(), (QList<int> { 3, 1, 2 }));
}

TEST(DThreadListTest, SetListReplacesEntireContents)
{
    DThreadList<int> l;
    l.push_backByLock(1);
    l.setList(QList<int> { 7, 8, 9 });
    EXPECT_EQ(l.listByLock(), (QList<int> { 7, 8, 9 }));
    EXPECT_EQ(l.count(), 3);
}

TEST(DThreadListTest, ListByLockReturnsConstReferenceToCurrentContents)
{
    DThreadList<QString> l;
    l.push_backByLock(QStringLiteral("a"));
    const QList<QString> &ref = l.listByLock();
    EXPECT_EQ(ref, (QList<QString> { QStringLiteral("a") }));
}

TEST(DThreadListTest, RemoveAllByLockRemovesEveryOccurrence)
{
    DThreadList<int> l;
    l.setList(QList<int> { 1, 2, 1, 3, 1 });
    l.removeAllByLock(1);
    EXPECT_EQ(l.listByLock(), (QList<int> { 2, 3 }));
}

TEST(DThreadListTest, RemoveOneByLockRemovesOnlyFirstOccurrence)
{
    DThreadList<int> l;
    l.setList(QList<int> { 1, 2, 1 });
    l.removeOneByLock(1);
    EXPECT_EQ(l.listByLock(), (QList<int> { 2, 1 }));
}

TEST(DThreadListTest, ContainsByLockReportsMembership)
{
    DThreadList<int> l;
    l.setList(QList<int> { 5, 10, 15 });
    EXPECT_TRUE(l.containsByLock(10));
    EXPECT_FALSE(l.containsByLock(99));
}

TEST(DThreadListTest, ClearByLockEmptiesTheList)
{
    DThreadList<int> l;
    l.setList(QList<int> { 1, 2, 3 });
    l.clearByLock();
    EXPECT_EQ(l.count(), 0);
    EXPECT_TRUE(l.listByLock().isEmpty());
}

TEST(DThreadListTest, CountReturnsNumberOfElements)
{
    DThreadList<int> l;
    EXPECT_EQ(l.count(), 0);
    l.push_backByLock(1);
    l.push_backByLock(2);
    EXPECT_EQ(l.count(), 2);
}

TEST(DThreadListTest, IndexOfReturnsPositionOrMinusOne)
{
    DThreadList<int> l;
    l.setList(QList<int> { 10, 20, 30, 20 });
    EXPECT_EQ(l.indexOf(20), 1);
    EXPECT_EQ(l.indexOf(20, 2), 3);   // search forward from offset 2
    EXPECT_EQ(l.indexOf(99), -1);
}

TEST(DThreadListTest, AssignmentOperatorDeepCopiesContents)
{
    DThreadList<int> l;
    l.setList(QList<int> { 1, 2, 3 });
    DThreadList<int> l2;   // default-constructed, owns its own QList
    l2 = l;                // deep copy via operator=
    EXPECT_EQ(l2.listByLock(), l.listByLock());
    // mutate the copy, original must be unaffected (independent storage)
    l2.push_backByLock(4);
    EXPECT_EQ(l.count(), 3);
    EXPECT_EQ(l2.count(), 4);
}

// ---------------------------------------------------------------------------
// DThreadMap<DKey, DValue>
// ---------------------------------------------------------------------------
TEST(DThreadMapTest, DefaultConstructIsEmpty)
{
    DThreadMap<QString, int> m;
    EXPECT_EQ(m.count(), 0);
    EXPECT_TRUE(m.keys().isEmpty());
}

TEST(DThreadMapTest, InsertStoresKeyValuePair)
{
    DThreadMap<QString, int> m;
    m.insert(QStringLiteral("a"), 1);
    m.insert(QStringLiteral("b"), 2);
    EXPECT_EQ(m.count(), 2);
    EXPECT_EQ(m.value(QStringLiteral("a")), 1);
    EXPECT_EQ(m.value(QStringLiteral("b")), 2);
}

TEST(DThreadMapTest, ValueReturnsDefaultForMissingKey)
{
    DThreadMap<QString, int> m;
    m.insert(QStringLiteral("a"), 1);
    EXPECT_EQ(m.value(QStringLiteral("missing")), 0);   // int default
}

TEST(DThreadMapTest, ContainsReportsKeyPresence)
{
    DThreadMap<QString, int> m;
    m.insert(QStringLiteral("a"), 1);
    EXPECT_TRUE(m.contains(QStringLiteral("a")));
    EXPECT_FALSE(m.contains(QStringLiteral("z")));
}

TEST(DThreadMapTest, KeysReturnsAllKeysInMapOrder)
{
    DThreadMap<QString, int> m;
    m.insert(QStringLiteral("b"), 2);
    m.insert(QStringLiteral("a"), 1);
    m.insert(QStringLiteral("c"), 3);
    EXPECT_EQ(m.keys(), (QList<QString> { QStringLiteral("a"), QStringLiteral("b"), QStringLiteral("c") }));
}

TEST(DThreadMapTest, MapReturnsACopyOfTheUnderlyingQMap)
{
    DThreadMap<QString, int> m;
    m.insert(QStringLiteral("a"), 1);
    m.insert(QStringLiteral("b"), 2);
    QMap<QString, int> copy = m.map();
    EXPECT_EQ(copy.size(), 2);
    EXPECT_EQ(copy.value(QStringLiteral("a")), 1);
}

TEST(DThreadMapTest, RemoveDeletesASingleKey)
{
    DThreadMap<QString, int> m;
    m.insert(QStringLiteral("a"), 1);
    m.insert(QStringLiteral("b"), 2);
    m.remove(QStringLiteral("a"));
    EXPECT_EQ(m.count(), 1);
    EXPECT_FALSE(m.contains(QStringLiteral("a")));
    EXPECT_TRUE(m.contains(QStringLiteral("b")));
}

TEST(DThreadMapTest, ClearRemovesAllEntries)
{
    DThreadMap<QString, int> m;
    m.insert(QStringLiteral("a"), 1);
    m.insert(QStringLiteral("b"), 2);
    m.clear();
    EXPECT_EQ(m.count(), 0);
    EXPECT_TRUE(m.keys().isEmpty());
}

// ---------------------------------------------------------------------------
// DThreadHash<DKey, DValue>
// (begin()/end()/erase() are source_defect_compile — see file header, not called)
// ---------------------------------------------------------------------------
TEST(DThreadHashTest, DefaultConstructIsEmpty)
{
    DThreadHash<QString, int> h;
    EXPECT_EQ(h.count(), 0);
    EXPECT_TRUE(h.keys().isEmpty());
}

TEST(DThreadHashTest, InsertStoresKeyValuePair)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    h.insert(QStringLiteral("b"), 2);
    EXPECT_EQ(h.count(), 2);
    EXPECT_EQ(h.value(QStringLiteral("a")), 1);
    EXPECT_EQ(h.value(QStringLiteral("b")), 2);
}

TEST(DThreadHashTest, ValueReturnsDefaultForMissingKey)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    EXPECT_EQ(h.value(QStringLiteral("missing")), 0);
}

TEST(DThreadHashTest, ContainsReportsKeyPresence)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    EXPECT_TRUE(h.contains(QStringLiteral("a")));
    EXPECT_FALSE(h.contains(QStringLiteral("z")));
}

TEST(DThreadHashTest, KeysReturnsAllStoredKeys)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    h.insert(QStringLiteral("b"), 2);
    QList<QString> keys = h.keys();
    EXPECT_EQ(keys.size(), 2);
    EXPECT_TRUE(keys.contains(QStringLiteral("a")));
    EXPECT_TRUE(keys.contains(QStringLiteral("b")));
}

TEST(DThreadHashTest, HashReturnsACopyOfTheUnderlyingQHash)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    h.insert(QStringLiteral("b"), 2);
    QHash<QString, int> copy = h.hash();
    EXPECT_EQ(copy.size(), 2);
    EXPECT_EQ(copy.value(QStringLiteral("a")), 1);
}

TEST(DThreadHashTest, RemoveDeletesASingleKey)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    h.insert(QStringLiteral("b"), 2);
    h.remove(QStringLiteral("a"));
    EXPECT_EQ(h.count(), 1);
    EXPECT_FALSE(h.contains(QStringLiteral("a")));
    EXPECT_TRUE(h.contains(QStringLiteral("b")));
}

TEST(DThreadHashTest, RemoveIfReturnsMatchingKeysAndRemovesThem)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    h.insert(QStringLiteral("b"), 2);
    h.insert(QStringLiteral("c"), 3);
    QList<QString> removed = h.removeIf([](const QString &, const int &v) { return v == 2; });
    ASSERT_EQ(removed.size(), 1);
    EXPECT_TRUE(removed.contains(QStringLiteral("b")));
    EXPECT_EQ(h.count(), 2);
    EXPECT_FALSE(h.contains(QStringLiteral("b")));
}

TEST(DThreadHashTest, RemoveIfWithNoMatchRemovesNothing)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    QList<QString> removed = h.removeIf([](const QString &, const int &) { return false; });
    EXPECT_TRUE(removed.isEmpty());
    EXPECT_EQ(h.count(), 1);
}

TEST(DThreadHashTest, RemoveIfMatchingAllClearsEverything)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    h.insert(QStringLiteral("b"), 2);
    QList<QString> removed = h.removeIf([](const QString &, const int &) { return true; });
    EXPECT_EQ(removed.size(), 2);
    EXPECT_EQ(h.count(), 0);
}

TEST(DThreadHashTest, ClearRemovesAllEntries)
{
    DThreadHash<QString, int> h;
    h.insert(QStringLiteral("a"), 1);
    h.insert(QStringLiteral("b"), 2);
    h.clear();
    EXPECT_EQ(h.count(), 0);
    EXPECT_TRUE(h.keys().isEmpty());
}
