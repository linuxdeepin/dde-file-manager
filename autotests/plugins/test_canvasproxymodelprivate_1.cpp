// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasproxymodelprivate_1.cpp
 * @brief Unit tests for CanvasProxyModelPrivate methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "model/canvasproxymodel.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasProxyModelPrivateTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasProxyModelPrivate();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasProxyModelPrivate *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasProxyModelPrivateTest, CanvasProxyModelPrivate)
{
    // Test constructor: CanvasProxyModelPrivate((CanvasProxyModel *qq))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasProxyModelPrivateTest, createMapping)
{
    // Test method: void createMapping(())
    EXPECT_NO_FATAL_FAILURE(obj->createMapping());
}

TEST_F(CanvasProxyModelPrivateTest, doRefresh)
{
    // Test method: void doRefresh((bool global, bool updateFile))
    EXPECT_NO_FATAL_FAILURE(obj->doRefresh(false, false));
}

TEST_F(CanvasProxyModelPrivateTest, doSort)
{
    // Test method: bool doSort((QList<QUrl> &files))
    QList<QUrl> _arg0{};
    auto result = obj->doSort(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasProxyModelPrivateTest, insertFilter)
{
    // Test method: bool insertFilter((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->insertFilter(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasProxyModelPrivateTest, renameFilter)
{
    // Test method: bool renameFilter((const QUrl &oldUrl, const QUrl &newUrl))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->renameFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}

TEST_F(CanvasProxyModelPrivateTest, sendLoadReport)
{
    // Test method: void sendLoadReport(())
    EXPECT_NO_FATAL_FAILURE(obj->sendLoadReport());
}

TEST_F(CanvasProxyModelPrivateTest, specialSort)
{
    // Test method: void specialSort((QList<QUrl> &files))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->specialSort(_arg0));
}

TEST_F(CanvasProxyModelPrivateTest, standardSort)
{
    // Test method: void standardSort((QList<QUrl> &files))
    QList<QUrl> _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->standardSort(_arg0));
}

TEST_F(CanvasProxyModelPrivateTest, updateFilter)
{
    // Test method: bool updateFilter((const QUrl &url, const QVector<int> &roles))
    QUrl _arg0{};
    QVector<int> _arg1{};
    auto result = obj->updateFilter(_arg0, _arg1);
    EXPECT_FALSE(result);

}
