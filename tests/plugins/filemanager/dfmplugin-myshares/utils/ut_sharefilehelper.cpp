// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

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
