// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
