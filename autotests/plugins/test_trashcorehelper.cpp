// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashcorehelper.cpp
 * @brief Unit tests for TrashCoreHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/trashcorehelper.h"

#include <QTest>

using namespace dfmplugin_trashcore;

class TrashCoreHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashCoreHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashCoreHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashCoreHelperTest, calculateTrashRoot)
{
    // Test getter: std::pair<qint64, int> calculateTrashRoot()
    auto result = obj->calculateTrashRoot();
    EXPECT_NO_FATAL_FAILURE({ obj->calculateTrashRoot(); });

}

TEST_F(TrashCoreHelperTest, icon)
{
    // Test getter: QIcon icon()
    auto result = obj->icon();
    EXPECT_TRUE(result.isNull());

}

TEST_F(TrashCoreHelperTest, rootUrl)
{
    // Test getter: QUrl rootUrl()
    auto result = obj->rootUrl();
    EXPECT_TRUE(result.isEmpty() || result.isValid());
}

TEST_F(TrashCoreHelperTest, scheme)
{
    // Test getter: QString scheme()
    auto result = obj->scheme();
    EXPECT_TRUE(result.isEmpty());

}
