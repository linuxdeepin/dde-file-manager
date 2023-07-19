// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "files/recentiterateworker.h"
#include "utils/recentmanager.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/file/local/private/syncfileinfo_p.h>
#include <dfm-base/interfaces/fileinfo.h>
#include <gtest/gtest.h>

#include <QPaintEvent>
#include <QPainter>

DFMBASE_USE_NAMESPACE
using namespace dfmplugin_recent;

class RecentIterateWorkerTest : public testing::Test
{
protected:
    virtual void SetUp() override
    {
    }
    virtual void TearDown() override
    {
        stub.clear();
    }

private:
    stub_ext::StubExt stub;
};

TEST_F(RecentIterateWorkerTest, onRecentFileChanged)
{
    stub.set_lamda(&RecentManager::init, []() {});
    stub.set_lamda(&SyncFileInfoPrivate::init, [] {});
    int i = 1;
    stub.set_lamda(&QXmlStreamReader::atEnd, [&i]() -> bool {
        return i-- <= 0;
    });
    stub.set_lamda(&QXmlStreamReader::readNextStartElement, []() -> bool {
        return true;
    });

    stub.set_lamda(&QString::isEmpty, []() -> bool { return false; });
    QString str("bookmark");
    stub.set_lamda(&QXmlStreamReader::name, [str]() -> QStringRef {
        return QStringRef(&str);
    });
    stub.set_lamda(&QXmlStreamReader::isStartElement, []() -> bool { return true; });
    stub.set_lamda(&QXmlStreamReader::hasError, []() -> bool { return false; });
    stub.set_lamda(&InfoFactory::create<FileInfo>, [] {
        return QSharedPointer<SyncFileInfo>(new SyncFileInfo(QUrl::fromLocalFile("/home")));
    });
    stub.set_lamda(VADDR(SyncFileInfo, isAttributes), [](SyncFileInfo *, const SyncFileInfo::FileIsType type) {
        return true;
    });
    stub.set_lamda(&FileUtils::bindPathTransform, []() -> QString { return "hello/world"; });
    stub.set_lamda(&RecentHelper::xbelPath, []() -> QString { return "123"; });
    stub.set_lamda((bool (*)(QIODevice::OpenMode))((bool (QFile::*)(QIODevice::OpenMode)) & QFile::open), []() {
        return true;
    });

    RecentIterateWorker worker;
    int flag = 0;
    QObject::connect(&worker, &RecentIterateWorker::updateRecentFileInfo, [&flag](const QUrl &url, const QString originPath, qint64 readTime) {
        EXPECT_FALSE(url.isEmpty());
        flag++;
    });
    QObject::connect(&worker, &RecentIterateWorker::deleteExistRecentUrls, [&flag](const QList<QUrl> &urls) {
        EXPECT_FALSE(urls.isEmpty());
        flag++;
    });
    EXPECT_NO_FATAL_FAILURE(worker.onRecentFileChanged({ QUrl("hello/uos") }));

    EXPECT_EQ(flag, 2);
}
