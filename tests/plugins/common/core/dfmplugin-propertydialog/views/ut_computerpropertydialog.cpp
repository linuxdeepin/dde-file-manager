// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugins/common/core/dfmplugin-propertydialog/views/computerpropertydialog.h"

#include <gtest/gtest.h>

DPPROPERTYDIALOG_USE_NAMESPACE

class UT_ComputerPropertyDialog : public testing::Test
{
protected:
    void SetUp() override {}
    void TearDown() override {}
};

TEST_F(UT_ComputerPropertyDialog, computerProcess)
{
    ComputerPropertyDialog dialog;
    QMap<ComputerInfoItem, QString> computerInfo;
    computerInfo.insert(ComputerInfoItem::kName, "uos");
    dialog.computerProcess(computerInfo);
    EXPECT_TRUE(dialog.computerName->RightValue() == "uos");
}
