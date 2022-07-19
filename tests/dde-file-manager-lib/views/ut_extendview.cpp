/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangyu<zhangyub@uniontech.com>
 *
 * Maintainer: zhangyu<zhangyub@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <gtest/gtest.h>
#include <gmock/gmock-matchers.h>

#define private public
#include "views/extendview.h"

TEST(ExtendViewTest,init)
{
    ExtendView w;
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
