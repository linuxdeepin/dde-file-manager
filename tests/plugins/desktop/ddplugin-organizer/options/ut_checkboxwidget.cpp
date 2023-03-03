// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
