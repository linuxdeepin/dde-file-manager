// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "wallpaperitem.h"
#include "editlabel.h"

#include <dfm-framework/dpf.h>
#include <QPushButton>
#include <QMouseEvent>
#include <QGridLayout>

#include "stubext.h"
#include <gtest/gtest.h>

DDP_WALLPAERSETTING_USE_NAMESPACE

class UT_wallpaperItem : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        item = new WallpaperItem;
    }
    virtual void TearDown() override
    {
        delete item;
        stub.clear();
    }
    WallpaperItem *item = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_wallpaperItem, setOpacity)
{
    item->setOpacity(0.5);
    EXPECT_TRUE(qFuzzyCompare(item->wrapper->opacity(), 0.5));
}

TEST_F(UT_wallpaperItem, slideUp)
{
    bool call = false;
    stub.set_lamda(&QAbstractAnimation::start, [&call, this](QAbstractAnimation *th, QAbstractAnimation::DeletionPolicy policy) {
        __DBG_STUB_INVOKE__
        if (th == this->item->upAnim)
            call = true;
        return;
    });
    item->slideUp();
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(&QWidget::y, []() {
        return -1;
    });
    stub.set_lamda(&QAbstractAnimation::state, []() {
        return QAbstractAnimation::Stopped;
    });
    item->slideUp();
    EXPECT_FALSE(call);
}

TEST_F(UT_wallpaperItem, slideDown)
{
    bool call = false;
    stub.set_lamda(&QWidget::y, []() {
        return -1;
    });
    stub.set_lamda(&QAbstractAnimation::start, [&call, this](QAbstractAnimation *th, QAbstractAnimation::DeletionPolicy policy) {
        __DBG_STUB_INVOKE__
        Q_UNUSED(policy);
        if (th == this->item->downAnim)
            call = true;
        return;
    });
    item->slideDown();
    EXPECT_TRUE(call);

    call = false;
    stub.set_lamda(&QWidget::y, []() {
        return 1;
    });
    stub.set_lamda(&QAbstractAnimation::state, []() {
        return QAbstractAnimation::Stopped;
    });
    item->slideDown();
    EXPECT_FALSE(call);
}

TEST_F(UT_wallpaperItem, addButton)
{
    bool connect = false;
    stub.set_lamda(&WallpaperItem::onButtonClicked, [&connect]() {
        connect = true;
    });

    QPushButton *btn = item->addButton("1", "test", 0, 0, 0, 0, 0);
    btn->clicked();
    EXPECT_TRUE(connect);
}

TEST_F(UT_wallpaperItem, mousePressEvent)
{
    QMouseEvent *mouseEvent = new QMouseEvent(QEvent::Type::MouseButtonRelease, { 10, 10 }, Qt::MouseButton::LeftButton,
                                              Qt::MouseButton::LeftButton, Qt::KeyboardModifier::AltModifier);
    bool emited = false;
    QObject::connect(item, &WallpaperItem::pressed, [&emited]() {
        emited = true;
    });
    item->mousePressEvent(mouseEvent);
    EXPECT_TRUE(emited);

    emited = false;
    mouseEvent->mouseState = Qt::RightButton;
    item->mousePressEvent(mouseEvent);
    EXPECT_FALSE(emited);

    delete mouseEvent;
}

TEST_F(UT_wallpaperItem, keyPressEvent)
{
    QWidget *wid1 = new QWidget();
    QWidget *wid2 = new QWidget();
    QWidget *wid3 = new QWidget();
    item->buttonLayout->addWidget(wid1);
    item->buttonLayout->addWidget(wid2);
    item->buttonLayout->addWidget(wid3);

    QWidget *ret = wid1;
    stub.set_lamda(&QWidget::focusWidget, [&ret]() {
        __DBG_STUB_INVOKE__
        return ret;
    });

    bool focusOnLast = false;
    stub.set_lamda(&WallpaperItem::focusOnLastButton, [&focusOnLast]() {
        focusOnLast = true;
    });

    QWidget *focusWidget = wid1;
    auto setFocusFunc = static_cast<void (QWidget::*)()>(&QWidget::setFocus);
    bool focus = false;
    stub.set_lamda(setFocusFunc, [&focusWidget, &focus](QWidget *th) {
        if (th == focusWidget)
            focus = true;
    });

    QKeyEvent *mouseEvent = new QKeyEvent(QEvent::Type::KeyPress, 0, Qt::KeyboardModifier::AltModifier);
    mouseEvent->k = Qt::Key_Up;
    {
        item->keyPressEvent(mouseEvent);
        EXPECT_TRUE(focusOnLast);
        EXPECT_FALSE(focus);
    }
    {
        focusOnLast = false;
        focus = false;
        ret = wid2;
        item->keyPressEvent(mouseEvent);
        EXPECT_FALSE(focusOnLast);
        EXPECT_TRUE(focus);
    }

    bool focusOnFirst = false;
    stub.set_lamda(&WallpaperItem::focusOnFirstButton, [&focusOnFirst]() {
        focusOnFirst = true;
    });
    mouseEvent->k = Qt::Key_Down;
    {
        ret = wid3;
        focusOnLast = false;
        focus = false;
        item->keyPressEvent(mouseEvent);
        EXPECT_TRUE(focusOnFirst);
        EXPECT_FALSE(focus);
    }
    {
        ret = wid2;
        focusOnFirst = false;
        focus = false;
        focusWidget = wid3;
        item->keyPressEvent(mouseEvent);
        EXPECT_FALSE(focusOnFirst);
        EXPECT_TRUE(focus);
    }

    delete wid1;
    delete wid2;
    delete wid3;
    delete mouseEvent;
}

TEST_F(UT_wallpaperItem, eventFilter)
{
    QPushButton *watched = new QPushButton;
    item->buttons.insert(watched, "1");
    QKeyEvent *keyEvent = new QKeyEvent(QEvent::Type::KeyPress, 0, Qt::KeyboardModifier::AltModifier);

    bool emitTab = false;
    bool emitBacktab = false;
    bool emitClicked = false;
    QObject::connect(item, &WallpaperItem::tab, [&emitTab]() {
        emitTab = true;
    });
    QObject::connect(item, &WallpaperItem::backtab, [&emitBacktab]() {
        emitBacktab = true;
    });
    QObject::connect(watched, &QPushButton::clicked, [&emitClicked]() {
        emitClicked = true;
    });
    {
        keyEvent->k = Qt::Key_Tab;
        item->eventFilter(watched, keyEvent);
        EXPECT_TRUE(emitTab);
        EXPECT_FALSE(emitBacktab);
        EXPECT_FALSE(emitClicked);
    }
    {
        emitTab = false;
        keyEvent->k = Qt::Key_Backtab;
        item->eventFilter(watched, keyEvent);
        EXPECT_TRUE(emitBacktab);
        EXPECT_FALSE(emitClicked);
        EXPECT_FALSE(emitTab);
    }
    {
        emitBacktab = false;
        keyEvent->k = Qt::Key_Space;
        item->eventFilter(watched, keyEvent);
        EXPECT_TRUE(emitClicked);

        emitClicked = false;
        keyEvent->k = Qt::Key_Enter;
        item->eventFilter(watched, keyEvent);
        EXPECT_TRUE(emitClicked);

        emitClicked = false;
        keyEvent->k = Qt::Key_Return;
        item->eventFilter(watched, keyEvent);
        EXPECT_TRUE(emitClicked);
        EXPECT_TRUE(emitClicked);
        EXPECT_FALSE(emitBacktab);
        EXPECT_FALSE(emitTab);
    }
    delete watched;
    delete keyEvent;
}
