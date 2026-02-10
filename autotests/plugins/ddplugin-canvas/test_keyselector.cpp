// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/operator/keyselector.h"
#include "view/canvasview.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>

using namespace ddplugin_canvas;

class UT_KeySelector : public testing::Test
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
        selector = new KeySelector(view);
    }

    virtual void TearDown() override
    {
        if (selector) {
            delete selector;
            selector = nullptr;
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
    KeySelector *selector = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_KeySelector, constructor_CreateSelector_InitializesCorrectly)
{
    EXPECT_NE(selector, nullptr);
}

TEST_F(UT_KeySelector, filterKeys_WithValidView_ReturnsKeyList)
{
    QList<Qt::Key> keys = selector->filterKeys();
    EXPECT_FALSE(keys.isEmpty());
}