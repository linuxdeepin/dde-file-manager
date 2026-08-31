// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_canvasviewhook_1.cpp
 * @brief Unit tests for CanvasViewHook methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "hook/canvasviewhook.h"

#include <QTest>

using namespace ddplugin_canvas;

class CanvasViewHookTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new CanvasViewHook();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    CanvasViewHook *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(CanvasViewHookTest, contextMenu)
{
    // Test method: bool contextMenu((int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &pos, void *extData))
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    QPoint _arg3{};
    auto result = obj->contextMenu(0, _arg1, _arg2, _arg3, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, dragEnter)
{
    // Test method: bool dragEnter((int viewIndex, const QMimeData *mime, void *extData))
    auto result = obj->dragEnter(0, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, dragLeave)
{
    // Test method: bool dragLeave((int viewIndex, const QMimeData *mime, void *extData))
    auto result = obj->dragLeave(0, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, dragMove)
{
    // Test method: bool dragMove((int viewIndex, const QMimeData *mime, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->dragMove(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, drawFile)
{
    // Test method: bool drawFile((int viewIndex, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData))
    QUrl _arg1{};
    auto result = obj->drawFile(0, _arg1, nullptr, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, dropData)
{
    // Test method: bool dropData((int viewIndex, const QMimeData *md, const QPoint &viewPoint, void *extData))
    QPoint _arg2{};
    auto result = obj->dropData(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, keyPress)
{
    // Test method: bool keyPress((int viewIndex, int key, int modifiers, void *extData))
    auto result = obj->keyPress(0, 0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, keyboardSearch)
{
    // Test method: bool keyboardSearch((int viewIndex, const QString &search, void *extData))
    QString _arg1{};
    auto result = obj->keyboardSearch(0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, mouseDoubleClick)
{
    // Test method: bool mouseDoubleClick((int viewIndex, int button, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->mouseDoubleClick(0, 0, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, mousePress)
{
    // Test method: bool mousePress((int viewIndex, int button, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->mousePress(0, 0, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, mouseRelease)
{
    // Test method: bool mouseRelease((int viewIndex, int button, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->mouseRelease(0, 0, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, shortcutkeyPress)
{
    // Test method: bool shortcutkeyPress((int viewIndex, int key, int modifiers, void *extData))
    auto result = obj->shortcutkeyPress(0, 0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, startDrag)
{
    // Test method: bool startDrag((int viewIndex, int supportedActions, void *extData))
    auto result = obj->startDrag(0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(CanvasViewHookTest, wheel)
{
    // Test method: bool wheel((int viewIndex, const QPoint &angleDelta, void *extData))
    QPoint _arg1{};
    auto result = obj->wheel(0, _arg1, nullptr);
    EXPECT_FALSE(result);

}
