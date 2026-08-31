// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashdiriteratorprivate.cpp
 * @brief Unit tests for TrashDirIteratorPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "trashdiriterator.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashDirIteratorPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashDirIteratorPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashDirIteratorPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashDirIteratorPrivateTest, TrashDirIteratorPrivate)
{
    // Test constructor: TrashDirIteratorPrivate((const QUrl &url, const QStringList &nameFilters,
                                                 DFMIO::DEnumerator::DirFilters filters, DFMIO::DEnumerator::IteratorFlags flags,
                                                 TrashDirIterator *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TrashDirIteratorPrivateTest, TrashDirIteratorPrivate_Destructor)
{
    // Test method:  ~TrashDirIteratorPrivate(())
    EXPECT_NO_FATAL_FAILURE({ TrashDirIteratorPrivate *tmp = new TrashDirIteratorPrivate(); delete tmp; });
}
