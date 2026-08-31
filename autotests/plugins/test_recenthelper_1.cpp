// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_recenthelper_1.cpp
 * @brief Unit tests for RecentHelper methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "utils/recentmanager.h"

#include <QTest>

using namespace dfmplugin_recent;

class RecentHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new RecentHelper();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    RecentHelper *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(RecentHelperTest, contenxtMenuHandle)
{
    // Test method: void contenxtMenuHandle((quint64 windowId, const QUrl &url, const QPoint &globalPos))
    QUrl _arg1{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->contenxtMenuHandle(0, _arg1, _arg2));
}

TEST_F(RecentHelperTest, openFileLocation)
{
    // Test method: void openFileLocation((const QList<QUrl> &urls))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->openFileLocation(_arg0));
}

TEST_F(RecentHelperTest, propetyExtensionFunc)
{
    // Test method: ExpandFieldMap propetyExtensionFunc((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->propetyExtensionFunc(_arg0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(RecentHelperTest, recentUrl)
{
    // Test method: QUrl recentUrl((const QString &path))
    QString _arg0{};
    auto result = obj->recentUrl(_arg0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(RecentHelperTest, urlTransform)
{
    // Test method: QUrl urlTransform((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->urlTransform(_arg0);
    EXPECT_FALSE(result.isValid());

}
