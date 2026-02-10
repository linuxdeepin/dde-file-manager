// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>

#include "loadinglabel.h"

using namespace ddplugin_wallpapersetting;

// [LoadingLabel]_[StartAndResize]_[LayoutOK]
TEST(UT_LoadingLabel_Original, StartAndResize_LayoutOK)
{
    LoadingLabel loading;
    loading.resize(QSize(400, 100));
    loading.setText("Loading...");
    EXPECT_NO_THROW(loading.start());
}
