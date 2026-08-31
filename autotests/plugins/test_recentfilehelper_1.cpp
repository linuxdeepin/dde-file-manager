// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recentfilehelper_1.cpp
 * @brief Unit tests for RecentFileHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/recentfilehelper.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentFileHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentFileHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentFileHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentFileHelperTest, RecentFileHelper)
{
    // Test constructor: RecentFileHelper((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(RecentFileHelperTest, copyFile)
{
    // Test method: bool copyFile((const quint64, const QList<QUrl>, const QUrl target, const AbstractJobHandler::JobFlags))
    auto result = obj->copyFile({}, QList<QUrl>(), QUrl(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(RecentFileHelperTest, cutFile)
{
    // Test method: bool cutFile((const quint64 windowId, const QList<QUrl> sources, const QUrl target, const AbstractJobHandler::JobFlags flags))
    auto result = obj->cutFile(0, QList<QUrl>(), QUrl(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(RecentFileHelperTest, instance)
{
    // Test getter: RecentFileHelper instance()
    auto result = obj->instance();
    EXPECT_NO_FATAL_FAILURE({ obj->instance(); });

}

TEST_F(RecentFileHelperTest, moveToTrash)
{
    // Test method: bool moveToTrash((const quint64 windowId, const QList<QUrl> sources, const AbstractJobHandler::JobFlags flags))
    auto result = obj->moveToTrash(0, QList<QUrl>(), AbstractJobHandler::JobFlags());
    EXPECT_FALSE(result);

}

TEST_F(RecentFileHelperTest, openFileInPlugin)
{
    // Test method: bool openFileInPlugin((quint64 winId, QList<QUrl> urls))
    auto result = obj->openFileInPlugin(0, QList<QUrl>());
    EXPECT_FALSE(result);

}

TEST_F(RecentFileHelperTest, openFileInTerminal)
{
    // Test method: bool openFileInTerminal((const quint64 windowId, const QList<QUrl> urls))
    auto result = obj->openFileInTerminal(0, QList<QUrl>());
    EXPECT_FALSE(result);

}

TEST_F(RecentFileHelperTest, setPermissionHandle)
{
    // Test method: bool setPermissionHandle((const quint64 windowId, const QUrl url, const QFileDevice::Permissions permissions, bool *ok, QString *error))
    auto result = obj->setPermissionHandle(0, QUrl(), QFileDevice::Permissions(), nullptr, nullptr);
    EXPECT_FALSE(result);

}
