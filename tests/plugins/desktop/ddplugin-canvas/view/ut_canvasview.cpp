/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

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
