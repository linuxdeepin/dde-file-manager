// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include <gtest/gtest.h>

#include "options/widgets/contentbackgroundwidget.h"

#include <DPalette>
#include <DApplicationHelper>

#include <QPainter>
#include <QPainterPath>
#include <QRect>
#include <QPaintEvent>
DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

TEST(ContentBackgroundWidget, paintEvent)
{
    ContentBackgroundWidget widget;
    QPaintEvent event(QRect(0, 0, 100, 100));
    stub_ext::StubExt stub;
    typedef void(*fun_type)(QPaintEvent* event);
    bool call = false;
    stub.set_lamda((fun_type)(&QWidget::paintEvent),[&call](QPaintEvent *){call= true;});

    widget.edge = ContentBackgroundWidget::RoundEdge::kTop;
    EXPECT_NO_FATAL_FAILURE(widget.paintEvent(&event));
    widget.edge = ContentBackgroundWidget::RoundEdge::kBottom;
    EXPECT_NO_FATAL_FAILURE(widget.paintEvent(&event));
    EXPECT_TRUE(call);
}
