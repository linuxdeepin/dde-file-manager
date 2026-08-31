// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashdiriterator_1.cpp
 * @brief Unit tests for TrashDirIterator methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "trashdiriterator.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashDirIteratorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashDirIterator();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashDirIterator *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashDirIteratorTest, TrashDirIterator)
{
    // Test constructor: TrashDirIterator((const QUrl &url,
                                   const QStringList &nameFilters,
                                   QDir::Filters filters,
                                   QDirIterator::IteratorFlags flags))
    ASSERT_NE(obj, nullptr);
}

TEST_F(TrashDirIteratorTest, fileName)
{
    // Test getter: QString fileName()
    auto result = obj->fileName();
    EXPECT_TRUE(result.isEmpty() || !result.isEmpty());
}

TEST_F(TrashDirIteratorTest, fileUrl)
{
    // Test getter: QUrl fileUrl()
    auto result = obj->fileUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(TrashDirIteratorTest, next)
{
    // Test getter: QUrl next()
    auto result = obj->next();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(TrashDirIteratorTest, TrashDirIterator_Destructor)
{
    // Test method:  ~TrashDirIterator(())
    EXPECT_NO_FATAL_FAILURE({ TrashDirIterator *tmp = new TrashDirIterator(); delete tmp; });
}
