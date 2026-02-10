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

#include "events/avfseventhandler.h"

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

class TestAvfsEventHandler : public testing::Test
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

// 测试AvfsEventHandler类
TEST_F(TestAvfsEventHandler, Instance)
{
    AvfsEventHandler *instance1 = AvfsEventHandler::instance();
    AvfsEventHandler *instance2 = AvfsEventHandler::instance();
    EXPECT_EQ(instance1, instance2);
}

TEST_F(TestAvfsEventHandler, HookOpenFiles)
{
    QList<QUrl> urls { QUrl("file:///test/test.zip") };
    bool result = AvfsEventHandler::instance()->hookOpenFiles(123, urls);
    EXPECT_FALSE(result);  // 默认情况下返回false
}

TEST_F(TestAvfsEventHandler, HookEnterPressed)
{
    QList<QUrl> urls { QUrl("file:///test/test.zip") };
    bool result = AvfsEventHandler::instance()->hookEnterPressed(123, urls);
    EXPECT_FALSE(result);  // 默认情况下返回false
}

TEST_F(TestAvfsEventHandler, SepateTitlebarCrumb)
{
    QUrl testUrl("avfs:///test/path");
    QList<QVariantMap> mapGroup;
    bool result = AvfsEventHandler::instance()->sepateTitlebarCrumb(testUrl, &mapGroup);
    EXPECT_FALSE(result); // 默认情况下返回false
}

TEST_F(TestAvfsEventHandler, OpenArchivesAsDir)
{
    QList<QUrl> urls { QUrl("file:///test/test.zip") };
    EXPECT_NO_FATAL_FAILURE(AvfsEventHandler::instance()->openArchivesAsDir(123, urls));
}

TEST_F(TestAvfsEventHandler, WriteToClipbord)
{
    QList<QUrl> urls { QUrl("file:///test/test.zip") };
    EXPECT_NO_FATAL_FAILURE(AvfsEventHandler::instance()->writeToClipbord(123, urls));
}

TEST_F(TestAvfsEventHandler, ShowProperty)
{
    QList<QUrl> urls { QUrl("file:///test/test.zip") };
    EXPECT_NO_FATAL_FAILURE(AvfsEventHandler::instance()->showProperty(urls));
}