// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
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

#include "files/avfsfileinfo.h"

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

class TestAvfsFileInfo : public testing::Test
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

// 测试AvfsFileInfo类
TEST_F(TestAvfsFileInfo, Constructor)
{
    QUrl testUrl("avfs:///test/path");
    EXPECT_NO_FATAL_FAILURE(AvfsFileInfo fileInfo(testUrl));
}

TEST_F(TestAvfsFileInfo, UrlOf)
{
    QUrl testUrl("avfs:///test/path");
    AvfsFileInfo fileInfo(testUrl);
    QUrl result = fileInfo.urlOf(FileInfo::FileUrlInfoType::kUrl);
    EXPECT_EQ(result, testUrl);
}

TEST_F(TestAvfsFileInfo, CanAttributes)
{
    QUrl testUrl("avfs:///test/path");
    AvfsFileInfo fileInfo(testUrl);
    bool canRedirection = fileInfo.canAttributes(FileInfo::FileCanType::kCanRedirectionFileUrl);
    EXPECT_FALSE(canRedirection);  // 默认情况下返回false
}