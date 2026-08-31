// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file test_viewhookinterface_1.cpp
 * @brief Unit tests for ViewHookInterface methods with real assertions
 */

#include <gtest/gtest.h>

#include "stubext.h"

#include "view/viewhookinterface.h"

#include <QTest>

using namespace ddplugin_canvas;

class ViewHookInterfaceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        obj = new ViewHookInterface();
    }

    void TearDown() override
    {
        delete obj;
        obj = nullptr;
        stub.clear();
    }

    ViewHookInterface *obj = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(ViewHookInterfaceTest, contextMenu)
{
    // Test method: bool contextMenu((int viewIndex, const QUrl &dir, const QList<QUrl> &files, const QPoint &viewPos, void *extData))
    QUrl _arg1{};
    QList<QUrl> _arg2{};
    QPoint _arg3{};
    auto result = obj->contextMenu(0, _arg1, _arg2, _arg3, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, dragEnter)
{
    // Test method: bool dragEnter((int viewIndex, const QMimeData *, void *extData))
    auto result = obj->dragEnter(0, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, dragLeave)
{
    // Test method: bool dragLeave((int viewIndex, const QMimeData *, void *extData))
    auto result = obj->dragLeave(0, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, dragMove)
{
    // Test method: bool dragMove((int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->dragMove(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, drawFile)
{
    // Test method: bool drawFile((int viewIndex, const QUrl &file, QPainter *painter, const QStyleOptionViewItem *option, void *extData))
    QUrl _arg1{};
    auto result = obj->drawFile(0, _arg1, nullptr, nullptr, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, dropData)
{
    // Test method: bool dropData((int viewIndex, const QMimeData *, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->dropData(0, nullptr, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, keyPress)
{
    // Test method: bool keyPress((int viewIndex, int key, int modifiers, void *extData))
    auto result = obj->keyPress(0, 0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, keyboardSearch)
{
    // Test method: bool keyboardSearch((int viewIndex, const QString &search, void *extData))
    QString _arg1{};
    auto result = obj->keyboardSearch(0, _arg1, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, mouseDoubleClick)
{
    // Test method: bool mouseDoubleClick((int viewIndex, int button, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->mouseDoubleClick(0, 0, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, mousePress)
{
    // Test method: bool mousePress((int viewIndex, int button, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->mousePress(0, 0, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, mouseRelease)
{
    // Test method: bool mouseRelease((int viewIndex, int button, const QPoint &viewPos, void *extData))
    QPoint _arg2{};
    auto result = obj->mouseRelease(0, 0, _arg2, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, shortcutAction)
{
    // Test method: bool shortcutAction((int viewIndex, int keySequence, void *extData))
    auto result = obj->shortcutAction(0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, shortcutkeyPress)
{
    // Test method: bool shortcutkeyPress((int viewIndex, int key, int modifiers, void *extData))
    auto result = obj->shortcutkeyPress(0, 0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, startDrag)
{
    // Test method: bool startDrag((int viewIndex, int supportedActions, void *extData))
    auto result = obj->startDrag(0, 0, nullptr);
    EXPECT_FALSE(result);

}

TEST_F(ViewHookInterfaceTest, wheel)
{
    // Test method: bool wheel((int viewIndex, const QPoint &angleDelta, void *extData))
    QPoint _arg1{};
    auto result = obj->wheel(0, _arg1, nullptr);
    EXPECT_FALSE(result);

}
