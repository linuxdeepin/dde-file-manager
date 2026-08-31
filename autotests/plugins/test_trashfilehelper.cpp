// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_trashfilehelper.cpp
 * @brief Unit tests for TrashFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/trashfilehelper.h"

#include <QTest>

using namespace dfmplugin_trash;

class TrashFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TrashFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TrashFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TrashFileHelperTest, deleteFile)
{
    // Test method: bool deleteFile((const quint64 windowId, const QList<QUrl> sources, const DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags flags))
    auto result = obj->deleteFile(0, QList<QUrl>(), DFMBASE_NAMESPACE::AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(TrashFileHelperTest, instance)
{
    // Test getter: DPTRASH_USE_NAMESPACE instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(TrashFileHelperTest, openFileInPlugin)
{
    // Test method: bool openFileInPlugin((quint64 windowId, const QList<QUrl> urls))
    auto result = obj->openFileInPlugin(0, QList<QUrl>());
    EXPECT_FALSE(result);

}
