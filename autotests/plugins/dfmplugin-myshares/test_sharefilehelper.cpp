// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <stubext.h>

#include "utils/sharefilehelper.h"
#include "utils/shareutils.h"
#include "events/shareeventscaller.h"
#include "dfmplugin_myshares_global.h"

#include <QUrl>
#include <QList>

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
