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
#include "models/fileproxymodel.h"
#include "view/collectionframe.h"
#include "collection/collectionholder_p.h"
#include "collection/collectionholder.h"
#include "view/collectionwidget.h"
#include "view/collectionview.h"
#include "view/collectionview_p.h"
#include "interface/canvasmanagershell.h"
#include "mode/normalized/fileclassifier.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;

class CollectionHolderTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};


TEST_F(CollectionHolderTest, setCanvasManagerShell)
{

    CanvasManagerShell *expectSh = nullptr;
    bool isCall = false;
    stub.set_lamda(ADDR(CollectionWidget, setCanvasManagerShell), [&] (CollectionWidget *obj, CanvasManagerShell *sh) {
        Q_UNUSED(obj)

        if (sh == expectSh)
            isCall = true;
    });

    stub.set_lamda(ADDR(CollectionViewPrivate, initUI), [] () {
        return ;
    });

    CollectionHolder holder(QString("testuuid"), nullptr);
    holder.d->widget = new CollectionWidget(QString("testuuid"), nullptr);

    holder.setCanvasManagerShell(expectSh);
    EXPECT_TRUE(isCall);

    isCall = false;
    expectSh = new CanvasManagerShell;
    holder.setCanvasManagerShell(expectSh);
    EXPECT_TRUE(isCall);

    expectSh->deleteLater();
    delete holder.d->widget;
}

TEST_F(CollectionHolderTest, widget)
{
    CollectionHolder holder(QString("testuuid"), nullptr);
    auto w = new CollectionWidget(QString("testuuid"), nullptr);
    holder.d->widget = w;

    EXPECT_EQ(holder.widget(), w);
    delete w;
}
