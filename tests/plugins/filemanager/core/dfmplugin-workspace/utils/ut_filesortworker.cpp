// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-workspace/utils/filesortworker.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <gtest/gtest.h>

#include <QStandardPaths>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

class UT_FileSortWorker : public testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        InfoFactory::regClass<dfmbase::SyncFileInfo>(Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Scheme::kFile);

        url = QUrl(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
        url.setScheme(Scheme::kFile);

        worker = new FileSortWorker(url, key);
    }
    void TearDown() override
    {
        delete worker;
        worker = nullptr;

        stub.clear();
    }

    QUrl url {};
    QString key {"theKey"};
    FileSortWorker *worker = nullptr;
    stub_ext::StubExt stub;
};

TEST_F(UT_FileSortWorker, Bug_199473_handleUpdateFile)
{
    stub.set_lamda(ADDR(FileSortWorker, checkFilters), []{
        return true;
    });

    QUrl updateFile(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first() + "/updateFile");
    updateFile.setScheme(Scheme::kFile);

    QUrl selectAndEditFile {};
    QObject::connect(worker, &FileSortWorker::selectAndEditFile, worker, [&selectAndEditFile](const QUrl &url){
        selectAndEditFile = url;
    });

    worker->childrenUrlList.append(updateFile);
    SortInfoPointer sortInfo(new SortFileInfo());
    sortInfo->setUrl(updateFile);
    sortInfo->setDir(true);
    sortInfo->setReadable(true);
    sortInfo->setWriteable(true);
    sortInfo->setExecutable(true);
    worker->children.append(sortInfo);

    worker->handleUpdateFile(updateFile);

    EXPECT_EQ(selectAndEditFile, updateFile);
}
