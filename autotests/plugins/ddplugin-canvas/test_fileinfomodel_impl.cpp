// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"
#include "canvas_test_common.h"

#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"
#include "plugins/desktop/ddplugin-canvas/model/fileprovider.h"
#include "plugins/desktop/ddplugin-canvas/model/filefilter.h"
#include "plugins/desktop/ddplugin-canvas/utils/fileutil.h"

#include <dfm-base/base/application/application.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-base/file/local/syncfileinfo.h>

#include <QFile>
#include <QMimeData>
#include <QModelIndex>
#include <QTemporaryDir>

DFMGLOBAL_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

class FileInfoModelImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        stub.set_lamda(ADDR(Application, instance), []() -> Application * {
            return canvas_test::sharedApp();   // single shared instance
        });
        stub.set_lamda(ADDR(Application, appAttribute),
                       [](Application::ApplicationAttribute) -> QVariant { return QVariant(); });

        // Capture the root URL and avoid starting real file-system threads.
        using SetRootFunc = bool (FileProvider::*)(const QUrl &);
        stub.set_lamda(static_cast<SetRootFunc>(&FileProvider::setRoot),
                       [this](FileProvider *, const QUrl &url) -> bool {
                           capturedRoot = url;
                           return true;
                       });

        using RootFunc = QUrl (FileProvider::*)() const;
        stub.set_lamda(static_cast<RootFunc>(&FileProvider::root),
                       [this](FileProvider *) -> QUrl { return capturedRoot; });

        using RefreshFunc = void (FileProvider::*)(QDir::Filters);
        stub.set_lamda(static_cast<RefreshFunc>(&FileProvider::refresh),
                       [](FileProvider *, QDir::Filters) {});

        // Provide local file info for the root; scheme registration is not
        // available in the unit test environment.
        stub.set_lamda(ADDR(DesktopFileCreator, createFileInfo),
                       [this](DesktopFileCreator *, const QUrl &url,
                              dfmbase::Global::CreateFileInfoType) -> FileInfoPointer {
                           return FileInfoPointer(new SyncFileInfo(url));
                       });

        model = new FileInfoModel();
    }

    void TearDown() override
    {
        delete model;
        stub.clear();
    }

    QUrl rootUrl() const { return QUrl::fromLocalFile(tempDir->path()); }

    QUrl makeFile(const QString &name)
    {
        QFile f(tempDir->filePath(name));
        f.open(QIODevice::WriteOnly);
        f.close();
        return QUrl::fromLocalFile(tempDir->filePath(name));
    }

    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    FileInfoModel *model = nullptr;
    QUrl capturedRoot;
};

TEST_F(FileInfoModelImpl, constructAndRoot)
{
    EXPECT_NE(model, nullptr);

    QModelIndex rootIdx = model->setRootUrl(rootUrl());
    EXPECT_TRUE(rootIdx.isValid());
    EXPECT_EQ(rootIdx, model->rootIndex());
    EXPECT_EQ(model->rootUrl(), rootUrl());

    // Refresh was requested but stubbed; model should be in refreshing state.
    EXPECT_EQ(model->modelState(), 2);
}

TEST_F(FileInfoModelImpl, emptyIndexes)
{
    model->setRootUrl(rootUrl());

    EXPECT_FALSE(model->index(-1, 0).isValid());
    EXPECT_FALSE(model->index(0, 0).isValid());
    EXPECT_FALSE(model->index(QUrl("file:///no-such"), 0).isValid());

    EXPECT_EQ(model->index(rootUrl(), 0), model->rootIndex());

    EXPECT_EQ(model->parent(model->rootIndex()), QModelIndex());
    // index(0,0) is invalid in an empty model, and parent() only maps valid
    // children back to the root index.
    EXPECT_EQ(model->parent(model->index(0, 0)), QModelIndex());

    EXPECT_EQ(model->rowCount(model->rootIndex()), 0);
    EXPECT_EQ(model->columnCount(model->rootIndex()), 1);
    EXPECT_EQ(model->rowCount(QModelIndex()), 0);
    EXPECT_EQ(model->columnCount(QModelIndex()), 0);
}

TEST_F(FileInfoModelImpl, fileInfoAndUrl)
{
    model->setRootUrl(rootUrl());

    FileInfoPointer rootInfo = model->fileInfo(model->rootIndex());
    EXPECT_NE(rootInfo, nullptr);
    EXPECT_EQ(model->fileUrl(model->rootIndex()), rootUrl());

    QModelIndex invalid;
    EXPECT_EQ(model->fileInfo(invalid), FileInfoPointer());
    EXPECT_TRUE(model->fileUrl(invalid).isEmpty());

    EXPECT_TRUE(model->files().isEmpty());
}

TEST_F(FileInfoModelImpl, dataAndFlags)
{
    model->setRootUrl(rootUrl());

    EXPECT_FALSE(model->data(model->rootIndex(), Qt::DisplayRole).isValid());
    EXPECT_FALSE(model->data(QModelIndex(), Qt::DisplayRole).isValid());

    Qt::ItemFlags flags = model->flags(QModelIndex());
    // In Qt 6, QAbstractItemModel::flags() reports no flags for an invalid index.
    EXPECT_FALSE(flags & Qt::ItemIsEnabled);
    EXPECT_FALSE(flags & Qt::ItemIsDropEnabled);
}

TEST_F(FileInfoModelImpl, mimeAndDrop)
{
    model->setRootUrl(rootUrl());

    QStringList types = model->mimeTypes();
    EXPECT_FALSE(types.isEmpty());

    QMimeData *mime = model->mimeData(QModelIndexList());
    ASSERT_NE(mime, nullptr);
    EXPECT_TRUE(mime->urls().isEmpty());
    delete mime;

    QMimeData emptyMime;
    EXPECT_FALSE(model->dropMimeData(&emptyMime, Qt::CopyAction, 0, 0, QModelIndex()));

    EXPECT_NE(model->supportedDragActions(), Qt::IgnoreAction);
    EXPECT_NE(model->supportedDropActions(), Qt::IgnoreAction);
}

TEST_F(FileInfoModelImpl, refreshAndUpdate)
{
    model->setRootUrl(rootUrl());

    EXPECT_NO_THROW(model->refresh(model->rootIndex()));
    EXPECT_EQ(model->modelState(), 2);

    EXPECT_NO_THROW(model->update());
    EXPECT_NO_THROW(model->refreshAllFile());

    // Non-root parent should return early.
    QModelIndex child = model->index(0, 0);
    EXPECT_NO_THROW(model->refresh(child));

    EXPECT_NO_THROW(model->updateFile(QUrl("file:///nonexistent")));
}

TEST_F(FileInfoModelImpl, installAndRemoveFilter)
{
    QSharedPointer<FileFilter> filter(new FileFilter());
    EXPECT_NO_THROW(model->installFilter(filter));
    EXPECT_NO_THROW(model->removeFilter(filter));
}
