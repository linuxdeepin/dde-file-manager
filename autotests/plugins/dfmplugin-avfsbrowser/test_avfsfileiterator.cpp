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

#include "files/avfsfileiterator.h"

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

class TestAvfsFileIterator : public testing::Test
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

// 测试AvfsFileIterator类
TEST_F(TestAvfsFileIterator, Constructor)
{
    QUrl testUrl("avfs:///test/path");
    EXPECT_NO_FATAL_FAILURE(AvfsFileIterator iterator(testUrl));
}

TEST_F(TestAvfsFileIterator, Url)
{
    QUrl testUrl("avfs:///test/path");
    AvfsFileIterator iterator(testUrl);
    QUrl result = iterator.url();
    EXPECT_EQ(result, testUrl);
}

TEST_F(TestAvfsFileIterator, Next)
{
    QUrl testUrl("avfs:///test/path");
    AvfsFileIterator iterator(testUrl);
    EXPECT_NO_FATAL_FAILURE(iterator.next());
}

TEST_F(TestAvfsFileIterator, HasNext)
{
    QUrl testUrl("avfs:///test/path");
    AvfsFileIterator iterator(testUrl);
    EXPECT_NO_FATAL_FAILURE(iterator.hasNext());
}

TEST_F(TestAvfsFileIterator, FileName)
{
    QUrl testUrl("avfs:///test/path");
    AvfsFileIterator iterator(testUrl);
    EXPECT_NO_FATAL_FAILURE(iterator.fileName());
}

TEST_F(TestAvfsFileIterator, FileUrl)
{
    QUrl testUrl("avfs:///test/path");
    AvfsFileIterator iterator(testUrl);
    EXPECT_NO_FATAL_FAILURE(iterator.fileUrl());
}

TEST_F(TestAvfsFileIterator, FileInfo)
{
    QUrl testUrl("avfs:///test/path");
    AvfsFileIterator iterator(testUrl);
    EXPECT_NO_FATAL_FAILURE(iterator.fileInfo());
}