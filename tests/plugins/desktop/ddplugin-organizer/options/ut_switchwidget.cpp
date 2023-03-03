// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
