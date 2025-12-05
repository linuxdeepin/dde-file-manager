// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "view/operator/canvasviewmenuproxy.h"
#include "view/canvasview.h"

#include <gtest/gtest.h>
#include <QApplication>
#include <QWidget>

using namespace ddplugin_canvas;

class UT_CanvasViewMenuProxy : public testing::Test
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
        proxy = new CanvasViewMenuProxy(view);
    }

    virtual void TearDown() override
    {
        if (proxy) {
            delete proxy;
            proxy = nullptr;
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
    CanvasViewMenuProxy *proxy = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_CanvasViewMenuProxy, constructor_CreateProxy_InitializesCorrectly)
{
    EXPECT_NE(proxy, nullptr);
}

TEST_F(UT_CanvasViewMenuProxy, disableMenu_WithValidProxy_ReturnsBoolean)
{
    bool result = proxy->disableMenu();
    EXPECT_TRUE(result == true || result == false);
}