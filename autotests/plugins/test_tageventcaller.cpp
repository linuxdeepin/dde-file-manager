// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_tageventcaller.cpp
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

TEST_F(TagEventCallerTest, getCollectionIconRect)
{
    // Test method: QRect getCollectionIconRect((const QString &id, QRect visualRect))
    QString _arg0{};
    auto result = obj->getCollectionIconRect(_arg0, QRect());
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagEventCallerTest, getCollectionVisualRect)
{
    // Test method: QRect getCollectionVisualRect((const QString &id, const QUrl &url))
    QString _arg0{};
    QUrl _arg1{};
    auto result = obj->getCollectionVisualRect(_arg0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(TagEventCallerTest, sendCheckTabAddable)
{
    // Test method: bool sendCheckTabAddable((quint64 windowId))
    auto result = obj->sendCheckTabAddable(0);
    EXPECT_FALSE(result);

}

TEST_F(TagEventCallerTest, sendOpenFiles)
{
    // Test method: void sendOpenFiles((const quint64 windowID, const QList<QUrl> &urls))
    QList<QUrl> _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenFiles(0, _arg1));
}

TEST_F(TagEventCallerTest, sendOpenTab)
{
    // Test method: void sendOpenTab((quint64 windowId, const QUrl &url))
    QUrl _arg1{};
    EXPECT_NO_FATAL_FAILURE(obj->sendOpenTab(0, _arg1));
}

TEST_F(TagEventCallerTest, getCollectionView)
{
    // Test method: QAbstractItemView getCollectionView((const QString &id))
    QString _arg0{};
    auto result = obj->getCollectionView(_arg0);
    EXPECT_NO_FATAL_FAILURE({ obj->getCollectionView(_arg0); });

}
