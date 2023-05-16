// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-workspace/views/fileview.h"
#include "plugins/filemanager/core/dfmplugin-workspace/views/private/fileview_p.h"
#include "plugins/filemanager/core/dfmplugin-workspace/models/fileviewmodel.h"
#include <dfm-base/base/application/application.h>

#include <gtest/gtest.h>

#include <QStandardPaths>
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

    FileView view(QUrl(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()));
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

    FileView view(QUrl(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()));
    view.setViewSelectState(true);
    QPaintEvent event(QRect(0, 0, 1, 1));
    EXPECT_NO_FATAL_FAILURE(view.paintEvent(&event));
    EXPECT_FALSE(view.viewport() == nullptr);
    EXPECT_TRUE(flag);
}

TEST_F(UT_FileView, Bug_200133_updateModelActiveIndex)
{
    FileView::RandeIndexList indexList = { QPair<int, int>(0, 10) };
    stub.set_lamda(ADDR(FileView, visibleIndexes), [&indexList]{
        return indexList;
    });

    int setInactiveCount = 0;
    stub.set_lamda(ADDR(FileViewModel, setIndexActive), [&setInactiveCount](FileViewModel*, const QModelIndex &, bool active){
        if(!active)
            ++setInactiveCount;
    });

    int setActiveCount = 0;
    stub.set_lamda(ADDR(FileView, updateVisibleIndex), [&setActiveCount]{
        ++setActiveCount;
    });

    FileView view(QUrl(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()));

    view.updateModelActiveIndex();
    EXPECT_EQ(setActiveCount, 11);

    indexList = { QPair<int, int>(5, 15) };
    setInactiveCount = 0;
    view.updateModelActiveIndex();
    EXPECT_EQ(setInactiveCount, 5);

    indexList = { QPair<int, int>(20, 30) };
    setInactiveCount = 0;
    view.updateModelActiveIndex();
    EXPECT_EQ(setInactiveCount, 11);

    indexList = { QPair<int, int>(15, 35) };
    setActiveCount = 0;
    view.updateModelActiveIndex();
    EXPECT_EQ(setActiveCount, 21);

    indexList = { QPair<int, int>(20, 30) };
    setInactiveCount = 0;
    view.updateModelActiveIndex();
    EXPECT_EQ(setInactiveCount, 10);

    indexList = { QPair<int, int>(15, 25) };
    setInactiveCount = 0;
    view.updateModelActiveIndex();
    EXPECT_EQ(setInactiveCount, 5);

    indexList = { QPair<int, int>(0, 10) };
    setInactiveCount = 0;
    view.updateModelActiveIndex();
    EXPECT_EQ(setInactiveCount, 11);
}
