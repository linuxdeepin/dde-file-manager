// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/canvasview_p.h"
#include "model/canvasproxymodel_p.h"

#include "stubext.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_canvas;

class CanvasViewTest : public Test
{
public:
    void SetUp() override {
        view = new CanvasView();
        model = new CanvasProxyModel();
        view->setModel(model);
    }
    void TearDown() override {
        stub.clear();
        delete view;
        delete model;
    }

    stub_ext::StubExt stub;
    CanvasView *view;
    CanvasProxyModel *model;
};

TEST_F(CanvasViewTest, refresh)
{
    bool refresh = false;
    stub.set_lamda(&CanvasProxyModel::refresh,[&refresh](){
        refresh = true;
    });
    bool filcker = false;
    stub.set_lamda((void (CanvasView::*)())&CanvasView::repaint,[&filcker](CanvasView *self){
        filcker = self->d->flicker;
    });

    view->refresh(true);
    EXPECT_TRUE(refresh);
    EXPECT_FALSE(filcker);

    refresh = false;
    filcker = false;
    view->refresh(false);
    EXPECT_TRUE(refresh);
    EXPECT_TRUE(filcker);
}
