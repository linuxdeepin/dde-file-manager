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
#include "options/widgets/checkboxwidget.h"

#include "stubext.h"

#include <QLabel>

#include <gtest/gtest.h>

DDP_ORGANIZER_USE_NAMESPACE
DWIDGET_USE_NAMESPACE

TEST(CheckBoxWidget, construct)
{
    const QString title = "xxx";
    CheckBoxWidget wid(title);
    ASSERT_NE(wid.checkBox, nullptr);
    EXPECT_EQ(wid.checkBox->text(), title);
}

TEST(CheckBoxWidget, checked)
{
    CheckBoxWidget wid("");
    EXPECT_EQ(wid.checked(), wid.checkBox->checkState() == Qt::Checked);
}

TEST(CheckBoxWidget, setChecked)
{
    CheckBoxWidget wid("");
    bool sig = false;
    wid.connect(&wid, &CheckBoxWidget::checked, [&sig](){
        sig = true;
    });

    bool check = false;
    wid.connect(wid.checkBox, &QCheckBox::stateChanged, [&check](){
        check = true;
    });

    bool old = wid.checked();
    wid.setChecked(!old);

    EXPECT_TRUE(check);
    EXPECT_FALSE(sig);
    EXPECT_EQ(wid.checked(), !old);
}
