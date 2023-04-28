// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QHBoxLayout>
#include "wallpaperlist.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-framework/dpf.h>

#include "stubext.h"
#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DDP_WALLPAERSETTING_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

class UT_wallpaperList : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        stub.set_lamda(&WallpaperList::init, [](WallpaperList *self) {
            self->prevButton = new DImageButton("",
                                                "",
                                                "", self);
            self->nextButton = new DImageButton("",
                                                "",
                                                "", self);
            self->updateTimer = new QTimer(self);
            self->contentWidget = new QWidget(self);
            self->contentLayout = new QHBoxLayout(self);
            return;
        });
        list = new WallpaperList;
    }
    virtual void TearDown() override
    {
        delete list;
        stub.clear();
    }
    WallpaperList *list = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_wallpaperList, MaskWidget)
{
    QWidget *maskWidget = new QWidget;
    list->setMaskWidget(maskWidget);
    EXPECT_EQ(list->widget(), maskWidget);
    list->removeMaskWidget();
    EXPECT_NE(list->widget(), maskWidget);
    EXPECT_EQ(list->widget(), list->contentWidget);
}

TEST_F(UT_wallpaperList, Item)
{
    QStringList itemDataList;
    itemDataList << "test1"
                 << "test2"
                 << "test3";
    for (auto itemData : itemDataList)
        list->addItem(itemData);

    EXPECT_EQ(list->items.size(), 3);
    list->removeItem("test2");
    EXPECT_EQ(list->items.size(), 2);
    for (auto item : list->items)
        EXPECT_NE(item->itemData(), "test2");
}

TEST_F(UT_wallpaperList, keypressEvent)
{
    auto now = list->currentIndex;
    QKeyEvent *mouseEvent = new QKeyEvent(QEvent::Type::KeyPress, 0, Qt::KeyboardModifier::AltModifier);
    mouseEvent->k = Qt::Key_Right;
    int ret = now;
    stub.set_lamda(&WallpaperList::setCurrentIndex, [&ret](WallpaperList *self, int index) {
        __DBG_STUB_INVOKE__
        Q_UNUSED(self);
        ret = index;
        return;
    });
    list->keyPressEvent(mouseEvent);
    EXPECT_EQ(now, ret - 1);

    mouseEvent->k = Qt::Key_Left;
    list->keyPressEvent(mouseEvent);
    EXPECT_EQ(now, ret + 1);
}
