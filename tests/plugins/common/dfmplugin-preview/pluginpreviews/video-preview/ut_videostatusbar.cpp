// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "videostatusbar.h"
#include "videopreview.h"

#include <gtest/gtest.h>

PREVIEW_USE_NAMESPACE

TEST(UT_VideoStatusBar, Constructor)
{
    VideoPreview view;
    VideoStatusBar bar(&view);

    EXPECT_TRUE(bar.slider);
}
