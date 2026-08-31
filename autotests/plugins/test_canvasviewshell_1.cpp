// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasviewshell_1.cpp
 * @brief Unit tests for CanvasViewShell methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "interface/canvasviewshell.h"

#include <QTest>

using namespace ddplugin_organizer;

class CanvasViewShellTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasViewShell();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasViewShell *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewShellTest, CanvasViewShell)
{
    // Test constructor: CanvasViewShell((QObject *parent))
    ASSERT_NE(obj, nullptr);
}

TEST_F(CanvasViewShellTest, canvasView)
{
    // Test method: QAbstractItemView canvasView((int viewIndex))
    auto result = obj->canvasView(0);
    EXPECT_NO_FATAL_FAILURE({ obj->canvasView(0); });

}

TEST_F(CanvasViewShellTest, eventContextMenu)
{
    // Test method: bool eventContextMenu((int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos, void *extData))
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    QPoint _arg3{};
    auto result = obj->eventContextMenu(0, _arg1, _arg2, _arg3, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewShellTest, eventDropData)
{
    // Test method: bool eventDropData((int viewIndex, const QMimeData *mimeData, const QPoint &viewPoint, void *extData))
    QPoint _arg2{};
    auto result = obj->eventDropData(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewShellTest, eventKeyPress)
{
    // Test method: bool eventKeyPress((int viewIndex, int key, int modifiers, void *extData))
    auto result = obj->eventKeyPress(0, 0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewShellTest, eventShortcutkeyPress)
{
    // Test method: bool eventShortcutkeyPress((int viewIndex, int key, int modifiers, void *extData))
    auto result = obj->eventShortcutkeyPress(0, 0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewShellTest, eventWheel)
{
    // Test method: bool eventWheel((int viewIndex, const QPoint &angleDelta, void *extData))
    QPoint _arg1{};
    auto result = obj->eventWheel(0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewShellTest, gridSize)
{
    // Test method: QSize gridSize((int viewIndex))
    auto result = obj->gridSize(0);
    EXPECT_TRUE(result.isEmpty());

}

TEST_F(CanvasViewShellTest, gridVisualRect)
{
    // Test method: QRect gridVisualRect((int viewIndex, const QPoint &gridPos))
    QPoint _arg1{};
    auto result = obj->gridVisualRect(0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewShellTest, initialize)
{
    // Test bool getter: initialize()
    bool result = obj->initialize();
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewShellTest, visualRect)
{
    // Test method: QRect visualRect((int viewIndex, const QUrl &url))
    QUrl _arg1{};
    auto result = obj->visualRect(0, _arg1);
    EXPECT_FALSE(result.isValid());

}

TEST_F(CanvasViewShellTest, CanvasViewShell_Destructor)
{
    // Test method:  ~CanvasViewShell(())
    EXPECT_NO_FATAL_FAILURE({ CanvasViewShell *tmp = new CanvasViewShell(); delete tmp; });
}
