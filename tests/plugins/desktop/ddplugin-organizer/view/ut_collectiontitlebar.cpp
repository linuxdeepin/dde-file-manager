/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     wangchunlin<wangchunlin@uniontech.com>
 *
 * Maintainer: wangchunlin<wangchunlin@uniontech.com>
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
