// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <QTest>
#include <QSignalSpy>
#include <QUrl>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>
#include <QList>
#include <QProcess>
#include <QStandardPaths>
#include <QMenu>

#include "stubext.h"

#include "avfsbrowser.h"
#include "utils/avfsutils.h"

#include <dfm-base/base/urlroute.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/interfaces/abstractdiriterator.h>
#include <dfm-base/interfaces/abstractfilewatcher.h>
#include <dfm-base/interfaces/abstractmenuscene.h>
#include <dfm-base/base/device/deviceutils.h>
#include <dfm-base/mimetype/mimetypedisplaymanager.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/clipboard.h>

#include <dfm-io/dfileinfo.h>

#include <dfm-framework/event/event.h>
#include <dfm-framework/dpf.h>

using namespace dfmplugin_avfsbrowser;
DFMBASE_USE_NAMESPACE
using namespace dfmbase;

class TestAvfsBrowser : public testing::Test
{
protected:
    void SetUp() override
    {
        stub.clear();
    }

    void TearDown() override
    {
        stub.clear();
    }

    stub_ext::StubExt stub;
};

// 测试AvfsBrowser类
TEST_F(TestAvfsBrowser, Initialize)
{
    AvfsBrowser browser;
    EXPECT_NO_FATAL_FAILURE(browser.initialize());
}

TEST_F(TestAvfsBrowser, Start)
{
    AvfsBrowser browser;
    bool result = browser.start();
    EXPECT_TRUE(result);
}

TEST_F(TestAvfsBrowser, FollowEvents)
{
    AvfsBrowser browser;
    EXPECT_NO_FATAL_FAILURE(browser.followEvents());
}

TEST_F(TestAvfsBrowser, RegCrumb)
{
    AvfsBrowser browser;
    EXPECT_NO_FATAL_FAILURE(browser.regCrumb());
}

TEST_F(TestAvfsBrowser, BeMySubScene)
{
    AvfsBrowser browser;
    EXPECT_NO_FATAL_FAILURE(browser.beMySubScene("testScene"));
}

TEST_F(TestAvfsBrowser, BeMySubOnAdded)
{
    AvfsBrowser browser;
    EXPECT_NO_FATAL_FAILURE(browser.beMySubOnAdded("newScene"));
}