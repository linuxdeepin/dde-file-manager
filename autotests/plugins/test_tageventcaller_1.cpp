// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tageventcaller_1.cpp
 * @brief Unit tests for TagEventCaller methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "events/tageventcaller.h"

#include <QTest>

using namespace dfmplugin_tag;

class TagEventCallerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new TagEventCaller();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    TagEventCaller *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(TagEventCallerTest, TagEventCaller)
{
    // Test constructor: TagEventCaller(())
    ASSERT_NE(obj, nullptr);
}

TEST_F(TagEventCallerTest, getCollectionViewId)
{
    // Test method: QString getCollectionViewId((const QUrl &url, QPoint *pos))
    QUrl _arg0{};
    auto result = obj->getCollectionViewId(_arg0, nullptr);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(TagEventCallerTest, getDesktopView)
{
    // Test method: QAbstractItemView getDesktopView((int viewIdx))
    auto result = obj->getDesktopView(0);
    EXPECT_NO_FATAL_FAILURE({ obj->getDesktopView(0); });

}

TEST_F(TagEventCallerTest, getDesktopViewIndex)
{
    // Test method: int getDesktopViewIndex((const QString &url, QPoint *pos))
    QString _arg0{};
    auto result = obj->getDesktopViewIndex(_arg0, nullptr);
    EXPECT_GE(result, 0);

}

TEST_F(TagEventCallerTest, getIconRect)
{
    // Test method: QRect getIconRect((int viewIndex, QRect visualRect))
    auto result = obj->getIconRect(0, QRect());
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagEventCallerTest, getItemRect)
{
    // Test method: QRectF getItemRect((const quint64 windowID, const QUrl &url, const ItemRoles role))
    QUrl _arg1{};
    auto result = obj->getItemRect(0, _arg1, ItemRoles());
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagEventCallerTest, getVisibleGeometry)
{
    // Test method: QRectF getVisibleGeometry((const quint64 windowID))
    auto result = obj->getVisibleGeometry(0);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagEventCallerTest, getVisualRect)
{
    // Test method: QRect getVisualRect((int viewIndex, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->getVisualRect(0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagEventCallerTest, sendFileUpdate)
{
    // Test method: void sendFileUpdate((const QString &path))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->sendFileUpdate(_arg0));
}
