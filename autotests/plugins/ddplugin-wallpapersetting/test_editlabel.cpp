// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>

#include "editlabel.h"

DDP_WALLPAERSETTING_USE_NAMESPACE

// [EditLabel]_[OutsideHotZone_RouteToBase]_[NoSignal]
TEST(UT_EditLabel_Original, OutsideHotZone_RouteToBase_NoSignal)
{
    EditLabel label;
    label.setFixedSize(100, 40);
    label.setHotZoom(QRect(50, 0, 40, 40));
    bool clicked = false;
    QObject::connect(&label, &EditLabel::editLabelClicked, [&]() { clicked = true; });

    // Simulate mouse press outside hot area
    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(10, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&label, &ev);
    EXPECT_FALSE(clicked);
}
