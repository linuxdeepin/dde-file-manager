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
