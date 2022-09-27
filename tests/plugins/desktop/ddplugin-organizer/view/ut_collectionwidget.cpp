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

#include "stubext.h"
#include "view/collectionview.h"
#include "view/collectionview_p.h"
#include "view/collectionwidget_p.h"
#include "view/collectionwidget.h"
#include "view/collectiontitlebar.h"
#include "interface/canvasmanagershell.h"

#include <gtest/gtest.h>
#include <QMenu>

using namespace testing;
using namespace ddplugin_organizer;

class CollectionWidgetPrivateTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

class CollectionWidgetTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};

TEST_F(CollectionWidgetTest, setCanvasManagerShell)
{
    QString testUuid("tsetUuid");
    CollectionWidget widget(testUuid, nullptr);

    CanvasManagerShell *expectSh = nullptr;
    bool isCall = false;
    stub.set_lamda(ADDR(CollectionView, setCanvasManagerShell), [&](CollectionView *obj, CanvasManagerShell *sh) {
        Q_UNUSED(obj)

        if (sh == expectSh)
            isCall = true;
    });

    widget.setCanvasManagerShell(expectSh);
    EXPECT_TRUE(isCall);

    isCall = false;
    expectSh = new CanvasManagerShell;
    widget.setCanvasManagerShell(expectSh);
    EXPECT_TRUE(isCall);

    expectSh->deleteLater();
    expectSh = nullptr;
}

TEST_F(CollectionWidgetTest, setCollectionSize)
{

    CollectionFrameSize setSize = CollectionFrameSize::kLarge;
    bool isCall = false;
    stub.set_lamda(ADDR(CollectionTitleBar, setCollectionSize), [&] (CollectionTitleBar *obj, const CollectionFrameSize &size) {
        Q_UNUSED(obj)

        if (size == setSize)
            isCall = true;
    });

    QString testUuid("tsetUuid");
    CollectionWidget widget(testUuid, nullptr);

    widget.setCollectionSize(setSize);
    EXPECT_TRUE(isCall);
}

TEST_F(CollectionWidgetTest, collectionSize)
{
    CollectionFrameSize getSize = CollectionFrameSize::kSmall;
    bool isCall = false;
    stub.set_lamda(ADDR(CollectionTitleBar, collectionSize), [&] () {
        isCall = true;
        return getSize;
    });

    QString testUuid("tsetUuid");
    CollectionWidget widget(testUuid, nullptr);

    auto resutl = widget.collectionSize();
    EXPECT_TRUE(isCall);
    EXPECT_EQ(resutl, getSize);
}

TEST_F(CollectionWidgetTest, view)
{
    CollectionWidget widget("tsetUuid", nullptr);
    auto fake = reinterpret_cast<CollectionView *>(0x1);
    widget.d->view = fake;

    EXPECT_EQ(widget.view(), fake);
}
