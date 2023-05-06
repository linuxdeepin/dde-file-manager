// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include <QMouseEvent>
#include "editlabel.h"

#include <gtest/gtest.h>
#include "stubext.h"

DDP_WALLPAERSETTING_USE_NAMESPACE

TEST(TestEditLabel, hotzoom)
{
    EditLabel editlabel;
    stub_ext::StubExt stub;
    editlabel.setGeometry(0, 0, 40, 40);
    QRect hotzoom(0, 0, 20, 20);

    QMouseEvent *mouseEvent = new QMouseEvent(QEvent::Type::MouseButtonRelease, { 10, 10 }, Qt::MouseButton::LeftButton,
                                              Qt::MouseButton::LeftButton, Qt::KeyboardModifier::AltModifier);

    editlabel.setHotZoom(hotzoom);
    bool call = false;
    stub.set_lamda(&EditLabel::editLabelClicked, [&call]() {
        __DBG_STUB_INVOKE__
        call = true;
        return;
    });
    editlabel.mousePressEvent(mouseEvent);
    EXPECT_TRUE(call);

    call = false;
    mouseEvent->l = QPointF(QPoint(30, 30));
    editlabel.mousePressEvent(mouseEvent);
    EXPECT_FALSE(call);

    call = false;
    mouseEvent->l = QPointF(QPoint(50, 50));
    editlabel.mousePressEvent(mouseEvent);
    EXPECT_FALSE(call);

    delete mouseEvent;
}
