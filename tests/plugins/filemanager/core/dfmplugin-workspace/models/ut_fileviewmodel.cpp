// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/filemanager/core/dfmplugin-workspace/views/fileview.h"
#include "plugins/filemanager/core/dfmplugin-workspace/models/fileviewmodel.h"
#include "plugins/filemanager/core/dfmplugin-workspace/utils/workspacehelper.h"
#include "plugins/filemanager/core/dfmplugin-workspace/utils/filesortworker.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/asyncfileinfo.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/file/local/localdiriterator.h>
#include <dfm-base/file/local/localfilewatcher.h>

#include <gtest/gtest.h>

#include <QStandardPaths>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
DPWORKSPACE_USE_NAMESPACE

class UT_FileViewModel : public testing::Test
{
protected:
    void SetUp() override
    {
        UrlRoute::regScheme(Global::Scheme::kFile, "/", QIcon(), false, QObject::tr("System Disk"));
        UrlRoute::regScheme(Global::Scheme::kAsyncFile, "/", QIcon(), false,
                            QObject::tr("System Disk"));

        InfoFactory::regClass<dfmbase::SyncFileInfo>(Global::Scheme::kFile);
        DirIteratorFactory::regClass<LocalDirIterator>(Global::Scheme::kFile);
        WatcherFactory::regClass<LocalFileWatcher>(Global::Scheme::kFile);

        QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
        url.setScheme(Scheme::kFile);
        view = new FileView(url);
        model = new FileViewModel(view);
    }

    void TearDown() override
    {
        stub.clear();

        delete model;
        model = nullptr;

        delete view;
        view = nullptr;
    }

private:
    stub_ext::StubExt stub;

    FileView* view;
    FileViewModel* model;
};

TEST_F(UT_FileViewModel, SetRootUrl) {
    auto index = model->setRootUrl(QUrl());
    EXPECT_FALSE(index.isValid());

    stub.set_lamda(ADDR(FileViewModel, closeCursorTimer),[]{});

    bool beginReset { false };
    bool endReset { false };
    stub.set_lamda(ADDR(FileViewModel, beginResetModel), [&beginReset]{ beginReset = true; });
    stub.set_lamda(ADDR(FileViewModel, endResetModel), [&endReset]{ endReset = true; });

    bool initWork { false };
    stub.set_lamda(ADDR(FileViewModel, initFilterSortWork), [&initWork]{ initWork = true; });

    QModelIndex rootIndex = QModelIndex(1, 1, nullptr, model);
    stub.set_lamda(ADDR(FileViewModel, rootIndex), [&rootIndex]{ return rootIndex;});

    QModelIndex fetchIndex = QModelIndex();
    stub.set_lamda(VADDR(FileViewModel, fetchMore), [&fetchIndex](FileViewModel*, const QModelIndex &index) {
        fetchIndex = index;
    });

    bool havePrehandler = false;
    stub.set_lamda(ADDR(WorkspaceHelper, haveViewRoutePrehandler), [&havePrehandler]{ return havePrehandler; });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);

    QModelIndex resultIndex = model->setRootUrl(url);
    EXPECT_TRUE(beginReset);
    EXPECT_TRUE(endReset);
    EXPECT_TRUE(initWork);
    EXPECT_EQ(model->dirRootUrl, url);
    EXPECT_TRUE(model->canFetchFiles);
    EXPECT_EQ(fetchIndex, rootIndex);
    EXPECT_EQ(resultIndex, rootIndex);
}

TEST_F(UT_FileViewModel, InitFilterSortWork) {
    bool destroyOld = false;
    stub.set_lamda(ADDR(FileViewModel, discardFilterSortObjects), [&destroyOld]{ destroyOld = true; });

    QUrl url(QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first());
    url.setScheme(Scheme::kFile);
    QString key("key");

    model->dirRootUrl = url;
    model->currentKey = key;

    bool beginInsert = false;
    bool endInsert = false;
    stub.set_lamda(ADDR(FileViewModel, beginInsertRows), [&beginInsert]{ beginInsert = true; });
    stub.set_lamda(ADDR(FileViewModel, endInsertRows), [&endInsert]{ endInsert = true; });

    bool threadStart = false;
    stub.set_lamda(ADDR(QThread, start), [&threadStart]{ threadStart = true;});

    model->initFilterSortWork();

    EXPECT_TRUE(beginInsert);
    EXPECT_TRUE(endInsert);
    EXPECT_TRUE(threadStart);
    EXPECT_FALSE(model->filterSortWorker.isNull());
    if (!model->filterSortWorker.isNull()) {
        EXPECT_EQ(model->filterSortWorker->current, url);
        EXPECT_EQ(model->filterSortWorker->currentKey, key);
    }
}
