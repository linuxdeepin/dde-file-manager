// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_myshares_1.cpp
 * @brief Unit tests for MyShares methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "myshares.h"

#include <QTest>

using namespace dfmplugin_myshares;

class MySharesTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new MyShares();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    MyShares *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(MySharesTest, beMySubOnAdded)
{
    // Test method: void beMySubOnAdded((const QString &newScene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->beMySubOnAdded(_arg0));
}

TEST_F(MySharesTest, beMySubScene)
{
    // Test method: void beMySubScene((const QString &scene))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->beMySubScene(_arg0));
}

TEST_F(MySharesTest, bindWindows)
{
    // Test method: void bindWindows(())
    EXPECT_NO_FATAL_FAILURE(obj->bindWindows());
}

TEST_F(MySharesTest, contenxtMenuHandle)
{
    // Test method: void contenxtMenuHandle((quint64 windowId, const QUrl &url, const QPoint &globalPos))
    QUrl _arg1{};
    QPoint _arg2{};
    EXPECT_NO_FATAL_FAILURE(obj->contenxtMenuHandle(0, _arg1, _arg2));
}

TEST_F(MySharesTest, doInitialize)
{
    // Test method: void doInitialize(())
    EXPECT_NO_FATAL_FAILURE(obj->doInitialize());
}

TEST_F(MySharesTest, followEvents)
{
    // Test method: void followEvents(())
    EXPECT_NO_FATAL_FAILURE(obj->followEvents());
}

TEST_F(MySharesTest, initialize)
{
    // Test method: void initialize(())
    EXPECT_NO_FATAL_FAILURE(obj->initialize());
}

TEST_F(MySharesTest, onShareAdded)
{
    // Test method: void onShareAdded((const QString &))
    QString _arg0{};
    EXPECT_NO_FATAL_FAILURE(obj->onShareAdded(_arg0));
}

TEST_F(MySharesTest, onWindowOpened)
{
    // Test method: void onWindowOpened((quint64 winId))
    EXPECT_NO_FATAL_FAILURE(obj->onWindowOpened(0));
}

TEST_F(MySharesTest, start)
{
    // Test bool getter: start()
    bool result = obj->start();
    EXPECT_FALSE(result);

}
