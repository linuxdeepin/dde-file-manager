/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
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

#include "organizer_defines.h"
#include "options/widgets/switchwidget.h"

#include "stubext.h"

#include <QLabel>

#include <gtest/gtest.h>

DWIDGET_USE_NAMESPACE
DDP_ORGANIZER_USE_NAMESPACE

TEST(SwitchWidget, construct)
{
    const QString title = "xxx";
    SwitchWidget sw(title);
    ASSERT_NE(sw.label, nullptr);
    ASSERT_NE(sw.switchBtn, nullptr);

    EXPECT_EQ(sw.label->text(), title);
}

TEST(SwitchWidget, checked)
{
    SwitchWidget sw("");
    EXPECT_EQ(sw.checked(), sw.switchBtn->isChecked());
}

TEST(SwitchWidget, setChecked)
{
    SwitchWidget sw("");
    bool sig = false;
    sw.connect(&sw, &SwitchWidget::checkedChanged, [&sig](){
        sig = true;
    });

    bool toggled = false;
    sw.connect(sw.switchBtn, &DSwitchButton::checkedChanged, [&toggled](){
        toggled = true;
    });

    bool old = sw.checked();
    sw.setChecked(!old);

    EXPECT_EQ(sw.checked(), !old);
    EXPECT_TRUE(toggled);
    EXPECT_FALSE(sig);
}
