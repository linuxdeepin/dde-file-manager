// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include "stubext.h"
#include "canvas_test_common.h"

#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel.h"
#include "plugins/desktop/ddplugin-canvas/model/canvasproxymodel_p.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"
#include "plugins/desktop/ddplugin-canvas/model/modelhookinterface.h"
#include "plugins/desktop/ddplugin-canvas/utils/fileutil.h"
#include "plugins/desktop/ddplugin-canvas/view/operator/fileoperatorproxy.h"

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

namespace {

class HookImpl : public ModelHookInterface
{
public:
    bool modelData(const QUrl &, int role, QVariant *out, void *) const override
    {
        if (role == hookRole && out) {
            *out = hookValue;
            return true;
        }
        return false;
    }

    bool mimeTypes(QStringList *types, void *) const override
    {
        if (types) {
            types->append("x-hook/test");
            return true;
        }
        return false;
    }

    int hookRole = Qt::UserRole + 100;
    QVariant hookValue;
};

} // namespace

// Exposes begin/endResetModel so tests can deterministically emit modelReset;
// CanvasProxyModel builds its mapping only in response to that signal.
class ResettableFileInfoModel : public FileInfoModel
{
public:
    using FileInfoModel::FileInfoModel;

    void triggerReset()
    {
        beginResetModel();
        endResetModel();
    }
};

class CanvasProxyModelImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        tempDir.reset(new QTemporaryDir());
        ASSERT_TRUE(tempDir->isValid());

        // Avoid requiring a running DDE application.
        stub.set_lamda(ADDR(Application, instance), []() -> Application * {
            return canvas_test::sharedApp();   // single shared instance
        });
        stub.set_lamda(ADDR(Application, appAttribute),
                       [](Application::ApplicationAttribute) -> QVariant { return QVariant(true); });

        // Local file info for filters and drop targets.
        stub.set_lamda(ADDR(DesktopFileCreator, createFileInfo),
                       [this](DesktopFileCreator *, const QUrl &url,
                              dfmbase::Global::CreateFileInfoType) -> FileInfoPointer {
                           return FileInfoPointer(new SyncFileInfo(url));
                       });

        // Keep drop handling hermetic.
        stub.set_lamda(ADDR(FileOperatorProxy, dropFiles),
                       [](FileOperatorProxy *, const Qt::DropAction &, const QUrl &, const QList<QUrl> &) {});
        stub.set_lamda(ADDR(FileOperatorProxy, dropToTrash),
                       [](FileOperatorProxy *, const QList<QUrl> &) {});
        stub.set_lamda(ADDR(FileOperatorProxy, dropToApp),
                       [](FileOperatorProxy *, const QList<QUrl> &, const QString &) {});

        // Source model callbacks are controlled by each test.
        using FilesFunc = QList<QUrl> (FileInfoModel::*)() const;
        stub.set_lamda(static_cast<FilesFunc>(&FileInfoModel::files),
                       [this](FileInfoModel *) -> QList<QUrl> { return currentFiles; });

        using IndexUrlFunc = QModelIndex (FileInfoModel::*)(const QUrl &, int) const;
        stub.set_lamda(static_cast<IndexUrlFunc>(&FileInfoModel::index),
                       [this](FileInfoModel *, const QUrl &url, int column) -> QModelIndex {
                           int row = currentFiles.indexOf(url);
                           if (row < 0)
                               return QModelIndex();
                           return srcModel->QAbstractItemModel::createIndex(row, column);
                       });

        using FileInfoFunc = FileInfoPointer (FileInfoModel::*)(const QModelIndex &) const;
        stub.set_lamda(static_cast<FileInfoFunc>(&FileInfoModel::fileInfo),
                       [this](FileInfoModel *, const QModelIndex &idx) -> FileInfoPointer {
                           if (!idx.isValid() || idx.row() < 0 || idx.row() >= currentFiles.count())
                               return FileInfoPointer();
                           return FileInfoPointer(new SyncFileInfo(currentFiles.at(idx.row())));
                       });

        using FileUrlFunc = QUrl (FileInfoModel::*)(const QModelIndex &) const;
        stub.set_lamda(static_cast<FileUrlFunc>(&FileInfoModel::fileUrl),
                       [this](FileInfoModel *, const QModelIndex &idx) -> QUrl {
                           if (!idx.isValid() || idx.row() < 0 || idx.row() >= currentFiles.count())
                               return QUrl();
                           return currentFiles.at(idx.row());
                       });

        // data() is virtual: use VADDR to resolve the real function address,
        // otherwise cpp-stub patches a vtable offset and crashes.
        stub.set_lamda(VADDR(FileInfoModel, data),
                       [this](FileInfoModel *, const QModelIndex &idx, int role) -> QVariant {
                           if (!idx.isValid() || idx.row() < 0 || idx.row() >= currentFiles.count())
                               return QVariant();
                           const QUrl &url = currentFiles.at(idx.row());
                           if (role == DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole)
                               return url.fileName();
                           if (role == DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileMimeTypeRole)
                               return QStringLiteral("text/plain");
                           if (role == DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileSizeRole)
                               return qint64(1024);
                           if (role == Qt::DisplayRole)
                               return url.fileName();
                           return QVariant();
                       });

        using RootUrlFunc = QUrl (FileInfoModel::*)() const;
        stub.set_lamda(static_cast<RootUrlFunc>(&FileInfoModel::rootUrl),
                       [this](FileInfoModel *) -> QUrl { return QUrl::fromLocalFile(tempDir->path()); });

        using RefreshFunc = void (FileInfoModel::*)(const QModelIndex &);
        stub.set_lamda(static_cast<RefreshFunc>(&FileInfoModel::refresh),
                       [](FileInfoModel *, const QModelIndex &) {});

        using UpdateFunc = void (FileInfoModel::*)();
        stub.set_lamda(static_cast<UpdateFunc>(&FileInfoModel::update), [](FileInfoModel *) {});

        srcModel = new ResettableFileInfoModel();
        proxy = new CanvasProxyModel();
    }

    void TearDown() override
    {
        delete proxy;
        delete srcModel;
        stub.clear();
    }

    QUrl makeUrl(const QString &name)
    {
        const QString path = tempDir->filePath(name);
        QFile f(path);
        if (!QFile::exists(path)) {
            f.open(QIODevice::WriteOnly);
            f.close();
        }
        return QUrl::fromLocalFile(path);
    }

    void populateSource(const QStringList &names)
    {
        currentFiles.clear();
        for (const QString &n : names)
            currentFiles << makeUrl(n);
    }

    // Populate the stubbed file list, wire the source model and emit modelReset
    // so CanvasProxyModel::createMapping() actually picks the files up.
    void loadSource(const QStringList &names)
    {
        populateSource(names);
        proxy->setSourceModel(srcModel);
        srcModel->triggerReset();
    }

    stub_ext::StubExt stub;
    QScopedPointer<QTemporaryDir> tempDir;
    ResettableFileInfoModel *srcModel = nullptr;
    CanvasProxyModel *proxy = nullptr;
    QList<QUrl> currentFiles;
};

TEST_F(CanvasProxyModelImpl, constructAndBasicAccessors)
{
    EXPECT_NE(proxy, nullptr);
    EXPECT_EQ(proxy->QObject::parent(), nullptr);

    EXPECT_TRUE(proxy->rootIndex().isValid());
    EXPECT_EQ(proxy->rootIndex().row(), INT_MAX);

    proxy->setSourceModel(srcModel);
    EXPECT_EQ(proxy->sourceModel(), srcModel);

    EXPECT_FALSE(proxy->showHiddenFiles());
    proxy->setShowHiddenFiles(true);
    EXPECT_TRUE(proxy->showHiddenFiles());
    proxy->setShowHiddenFiles(false);
    EXPECT_FALSE(proxy->showHiddenFiles());

    proxy->setSortOrder(Qt::DescendingOrder);
    EXPECT_EQ(proxy->sortOrder(), Qt::DescendingOrder);

    proxy->setSortRole(DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder);
    EXPECT_EQ(proxy->sortRole(), DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole);
    EXPECT_EQ(proxy->sortOrder(), Qt::AscendingOrder);

    EXPECT_EQ(proxy->modelHook(), nullptr);
}

TEST_F(CanvasProxyModelImpl, mappingWithSourceModel)
{
    loadSource({ "b.txt", "a.txt" });

    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 2);
    EXPECT_EQ(proxy->columnCount(proxy->rootIndex()), 1);
    EXPECT_EQ(proxy->columnCount(QModelIndex()), 0);

    QList<QUrl> files = proxy->files();
    EXPECT_EQ(files.size(), 2);

    for (const QUrl &url : currentFiles) {
        QModelIndex idx = proxy->index(url, 0);
        EXPECT_TRUE(idx.isValid());
        EXPECT_EQ(proxy->fileUrl(idx), url);

        QModelIndex src = proxy->mapToSource(idx);
        EXPECT_TRUE(src.isValid());
        EXPECT_EQ(proxy->mapFromSource(src), idx);

        EXPECT_EQ(proxy->data(idx, DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole).toString(), url.fileName());
    }

    QModelIndex invalid = proxy->index(QUrl("file:///no-such-file"), 0);
    EXPECT_FALSE(invalid.isValid());
    EXPECT_FALSE(proxy->mapToSource(QModelIndex()).isValid());
}

TEST_F(CanvasProxyModelImpl, indexAndParent)
{
    loadSource({ "item.txt" });

    QModelIndex root = proxy->rootIndex();
    QModelIndex child = proxy->index(0, 0);
    EXPECT_TRUE(child.isValid());
    EXPECT_EQ(proxy->parent(child), root);
    EXPECT_EQ(proxy->rowCount(child), 0);

    EXPECT_FALSE(proxy->index(-1, 0).isValid());
    EXPECT_FALSE(proxy->index(100, 0).isValid());
    EXPECT_FALSE(proxy->parent(root).isValid());
}

TEST_F(CanvasProxyModelImpl, sortReordersFiles)
{
    loadSource({ "b.txt", "a.txt" });
    proxy->setSortRole(DFMGLOBAL_NAMESPACE::ItemRoles::kItemFileDisplayNameRole, Qt::AscendingOrder);

    EXPECT_TRUE(proxy->sort());
    QList<QUrl> files = proxy->files();
    EXPECT_EQ(files.size(), 2);
    EXPECT_EQ(files.first().fileName(), QString("a.txt"));
    EXPECT_EQ(files.last().fileName(), QString("b.txt"));
}

TEST_F(CanvasProxyModelImpl, fetchAndTake)
{
    loadSource({ "existing.txt" });

    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 1);

    QUrl extra = makeUrl("extra.txt");
    // Let the stubbed source index()/fileInfo() report the new file too.
    currentFiles.append(extra);
    EXPECT_TRUE(proxy->fetch(extra));
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 2);
    EXPECT_TRUE(proxy->index(extra, 0).isValid());

    // Fetching an already-present URL is a no-op success.
    EXPECT_TRUE(proxy->fetch(extra));
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 2);

    EXPECT_TRUE(proxy->take(extra));
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 1);
    EXPECT_FALSE(proxy->index(extra, 0).isValid());

    // Taking a missing URL is also reported as success.
    EXPECT_TRUE(proxy->take(extra));
}

TEST_F(CanvasProxyModelImpl, refresh)
{
    loadSource({ "a.txt" });

    proxy->refresh(proxy->rootIndex(), false, 0, false);
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 1);

    // Timer-based refresh path.
    proxy->refresh(proxy->rootIndex(), false, 10, true);
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 1);

    // Non-root parent is ignored.
    QModelIndex child = proxy->index(0, 0);
    proxy->refresh(child, false, 0, false);
    EXPECT_EQ(proxy->rowCount(proxy->rootIndex()), 1);
}

TEST_F(CanvasProxyModelImpl, mimeDataAndMimeTypes)
{
    loadSource({ "drag.txt" });

    QStringList types = proxy->mimeTypes();
    EXPECT_FALSE(types.isEmpty());

    QModelIndexList indexes;
    indexes << proxy->index(0, 0);
    QMimeData *mime = proxy->mimeData(indexes);
    ASSERT_NE(mime, nullptr);
    EXPECT_FALSE(mime->urls().isEmpty());
    EXPECT_EQ(mime->text(), QString("dde-desktop"));
    delete mime;
}

TEST_F(CanvasProxyModelImpl, dropMimeData)
{
    proxy->setSourceModel(srcModel);

    QMimeData mime;
    mime.setUrls({ makeUrl("source.txt") });

    bool ok = proxy->dropMimeData(&mime, Qt::CopyAction, 0, 0, proxy->rootIndex());
    EXPECT_TRUE(ok);
}

TEST_F(CanvasProxyModelImpl, modelHook)
{
    loadSource({ "hook.txt" });

    HookImpl hook;
    hook.hookValue = QStringLiteral("hooked");
    proxy->setModelHook(&hook);
    EXPECT_EQ(proxy->modelHook(), &hook);

    QModelIndex idx = proxy->index(0, 0);
    QVariant value = proxy->data(idx, hook.hookRole);
    EXPECT_EQ(value.toString(), QString("hooked"));

    QStringList types = proxy->mimeTypes();
    EXPECT_TRUE(types.contains("x-hook/test"));
}
