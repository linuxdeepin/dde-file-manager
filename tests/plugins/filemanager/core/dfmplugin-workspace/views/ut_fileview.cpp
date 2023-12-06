// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-workspace/views/fileview.h"
#include "plugins/filemanager/core/dfmplugin-workspace/views/private/fileview_p.h"
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

TEST_F(UT_FileView, Bug_224471_setViewMode)
{
    stub.set_lamda(&FileView::initializeModel, [](){});

    FileView view(QUrl(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()));
    view.setViewMode(DFMGLOBAL_NAMESPACE::ViewMode::kListMode);

    EXPECT_EQ(view.iconSize().width(), kListViewIconSize);
    EXPECT_EQ(view.iconSize().height(), kListViewIconSize);
}

TEST_F(UT_FileView, Bug_232671_setViewMode)
{
    stub.set_lamda(&FileView::initializeModel, [](){});

    FileView view(QUrl(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first()));

    view.setVisible(false);
    view.setViewMode(DFMGLOBAL_NAMESPACE::ViewMode::kIconMode);

    EXPECT_TRUE(view.itemDelegate() != nullptr);
    EXPECT_EQ(view.currentViewMode(), DFMGLOBAL_NAMESPACE::ViewMode::kIconMode);

    view.setVisible(true);
    view.setViewMode(DFMGLOBAL_NAMESPACE::ViewMode::kListMode);
    EXPECT_NE(view.currentViewMode(), DFMGLOBAL_NAMESPACE::ViewMode::kListMode);
}
