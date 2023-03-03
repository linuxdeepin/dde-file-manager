// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filedialog/filedialogplugin-core/views/filedialog.h"

#include <gtest/gtest.h>

DIALOGCORE_USE_NAMESPACE

class UT_FileDialog : public testing::Test
{
protected:
    void SetUp() override { }
    void TearDown() override { stub.clear(); }

private:
    stub_ext::StubExt stub;
};

TEST_F(UT_FileDialog, onViewItemClicked)
{
    FileDialog dialog(QUrl("~"));
    QVariantMap data;
    data.insert("url", "~");
    data.insert("displayName", "test");
    dialog.onViewItemClicked(data);
    EXPECT_TRUE(data["displayName"] == "test");
}
