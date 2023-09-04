// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/collectiontitlebar_p.h"
#include "view/collectiontitlebar.h"

#include "stubext.h"


#include <gtest/gtest.h>

#include <QMenu>
#include <QPaintEvent>
#include <DStyle>
#include <DIconButton>

#include <QEvent>
#include <QFont>
#include <QFontMetrics>
#include <QPalette>
#include <QPaintEvent>
#include <QPainterPath>
#include <DFontSizeManager>

using namespace testing;
using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE
class CollectionTitleBarPrivateTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

class CollectionTitleBarTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(CollectionTitleBarPrivateTest, showMenu) {

    QString testUuid("testUuid");
    CollectionTitleBar titleBar(testUuid);

    titleBar.d->adjustable = true;
    titleBar.d->renamable = true;
    titleBar.d->closable = true;

    titleBar.d->size = CollectionFrameSize::kLarge;

    bool isCall = false;
    stub.set_lamda((QAction *(QMenu::*)(const QPoint &, QAction *))ADDR(QMenu, exec), [&] () {
        isCall = true;
        EXPECT_FALSE(titleBar.d->menu->actions().isEmpty());
        return nullptr;
    });

    titleBar.d->showMenu();
    EXPECT_TRUE(isCall);
}

TEST_F(CollectionTitleBarPrivateTest, modifyTitleName)
{
    QString testUuid("testUuid");
    CollectionTitleBar titleBar(testUuid);

    titleBar.rounded();

    titleBar.d->renamable = true;
    titleBar.d->q->setAttribute(Qt::WA_WState_Hidden);
    titleBar.d->titleName = "temp_name";
    Dtk::Widget::DStackedWidget widget;
    titleBar.d->nameWidget = &widget;
    Dtk::Widget::DLineEdit edit ;
    titleBar.d->nameLineEdit = &edit;

    titleBar.d->modifyTitleName();
    EXPECT_FALSE(titleBar.d->q->isHidden());
    EXPECT_EQ(titleBar.d->nameLineEdit->text(),"temp_name");
    EXPECT_EQ(titleBar.d->nameWidget->currentWidget(),nullptr);

    titleBar.d->titleNameModified();
    EXPECT_EQ(titleBar.d->titleName,"temp_name");
}

TEST_F(CollectionTitleBarTest, setCollectionSize) {

    QString testUuid("testUuid");
    CollectionTitleBar titleBar(testUuid);

    titleBar.d->size = CollectionFrameSize::kLarge;
    titleBar.setCollectionSize(CollectionFrameSize::kSmall);
    EXPECT_EQ(titleBar.d->size, CollectionFrameSize::kSmall);

    titleBar.setCollectionSize(CollectionFrameSize::kLarge);
    EXPECT_EQ(titleBar.d->size, CollectionFrameSize::kLarge);
}


TEST_F(CollectionTitleBarTest, collectionSize) {

    QString testUuid("testUuid");
    CollectionTitleBar titleBar(testUuid);

    titleBar.d->size = CollectionFrameSize::kLarge;
    EXPECT_EQ(titleBar.collectionSize(), CollectionFrameSize::kLarge);

    titleBar.d->size = CollectionFrameSize::kSmall;
    EXPECT_EQ(titleBar.collectionSize(), CollectionFrameSize::kSmall);
}


TEST(OptionButton, paintEvent)
{
    stub_ext::StubExt stub;
    OptionButton but;

    auto fun_type = static_cast<bool(QFlags<QStyle::StateFlag>::*)(QStyle::StateFlag)const>(&QFlags<QStyle::StateFlag>::testFlag);
    stub.set_lamda(fun_type,[](QFlags<QStyle::StateFlag> *self,QStyle::StateFlag )->bool{
        __DBG_STUB_INVOKE__
        return true;
    });

    QPaintEvent event(QRect(1,1,2,2));
    but.paintEvent(&event);
    EXPECT_TRUE(event.m_accept);
}

