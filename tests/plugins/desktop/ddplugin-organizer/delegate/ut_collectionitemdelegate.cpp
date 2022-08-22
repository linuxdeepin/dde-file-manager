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
#include "delegate/collectionitemdelegate.h"
#include "delegate/collectionitemdelegate_p.h"
#include "view/collectionview.h"
#include "view/collectionview_p.h"

#include <gtest/gtest.h>

using namespace testing;
using namespace ddplugin_organizer;

class CollectionItemDelegateTest : public Test
{
public:
    virtual void SetUp() override {}
    virtual void TearDown() override { stub.clear(); }

    stub_ext::StubExt stub;
};


TEST_F(CollectionItemDelegateTest, updateItemSizeHint) {

    stub.set_lamda(ADDR(CollectionViewPrivate, initUI), [] () {
        return ;
    });

    CollectionView view(QString("testuuid"), nullptr);
    CollectionItemDelegate delegate(&view);

    QSize testSize(100, 100);
    stub.set_lamda(ADDR(QAbstractItemView, iconSize), [&]() {
        return testSize;
    });

    delegate.d->textLineHeight = 0;

    delegate.updateItemSizeHint();

    EXPECT_EQ(delegate.d->itemSizeHint, QSize(170, 110));
}
