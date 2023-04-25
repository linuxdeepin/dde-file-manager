// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/filedialog.h"
#include "views/filedialog_p.h"

#include <gtest/gtest.h>

DIALOGCORE_USE_NAMESPACE

TEST(UT_FileDialogPluginBugTest, bug_163259_SelectNameFilterFail)
{
    FileDialog dialog(QUrl("file:///home"));
    dialog.setNameFilters(QStringList() << "*.ddf");
    int size = dialog.d->nameFilters.size();
    EXPECT_TRUE(size == 1);
    EXPECT_TRUE(dialog.d->nameFilters.contains("*.ddf"));
}
