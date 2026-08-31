// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasmodelshell.cpp
 * @brief Unit tests for CanvasModelShell methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "interface/canvasmodelshell.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasModelShellTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasModelShell();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasModelShell *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasModelShellTest, CanvasModelShell)
{
    // Test constructor: CanvasModelShell((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasModelShellTest, eventDataInserted)
{
    // Test method: bool eventDataInserted((const QUrl &url, void *extData))
    QUrl _arg0{};
    auto result = obj->eventDataInserted(_arg0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelShellTest, eventDataRenamed)
{
    // Test method: bool eventDataRenamed((const QUrl &oldUrl, const QUrl &newUrl, void *extData))
    QUrl _arg0{};
    QUrl _arg1{};
    auto result = obj->eventDataRenamed(_arg0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelShellTest, eventDataRested)
{
    // Test method: bool eventDataRested((QList<QUrl> *urls, void *extData))
    auto result = obj->eventDataRested(nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelShellTest, fetch)
{
    // Test method: bool fetch((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->fetch(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelShellTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelShellTest, refresh)
{
    // Test method: void refresh((int ms, bool updateFile))
    EXPECT_NO_FATAL_FAILURE(obj->refresh(0, false));
}

TEST_F(CanvasModelShellTest, take)
{
    // Test method: bool take((const QUrl &url))
    QUrl _arg0{};
    auto result = obj->take(_arg0);
    EXPECT_FALSE(result);

}

TEST_F(CanvasModelShellTest, CanvasModelShell_Destructor)
{
    // Test method:  ~CanvasModelShell(())
    EXPECT_NO_FATAL_FAILURE({ CanvasModelShell *tmp = new CanvasModelShell(); delete tmp; });
}
