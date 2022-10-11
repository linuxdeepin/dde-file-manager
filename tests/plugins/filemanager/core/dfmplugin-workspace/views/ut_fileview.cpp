/*
* Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
*
* Author:     gongheng <gongheng@uniontech.com>
*
* Maintainer: zhengyouge <zhengyouge@uniontech.com>
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

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-workspace/views/fileview.h"
#include "plugins/filemanager/core/dfmplugin-workspace/views/private/fileview_p.h"
#include "dfm-base/base/application/application.h"

#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

class UT_FileView : public testing::Test
{
protected:
    void SetUp() override { }
    void TearDown() override { stub.clear(); }
private:
    stub_ext::StubExt stub;
};

TEST_F(UT_FileView, setViewSelectState)
{
    stub.set_lamda(&Application::appAttribute, [](Application::ApplicationAttribute)->QVariant {
        return QVariant();
    });
    stub.set_lamda(&FileView::initializeModel, [](){});
    stub.set_lamda(&FileView::updateModelActiveIndex, [](){});

    FileView view(QUrl("~"));
    view.setViewSelectState(true);
    EXPECT_TRUE(view.d->isShowViewSelectBox);
}

TEST_F(UT_FileView, paintEvent)
{
    stub.set_lamda(&Application::appAttribute, [](Application::ApplicationAttribute)->QVariant {
        return QVariant();
    });
    stub.set_lamda(&FileView::initializeModel, [](){});
    stub.set_lamda(&FileView::updateModelActiveIndex, [](){});

    bool flag = false;
    auto drawRectFoo = (void(QPainter::*)(const QRectF&))(&QPainter::drawRect);
    stub.set_lamda(drawRectFoo, [&flag](){
        flag = true;
    });

    FileView view(QUrl("~"));
    view.setViewSelectState(true);
    QPaintEvent event(QRect(0, 0, 1, 1));
    EXPECT_NO_FATAL_FAILURE(view.paintEvent(&event));
    EXPECT_FALSE(view.viewport() == nullptr);
    EXPECT_TRUE(flag);
}
