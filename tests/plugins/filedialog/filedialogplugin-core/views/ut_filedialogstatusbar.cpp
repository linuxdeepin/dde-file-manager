/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng <gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge <zhengyouge@uniontech.com>
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

#include "stubext.h"
#include "plugins/filedialog/filedialogplugin-core/views/filedialogstatusbar.h"

#include <gtest/gtest.h>

#include <QLineEdit>

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
    bar.fileNameEdit->setText("test1.txt");
    bar.changeFileNameEditText("test2");
    QString ss = bar.fileNameEdit->text();
    EXPECT_TRUE(bar.fileNameEdit->text() == "test2.txt");
}
