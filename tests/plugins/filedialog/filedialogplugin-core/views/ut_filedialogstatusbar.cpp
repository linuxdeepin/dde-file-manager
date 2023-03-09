// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filedialog/filedialogplugin-core/views/filedialogstatusbar.h"

#include <gtest/gtest.h>

#include <DLineEdit>

DIALOGCORE_USE_NAMESPACE

class UT_FileDialogStatusBar : public testing::Test
{
protected:
    void SetUp() override { }
    void TearDown() override { }
};

TEST_F(UT_FileDialogStatusBar, onViewItemClicked)
{
    FileDialogStatusBar bar;
    bar.fileNameEdit->lineEdit()->setText("test1.txt");
    bar.changeFileNameEditText("test2");
    QString ss = bar.fileNameEdit->lineEdit()->text();
    EXPECT_TRUE(bar.fileNameEdit->lineEdit()->text() == "test2.txt");
}
