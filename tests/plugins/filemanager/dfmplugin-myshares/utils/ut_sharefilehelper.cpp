/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     xushitong<xushitong@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             zhangsheng<zhangsheng@uniontech.com>
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

#include "plugins/filemanager/dfmplugin-myshares/utils/shareutils.h"
#include "plugins/filemanager/dfmplugin-myshares/utils/sharefilehelper.h"
#include "plugins/filemanager/dfmplugin-myshares/events/shareeventscaller.h"

#include "stubext.h"

#include <gtest/gtest.h>

#include <QDebug>
#include <QUrl>

using namespace dfmplugin_myshares;

class UT_ShareFileHelper : public testing::Test
{
    // Test interface
protected:
    virtual void SetUp() override {}
    virtual void TearDown() override {}
};

TEST_F(UT_ShareFileHelper, OpenFileInPlugin)
{
    QList<QUrl> inputs;
    EXPECT_FALSE(ShareFileHelper::instance()->openFileInPlugin(0, inputs));

    inputs = { QUrl::fromLocalFile("/") };
    EXPECT_FALSE(ShareFileHelper::instance()->openFileInPlugin(0, inputs));
    inputs.clear();

    stub_ext::StubExt stub;
    stub.set_lamda(ShareEventsCaller::sendOpenDirs, [](quint64, const QList<QUrl>, ShareEventsCaller::OpenMode) {});
    inputs = { ShareUtils::makeShareUrl("/hello") };
    EXPECT_TRUE(ShareFileHelper::instance()->openFileInPlugin(0, inputs));
}
