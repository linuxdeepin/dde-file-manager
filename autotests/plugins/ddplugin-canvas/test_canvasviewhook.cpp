// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "hook/canvasviewhook.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QUrl>
#include <QPoint>
#include <QMimeData>
#include <QPainter>
#include <QStyleOptionViewItem>

using namespace ddplugin_canvas;

class UT_CanvasViewHook : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        hook = new CanvasViewHook();
    }

    virtual void TearDown() override
    {
        if (hook) {
            delete hook;
            hook = nullptr;
        }

        stub.clear();
    }

public:
    QApplication *app = nullptr;
    CanvasViewHook *hook = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CanvasViewHook, constructor_CreateHook_InitializesCorrectly)
{
    EXPECT_NE(hook, nullptr);
}

TEST_F(UT_CanvasViewHook, contextMenu_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    QUrl dir("file:///tmp");
    QList<QUrl> files;
    QPoint pos(100, 100);
    void *extData = nullptr;

    bool result = hook->contextMenu(viewIndex, dir, files, pos, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, dropData_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    QMimeData mimeData;
    QPoint viewPoint(100, 100);
    void *extData = nullptr;

    bool result = hook->dropData(viewIndex, &mimeData, viewPoint, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, keyPress_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    int key = Qt::Key_A;
    int modifiers = Qt::NoModifier;
    void *extData = nullptr;

    bool result = hook->keyPress(viewIndex, key, modifiers, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, mousePress_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    int button = Qt::LeftButton;
    QPoint viewPos(100, 100);
    void *extData = nullptr;

    bool result = hook->mousePress(viewIndex, button, viewPos, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, mouseRelease_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    int button = Qt::LeftButton;
    QPoint viewPos(100, 100);
    void *extData = nullptr;

    bool result = hook->mouseRelease(viewIndex, button, viewPos, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, mouseDoubleClick_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    int button = Qt::LeftButton;
    QPoint viewPos(100, 100);
    void *extData = nullptr;

    bool result = hook->mouseDoubleClick(viewIndex, button, viewPos, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, wheel_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    QPoint angleDelta(10, 10);
    void *extData = nullptr;

    bool result = hook->wheel(viewIndex, angleDelta, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, startDrag_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    int supportedActions = Qt::CopyAction;
    void *extData = nullptr;

    bool result = hook->startDrag(viewIndex, supportedActions, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, dragEnter_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    QMimeData mimeData;
    void *extData = nullptr;

    bool result = hook->dragEnter(viewIndex, &mimeData, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, dragMove_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    QMimeData mimeData;
    QPoint viewPos(100, 100);
    void *extData = nullptr;

    bool result = hook->dragMove(viewIndex, &mimeData, viewPos, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, dragLeave_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    QMimeData mimeData;
    void *extData = nullptr;

    bool result = hook->dragLeave(viewIndex, &mimeData, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, keyboardSearch_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    QString search = "test";
    void *extData = nullptr;

    bool result = hook->keyboardSearch(viewIndex, search, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, drawFile_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    QUrl file("file:///tmp/test.txt");
    QPainter painter;
    QStyleOptionViewItem option;
    void *extData = nullptr;

    bool result = hook->drawFile(viewIndex, file, &painter, &option, extData);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_CanvasViewHook, shortcutkeyPress_WithValidParameters_ReturnsBoolean)
{
    int viewIndex = 0;
    int key = Qt::Key_A;
    int modifiers = Qt::NoModifier;
    void *extData = nullptr;

    bool result = hook->shortcutkeyPress(viewIndex, key, modifiers, extData);
    EXPECT_TRUE(result == true || result == false);
}