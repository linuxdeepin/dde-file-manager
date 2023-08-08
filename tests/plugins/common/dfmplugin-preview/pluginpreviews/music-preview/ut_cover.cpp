// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "cover.h"

#include <gtest/gtest.h>

#include <QPaintEvent>

using namespace plugin_filepreview;

TEST(UT_cover, setCoverPixmap)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef void(QWidget::*FuncType)();
    stub.set_lamda(static_cast<FuncType>(&QWidget::update), [ &isOk ]{
        isOk = true;
    });

    Cover label;
    label.setCoverPixmap(QPixmap());

    EXPECT_TRUE(isOk);
}

TEST(UT_cover, paintEvent_one)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef void(QPainter::*FuncType)(const QRect &, qreal, qreal, Qt::SizeMode);
    stub.set_lamda(static_cast<FuncType>(&QPainter::drawRoundedRect), [ &isOk ]{
        isOk = true;
    });

    Cover label;
    QPaintEvent event(QRect(0, 0, 1, 1));
    label.paintEvent(&event);

    EXPECT_TRUE(isOk);
}

TEST(UT_cover, paintEvent_two)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef void(QPainter::*FuncType)(const QRect &, qreal, qreal, Qt::SizeMode);
    stub.set_lamda(static_cast<FuncType>(&QPainter::drawRoundedRect), [ &isOk ]{
        isOk = true;
    });
    stub.set_lamda(&QPixmap::isNull, []{
        return false;
    });

    Cover label;
    label.background = QPixmap(QSize(0, 0));
    QPaintEvent event(QRect(0, 0, 1, 1));
    label.paintEvent(&event);

    EXPECT_TRUE(isOk);
}

