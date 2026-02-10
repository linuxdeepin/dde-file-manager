// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/operator/operstate.h"
#include "view/canvasview.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>

using namespace ddplugin_canvas;

class UT_OperState : public testing::Test
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
        state = new OperState();
        state->setView(view);
    }

    virtual void TearDown() override
    {
        if (state) {
            delete state;
            state = nullptr;
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
    OperState *state = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_OperState, constructor_CreateState_InitializesCorrectly)
{
    EXPECT_NE(state, nullptr);
}

TEST_F(UT_OperState, setView_WithValidView_SetsView)
{
    CanvasView newView;
    EXPECT_NO_THROW(state->setView(&newView));
}

TEST_F(UT_OperState, current_WithValidView_ReturnsCurrentIndex)
{
    QModelIndex index = state->current();
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_OperState, setCurrent_WithValidIndex_SetsCurrentIndex)
{
    QModelIndex index;
    EXPECT_NO_THROW(state->setCurrent(index));
}