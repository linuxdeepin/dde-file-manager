// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QHBoxLayout>
#include "wallpaperlist.h"
#include "private/wallpapersettings_p.h"
#include "desktoputils/ddpugin_eventinterface_helper.h"

#include <dfm-base/dfm_desktop_defines.h>
#include <dfm-framework/dpf.h>
#include <QTimer>
#include <QWidget>

#include "stubext.h"
#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
DPF_USE_NAMESPACE
DDP_WALLPAERSETTING_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(WallpaperList, init)
{
    stub_ext::StubExt stub;
    bool connect1 = false;
    bool connect2 = false;
    bool connect3 = false;
    bool connect4 = false;
    stub.set_lamda(&WallpaperList::updateItemThumb, [&connect1]() {
        connect1 = true;
    });
    auto startFunc = static_cast<void (QTimer::*)()>(&QTimer::start);
    stub.set_lamda(startFunc, [&connect2]() {
        connect2 = true;
    });
    stub.set_lamda(&WallpaperList::prevPage, [&connect3]() {
        connect3 = true;
    });
    stub.set_lamda(&WallpaperList::nextPage, [&connect4]() {
        connect4 = true;
    });

    WallpaperList *list;
    EXPECT_NO_FATAL_FAILURE(list = new WallpaperList);
    emit list->updateTimer->timeout(QTimer::QPrivateSignal());
    emit list->scrollAnimation.finished();
    emit list->prevButton.widget()->clicked();
    emit list->nextButton.widget()->clicked();

    EXPECT_TRUE(connect1);
    EXPECT_TRUE(connect2);
    EXPECT_TRUE(connect3);
    EXPECT_TRUE(connect4);
}

class UT_wallpaperList : public testing::Test
{
protected:
    virtual void SetUp() override
    {
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
    delete maskWidget;
}

TEST_F(UT_wallpaperList, Item)
{
    QStringList itemDataList;

    bool connect1 = false;
    bool connect2 = false;
    bool connect3 = false;
    stub.set_lamda(&WallpaperList::onItemPressed, [&connect1]() {
        connect1 = true;
    });
    stub.set_lamda(&WallpaperList::onItemHoverIn, [&connect2]() {
        connect2 = true;
    });
    stub.set_lamda(&WallpaperList::onItemHoverOut, [&connect3]() {
        connect3 = true;
    });

    itemDataList << "test1"
                 << "test2"
                 << "test3";
    for (auto itemData : itemDataList)
        list->addItem(itemData);

    EXPECT_EQ(list->items.size(), 3);
    emit list->items.at(0)->pressed(new WallpaperItem);
    emit list->items.at(0)->hoverIn(new WallpaperItem);
    emit list->items.at(0)->hoverOut(new WallpaperItem);
    EXPECT_TRUE(connect1);
    EXPECT_TRUE(connect2);
    EXPECT_TRUE(connect3);

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
    delete mouseEvent;
}

TEST_F(UT_wallpaperList, gridSize)
{
    list->grid = QSize(10, 10);
    bool call = false;
    stub.set_lamda(&QWidget::adjustSize, [&call]() {
        call = true;
        return;
    });
    list->setGridSize(QSize(20, 20));
    EXPECT_EQ(list->grid, QSize(20, 20));
    EXPECT_TRUE(call);
    call = false;

    list->setGridSize(QSize(20, 20));
    EXPECT_EQ(list->grid, QSize(20, 20));
    EXPECT_FALSE(call);
}

TEST_F(UT_wallpaperList, clear)
{
    WallpaperItem *item1 = new WallpaperItem;
    WallpaperItem *item2 = new WallpaperItem;
    WallpaperItem *item3 = new WallpaperItem;
    list->items.append(item1);
    list->items.append(item2);
    list->items.append(item3);
    list->contentLayout->addWidget(item3);
    list->contentLayout->addWidget(item1);
    list->contentLayout->addWidget(item2);

    list->clear();
    EXPECT_TRUE(list->items.isEmpty());
    EXPECT_TRUE(list->contentLayout->isEmpty());

    delete item1;
    delete item2;
    delete item3;
}

TEST_F(UT_wallpaperList, setCurrentIndex)
{
    WallpaperItem *item1 = new WallpaperItem;
    WallpaperItem *item2 = new WallpaperItem;
    WallpaperItem *item3 = new WallpaperItem;
    list->items.append(item1);
    list->items.append(item2);
    list->items.append(item3);
    list->contentLayout->addWidget(item3);
    list->contentLayout->addWidget(item1);
    list->contentLayout->addWidget(item2);

    bool slideUp = false;
    stub.set_lamda(&WallpaperItem::slideUp, [&slideUp, item2](WallpaperItem *t) {
        if (t == item2)
            slideUp = true;
        return;
    });
    stub.set_lamda(&WallpaperItem::slideDown, []() {
        return;
    });

    WallpaperSettingsPrivate pr(nullptr);
    QObject::connect(list, &WallpaperList::itemPressed, &pr, &WallpaperSettingsPrivate::onItemPressed);
    bool emited = false;
    stub.set_lamda(&WallpaperSettingsPrivate::onItemPressed, [&emited]() {
        emited = true;
    });
    bool animationStart = false;
    stub.set_lamda(&QPropertyAnimation::start, [&animationStart]() {
        animationStart = true;
        return;
    });

    {
        list->setCurrentIndex(1);
        EXPECT_TRUE(slideUp);
        EXPECT_TRUE(emited);
        EXPECT_TRUE(animationStart);
        EXPECT_EQ(list->currentIndex, 1);
    }
    {
        slideUp = false;
        emited = false;
        animationStart = false;

        list->setCurrentIndex(4);
        EXPECT_FALSE(slideUp);
        EXPECT_FALSE(emited);
        EXPECT_FALSE(animationStart);
    }
    delete item1;
    delete item2;
    delete item3;
}

TEST_F(UT_wallpaperList, itemAt)
{
    WallpaperItem *item1 = new WallpaperItem;
    WallpaperItem *item2 = new WallpaperItem;
    list->contentLayout->addWidget(item1);
    list->contentLayout->addWidget(item2);

    EXPECT_EQ(list->itemAt(0), item1);
    EXPECT_EQ(list->itemAt(1), item2);
    EXPECT_EQ(list->itemAt(2), nullptr);

    delete item1;
    delete item2;
}
