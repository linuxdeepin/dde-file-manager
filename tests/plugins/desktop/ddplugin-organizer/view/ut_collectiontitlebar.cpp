// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "view/collectiontitlebar_p.h"
#include "view/collectiontitlebar.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QMenu>

using namespace testing;
using namespace ddplugin_organizer;

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
