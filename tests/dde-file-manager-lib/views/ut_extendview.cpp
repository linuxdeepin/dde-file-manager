// SPDX-FileCopyrightText: 2020 - 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "views/extendview.h"

TEST(ExtendViewTest,init)
{
    ExtendView w(nullptr);
    EXPECT_NE(nullptr,w.m_extendListView);
    EXPECT_NE(nullptr,w.m_detailView);
}

TEST(ExtendViewTest,set_start_url)
{
    ExtendView w;
    DUrl url("file:///home");
    w.setStartUrl(url);
    EXPECT_EQ(url, w.m_currentUrl);
}
