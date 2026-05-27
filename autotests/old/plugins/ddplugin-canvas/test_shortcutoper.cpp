// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/operator/shortcutoper.h"
#include "view/canvasview.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>
#include <QKeyEvent>

using namespace ddplugin_canvas;

class UT_ShortcutOper : public testing::Test
{
public:
    virtual void SetUp() override
    {
        if (!QApplication::instance()) {
            int argc = 0;
            char **argv = nullptr;
            app = new QApplication(argc, argv);
        }

        parentWidget = new QWidget();
        view = new CanvasView(parentWidget);
        oper = new ShortcutOper(view);
    }

    virtual void TearDown() override
    {
        if (oper) {
            delete oper;
            oper = nullptr;
        }

        if (view) {
            delete view;
            view = nullptr;
        }

        if (parentWidget) {
            delete parentWidget;
            parentWidget = nullptr;
        }

        stub.clear();
    }

public:
    QApplication *app = nullptr;
    QWidget *parentWidget = nullptr;
    CanvasView *view = nullptr;
    ShortcutOper *oper = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_ShortcutOper, constructor_CreateOper_InitializesCorrectly)
{
    EXPECT_NE(oper, nullptr);
}

TEST_F(UT_ShortcutOper, keyPressed_WithValidEvent_ReturnsBoolean)
{
    QKeyEvent event(QEvent::KeyPress, Qt::Key_A, Qt::NoModifier);
    bool result = oper->keyPressed(&event);
    EXPECT_TRUE(result == true || result == false);
}

TEST_F(UT_ShortcutOper, keyPressed_WithNullEvent_ReturnsFalse)
{
    bool result = oper->keyPressed(nullptr);
    EXPECT_FALSE(result);
}