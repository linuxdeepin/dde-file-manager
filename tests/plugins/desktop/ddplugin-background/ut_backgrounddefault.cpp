// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "backgrounddefault.h"

#include "stubext.h"
#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QBackingStore>
#include <QPainter>
#include <QFileInfo>
#include <QImageReader>
#include <QPaintDevice>
#include <qpa/qplatformwindow.h>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformbackingstore.h>

DDP_BACKGROUND_BEGIN_NAMESPACE
DDP_BACKGROUND_USE_NAMESPACE

DDP_BACKGROUND_USE_NAMESPACE
TEST(BackgroundDefault, setPixmap)
{
    QPixmap pix;
    stub_ext::StubExt stub;
    BackgroundDefault def(QString("Default"));

    stub.set_lamda((void(QWidget::*)())&QWidget::update,[](QWidget* self){
        __DBG_STUB_INVOKE__
        return;
    });
    def.setPixmap(pix);
    EXPECT_EQ(def.pixmap,pix);
    EXPECT_EQ(def.noScalePixmap,pix);
    EXPECT_EQ(def.noScalePixmap.devicePixelRatio(),1);
}


TEST(BackgroundDefault,paintEvent)
{
    stub_ext::StubExt stub;
    BackgroundDefault def(QString("Default"));
    QPaintEvent event(QRect(0,0,1,1));
    event.m_rect = QRect(0,0,1,1);
    bool call = false;
    stub.set_lamda(& QPixmap::isNull,[](){
        __DBG_STUB_INVOKE__
        return false;
    });
    stub.set_lamda(&QPaintDevice::devicePixelRatioF,[](){
       __DBG_STUB_INVOKE__
        return qreal(2.0);
    });

    stub.set_lamda(&QWidget::rect,[&call](){
        __DBG_STUB_INVOKE__
        call = true;
       return QRect(0,0,2,2);
    });

    def.paintEvent(&event);
    EXPECT_TRUE(call);
}


DDP_BACKGROUND_END_NAMESPACE

