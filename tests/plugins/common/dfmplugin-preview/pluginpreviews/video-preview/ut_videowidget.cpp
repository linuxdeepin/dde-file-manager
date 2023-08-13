// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "videowidget.h"
#include "videopreview.h"

#include <gtest/gtest.h>

PREVIEW_USE_NAMESPACE

TEST(UT_VideoWidget, sizeHint)
{
    VideoPreview preview;
    VideoWidget widget(&preview);
    QSize size = widget.sizeHint();

    EXPECT_TRUE(size.width() != 0);
}

TEST(UT_VideoWidget, mouseReleaseEvent)
{
    bool isOk { false };

    stub_ext::StubExt stub;
    typedef void(*FuncType1)();
    stub.set_lamda((FuncType1)(void(VideoPreview::*)())&VideoPreview::pause, [ &isOk ]{
        isOk = true;
    });

    VideoPreview preview;
    VideoWidget widget(&preview);
    QMouseEvent event(QEvent::MouseButtonRelease, QPoint(0, 0), Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    widget.mouseReleaseEvent(&event);

    EXPECT_TRUE(isOk);
}
