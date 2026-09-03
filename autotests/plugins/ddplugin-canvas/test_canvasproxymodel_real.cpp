// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"

// expose private members for source model population
#define private public
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel_p.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel_p.h"
#undef private

#include <dfm-base/base/application/application.h>
#include "canvas_test_common.h"
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/dfm_global_defines.h>

#include <QTemporaryDir>
#include <QFile>
#include <QMimeData>

DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using namespace ddplugin_canvas;

namespace {

QUrl makeUrl(QTemporaryDir *tempDir, const QString &name)
{
    const QString path = tempDir->filePath(name);
    QFile f(path);
    if (!QFile::exists(path)) {
        f.open(QIODevice::WriteOnly);
        f.close();
    }
    return QUrl::fromLocalFile(path);
}

} // namespace

class CanvasProxyModelReal : public testing::Test
{
protected:
    void SetUp() override
    {
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        // Avoid requiring the real DDE application singleton.
        stub.set_lamda(ADDR(Application, instance), []() -> Application * {
            return canvas_test::sharedApp();
        });
        stub.set_lamda(ADDR(Application, appAttribute),
                       [](Application::ApplicationAttribute) -> QVariant { return QVariant(true); });

        // Force local files to be represented by SyncFileInfo in tests.
        stub.set_lamda(
            static_cast<FileInfoPointer (*)(const QUrl &, dfmbase::Global::CreateFileInfoType, QString *)>(&InfoFactory::create<FileInfo>),
            [](const QUrl &url, dfmbase::Global::CreateFileInfoType, QString *err) -> FileInfoPointer {
                if (err)
                    *err = QString();
                FileInfoPointer info(new SyncFileInfo(url));
                if (info)
                    info->refresh();
                return info;
            });

        srcModel = new FileInfoModel();
        srcModel->setRootUrl(QUrl::fromLocalFile(tempDir->path()));

        // Populate the real source model directly via its private resetData path.
        urls.clear();
        urls << makeUrl(tempDir.data(), "a.txt")
             << makeUrl(tempDir.data(), "b.txt")
             << makeUrl(tempDir.data(), "c.txt");

        proxy = new CanvasProxyModel();
        proxy->setSourceModel(srcModel);

        // Populate after connecting the proxy so that the modelReset signal fills the proxy mapping.
        srcModel->d->resetData(urls);
    }

    void TearDown() override
    {
        delete proxy;
        proxy = nullptr;
        delete srcModel;
        srcModel = nullptr;
        stub.clear();
    }

    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    FileInfoModel *srcModel = nullptr;
    CanvasProxyModel *proxy = nullptr;
    QList<QUrl> urls;
};

TEST_F(CanvasProxyModelReal, constructAndSource)
{
    EXPECT_NE(proxy, nullptr);
    EXPECT_EQ(proxy->sourceModel(), srcModel);
    EXPECT_TRUE(proxy->rootIndex().isValid());
    EXPECT_EQ(proxy->rootUrl(), QUrl::fromLocalFile(tempDir->path()));
}

TEST_F(CanvasProxyModelReal, rowCountColumnCountParent)
{
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 3);
    EXPECT_EQ(proxy->columnCount(proxy->rootIndex()), 1);
    EXPECT_EQ(proxy->rowCount(QModelIndex()), 0);
    EXPECT_EQ(proxy->columnCount(QModelIndex()), 0);

    QModelIndex first = proxy->index(0, 0);
    EXPECT_TRUE(first.isValid());
    EXPECT_EQ(proxy->parent(first), proxy->rootIndex());
    EXPECT_FALSE(proxy->parent(proxy->rootIndex()).isValid());
}

TEST_F(CanvasProxyModelReal, indexByUrlAndMapToSource)
{
    for (const QUrl &url : urls) {
        QModelIndex idx = proxy->index(url);
        EXPECT_TRUE(idx.isValid());
        QModelIndex src = proxy->mapToSource(idx);
        EXPECT_TRUE(src.isValid());
        EXPECT_EQ(srcModel->fileUrl(src), url);
        EXPECT_EQ(proxy->mapFromSource(src), idx);
    }

    EXPECT_FALSE(proxy->index(QUrl()).isValid());
    EXPECT_FALSE(proxy->mapToSource(QModelIndex()).isValid());
    EXPECT_FALSE(proxy->mapFromSource(QModelIndex()).isValid());
}

TEST_F(CanvasProxyModelReal, dataAndFileInfo)
{
    QList<QUrl> found = proxy->files();
    EXPECT_EQ(found.size(), 3);

    QModelIndex first = proxy->index(0, 0);
    EXPECT_TRUE(first.isValid());
    QUrl url = proxy->fileUrl(first);
    EXPECT_TRUE(urls.contains(url));

    FileInfoPointer info = proxy->fileInfo(first);
    EXPECT_FALSE(info.isNull());

    QVariant display = proxy->data(first, Qt::DisplayRole);
    EXPECT_TRUE(display.isValid());

    QVariant mime = proxy->data(first, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole);
    EXPECT_TRUE(mime.isValid());
}

TEST_F(CanvasProxyModelReal, sortRoleAndHidden)
{
    proxy->setShowHiddenFiles(true);
    EXPECT_TRUE(proxy->showHiddenFiles());
    proxy->setShowHiddenFiles(false);
    EXPECT_FALSE(proxy->showHiddenFiles());

    proxy->setSortRole(DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder);
    EXPECT_EQ(proxy->sortRole(), DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole);
    EXPECT_EQ(proxy->sortOrder(), Qt::AscendingOrder);

    EXPECT_TRUE(proxy->sort());
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 3);
}

TEST_F(CanvasProxyModelReal, fetchAndTake)
{
    QUrl extra = makeUrl(tempDir.data(), "extra.txt");

    // Put the extra file into the source model without notifying the proxy,
    // so that fetch has real work to do.
    srcModel->d->fileList.append(extra);
    srcModel->d->fileMap.insert(extra, FileInfoPointer(new SyncFileInfo(extra)));

    EXPECT_TRUE(proxy->fetch(extra));
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 4);
    EXPECT_TRUE(proxy->files().contains(extra));

    EXPECT_TRUE(proxy->take(extra));
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 3);
    EXPECT_FALSE(proxy->files().contains(extra));
}

TEST_F(CanvasProxyModelReal, mimeTypes)
{
    QStringList types = proxy->mimeTypes();
    EXPECT_FALSE(types.isEmpty());
}
