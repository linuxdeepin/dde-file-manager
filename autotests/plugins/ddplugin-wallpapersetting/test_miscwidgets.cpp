// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QApplication>
#include <QMouseEvent>

#include "editlabel.h"
#include "loadinglabel.h"

DDP_WALLPAERSETTING_USE_NAMESPACE

class UT_MiscWidgets : public testing::Test {
};

// [EditLabel]_[HotZoneClick]_[SignalEmitted]
TEST_F(UT_MiscWidgets, EditLabel_HotZoneClick_SignalEmitted)
{
    EditLabel label;
    label.setFixedSize(100, 40);
    label.setHotZoom(QRect(50, 0, 40, 40));
    bool clicked = false;
    QObject::connect(&label, &EditLabel::editLabelClicked, [&]() { clicked = true; });

    // Simulate mouse press inside hot area
    QMouseEvent ev(QEvent::MouseButtonPress, QPoint(60, 20), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    QApplication::sendEvent(&label, &ev);
    EXPECT_TRUE(clicked);
}

// [LoadingLabel]_[StartAndResize]_[LayoutOK]
TEST_F(UT_MiscWidgets, LoadingLabel_StartAndResize_LayoutOK)
{
    LoadingLabel loading;
    loading.resize(QSize(400, 100));
    loading.setText("Loading...");
    EXPECT_NO_THROW(loading.start());
}
