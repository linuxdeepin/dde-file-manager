// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "backgroundpreview.h"

#include "stubext.h"

#include <gtest/gtest.h>
#include <QPaintEvent>
#include <QWindow>
#include <QBackingStore>
#include <QPainter>
DDP_WALLPAERSETTING_USE_NAMESPACE
using namespace ddplugin_wallpapersetting;
TEST(BackgroundPreview, BackgroundPreview)
{
    BackgroundPreview wid(":0");
    EXPECT_EQ(wid.screen, QString(":0"));
    EXPECT_TRUE(wid.testAttribute(Qt::WA_TranslucentBackground));
}


TEST(BackgroundPreview, setDisplay)
{
    BackgroundPreview wid(":0");
    stub_ext::StubExt stub;
    bool call = false;
    stub.set_lamda(VADDR(BackgroundPreview, updateDisplay), [&call](){
        call = true;
    });

    wid.setDisplay("/test");
    EXPECT_EQ(wid.filePath, QString("/test"));
    EXPECT_TRUE(call);
}

TEST(BackgroundPreview, getPixmap_empty)
{
    BackgroundPreview wid(":0");
    QPixmap pix(100, 100);

    EXPECT_EQ(wid.getPixmap("", pix), pix);
}

TEST(BackgroundPreview, getPixmap_invalid)
{
    BackgroundPreview wid(":0");
    QPixmap pix(100, 100);

    EXPECT_EQ(wid.getPixmap("/tmp/dde-desktop/ssssssssssssssssssx.png", pix), pix);
}

TEST(BackgroundPreview, paintEvent)
{
    BackgroundPreview wid(":0");
    QRect rect(0,0,2,2);
    QPaintEvent event(rect);
    stub_ext::StubExt stub;
    auto fun_type = static_cast<qreal(QPaintDevice::*)()const>(&QPaintDevice::devicePixelRatioF);
    stub.set_lamda(fun_type, [](){
        __DBG_STUB_INVOKE__
        return 1.0;
    });

    auto fun_type1 = static_cast<QRect(QWidget::*)()const>(&QWidget::rect);
    stub.set_lamda(fun_type1,[](){
        __DBG_STUB_INVOKE__
        return QRect(0,0,2,2);
    });

    auto fun_type2 = static_cast<QBackingStore*(QWidget::*)()const>(&QWidget::backingStore);
    stub.set_lamda(fun_type2,[](){
        __DBG_STUB_INVOKE__
        QWindow *win = new QWindow;
        QBackingStore backingStore(win);
        return &backingStore;
    });
    bool call = false;

    auto fun_type3 = static_cast<void (QPainter::*)(const QPointF &p, const QPixmap &pm, const QRectF &sr)>(&QPainter::drawPixmap);
    stub.set_lamda(fun_type3,[&call](QPainter *self,const QPointF &p, const QPixmap &pm, const QRectF &sr){
        __DBG_STUB_INVOKE__
        call = true;
        return;
    });

    wid.paintEvent(&event);
    EXPECT_TRUE(call);
}

TEST(BackgroundPreview, updateDisplay)
{
    BackgroundPreview wid(":0");
    typedef QMap<QString,QWidget*> qmsw;
    stub_ext::StubExt stub;
    QWidget win;
    auto fun_type = static_cast<QWidget* const(qmsw::*)(const QString &, QWidget* const &) const>(&qmsw::value);
    stub.set_lamda(fun_type, [&win](){
        __DBG_STUB_INVOKE__
        return &win;
    });

    wid.updateDisplay();

    EXPECT_EQ(1.0,wid.noScalePixmap.devicePixelRatio());
}
