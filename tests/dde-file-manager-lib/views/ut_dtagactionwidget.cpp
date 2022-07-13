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

#include "views/dtagactionwidget.h"

TEST(DTagActionWidgetTest,set_checked_color_list_empty)
{
    DTagActionWidget wid;
    wid.setCheckedColorList({});
    EXPECT_EQ(true,wid.checkedColorList().isEmpty());
}

TEST(DTagActionWidgetTest,set_checked_color_list_invaild)
{
    DTagActionWidget wid;
    wid.setCheckedColorList({Qt::red});
    EXPECT_EQ(true,wid.checkedColorList().isEmpty());
}

TEST(DTagActionWidgetTest,set_checked_color_list)
{
    DTagActionWidget wid;
    QColor c{"#ffa503"};
    wid.setCheckedColorList({c});
    ASSERT_EQ(false,wid.checkedColorList().isEmpty());
    EXPECT_EQ(c,wid.checkedColorList().first());
}

TEST(DTagActionWidgetTest,set_exclusive_false)
{
    DTagActionWidget wid;
    wid.setExclusive(false);
    EXPECT_EQ(false,wid.exclusive());
    EXPECT_EQ(false,wid.property("exclusive").toBool());
}

TEST(DTagActionWidgetTest,set_exclusive_true)
{
    DTagActionWidget wid;
    wid.setExclusive(true);
    EXPECT_EQ(true,wid.exclusive());
    EXPECT_EQ(true,wid.property("exclusive").toBool());
}

TEST(DTagActionWidgetTest,set_tooltip_text)
{
    DTagActionWidget wid;
    wid.setToolTipVisible(true);

    wid.setToolTipText("test");
    wid.clearToolTipText();
}
