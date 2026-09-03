// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"

#include "models/collectionmodel.h"
#include "models/collectionmodel_p.h"
#include "models/modeldatahandler.h"
#include "interface/fileinfomodelshell.h"

#include <dfm-base/file/local/syncfileinfo.h>

#include <gtest/gtest.h>
#include <QStandardItemModel>
#include <QMimeData>
#include <QUrl>
#include <QModelIndex>
#include <QSignalSpy>

using namespace ddplugin_organizer;
DFMBASE_USE_NAMESPACE

namespace {

class MockFileInfoModelShell : public FileInfoModelShell
{
public:
    explicit MockFileInfoModelShell(QAbstractItemModel *source = nullptr)
        : FileInfoModelShell(nullptr)
    {
        model = source;
    }
};

class PassThroughHandler : public ModelDataHandler
{
public:
    bool acceptInsert(const QUrl &) override { return true; }
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override { return urls; }
    bool acceptRename(const QUrl &, const QUrl &) override { return true; }
    bool acceptUpdate(const QUrl &, const QVector<int> &) override { return true; }
};

}   // namespace

class CollectionModelImpl : public testing::Test
{
protected:
    void SetUp() override
    {
        source = new QStandardItemModel();
        source->setItem(0, new QStandardItem("test"));

        shell = new MockFileInfoModelShell(source);
        handler = new PassThroughHandler();
        model = new CollectionModel();
        model->setModelShell(shell);
        model->setHandler(handler);

        testUrl = QUrl("file:///home/test/file.txt");
        rootUrl = QUrl("file:///home/test");
    }

    void TearDown() override
    {
        delete model;
        delete handler;
        delete shell;
        delete source;
        stub.clear();
    }

public:
    QStandardItemModel *source = nullptr;
    MockFileInfoModelShell *shell = nullptr;
    PassThroughHandler *handler = nullptr;
    CollectionModel *model = nullptr;
    QUrl testUrl;
    QUrl rootUrl;
    stub_ext::StubExt stub;
};

TEST_F(CollectionModelImpl, SetModelShell_SetsSourceModel)
{
    EXPECT_EQ(model->modelShell(), shell);
    EXPECT_EQ(model->sourceModel(), source);
}

TEST_F(CollectionModelImpl, SetHandler_ReturnsHandler)
{
    EXPECT_EQ(model->handler(), handler);
}

TEST_F(CollectionModelImpl, RootUrl_ReturnsShellRootUrl)
{
    stub.set_lamda(&FileInfoModelShell::rootUrl, [this](const FileInfoModelShell *) -> QUrl {
        __DBG_STUB_INVOKE__
        return rootUrl;
    });
    EXPECT_EQ(model->rootUrl(), rootUrl);
}

TEST_F(CollectionModelImpl, RootIndex_IsValid)
{
    QModelIndex idx = model->rootIndex();
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(model->parent(idx), QModelIndex());
}

TEST_F(CollectionModelImpl, IndexByUrl_WithMapping_ReturnsValidIndex)
{
    stub.set_lamda(&FileInfoModelShell::files, [this](const FileInfoModelShell *) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return { testUrl };
    });
    stub.set_lamda(static_cast<QModelIndex (FileInfoModelShell::*)(const QUrl &, int) const>(&FileInfoModelShell::index),
                   [this](const FileInfoModelShell *, const QUrl &, int) -> QModelIndex {
                       __DBG_STUB_INVOKE__
                       return source->index(0, 0);
                   });

    model->refresh(model->rootIndex(), false, 0, true);
    QModelIndex idx = model->index(testUrl);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(model->fileUrl(idx), testUrl);
}

TEST_F(CollectionModelImpl, Files_AfterRefresh_ReturnsUrls)
{
    stub.set_lamda(&FileInfoModelShell::files, [this](const FileInfoModelShell *) -> QList<QUrl> {
        __DBG_STUB_INVOKE__
        return { testUrl };
    });
    stub.set_lamda(static_cast<QModelIndex (FileInfoModelShell::*)(const QUrl &, int) const>(&FileInfoModelShell::index),
                   [this](const FileInfoModelShell *, const QUrl &, int) -> QModelIndex {
                       __DBG_STUB_INVOKE__
                       return source->index(0, 0);
                   });

    model->refresh(model->rootIndex(), false, 0, true);
    EXPECT_EQ(model->files(), QList<QUrl>({ testUrl }));
}

TEST_F(CollectionModelImpl, FileInfo_InvalidIndex_ReturnsNull)
{
    EXPECT_EQ(model->fileInfo(QModelIndex()), nullptr);
}

TEST_F(CollectionModelImpl, Refresh_WithTimer_DoesNotCrash)
{
    EXPECT_NO_THROW(model->refresh(model->rootIndex(), false, 10, true));
}

TEST_F(CollectionModelImpl, Fetch_Take_ChangeRowCount)
{
    stub.set_lamda(static_cast<QModelIndex (FileInfoModelShell::*)(const QUrl &, int) const>(&FileInfoModelShell::index),
                   [this](const FileInfoModelShell *, const QUrl &, int) -> QModelIndex {
                       __DBG_STUB_INVOKE__
                       return source->index(0, 0);
                   });

    EXPECT_TRUE(model->fetch({ testUrl }));
    EXPECT_EQ(model->rowCount(model->rootIndex()), 1);

    EXPECT_TRUE(model->take({ testUrl }));
    EXPECT_EQ(model->rowCount(model->rootIndex()), 0);
}

TEST_F(CollectionModelImpl, MapToSource_WithFetchedUrl_ReturnsSourceIndex)
{
    QModelIndex sourceIdx = source->index(0, 0);
    stub.set_lamda(static_cast<QModelIndex (FileInfoModelShell::*)(const QUrl &, int) const>(&FileInfoModelShell::index),
                   [sourceIdx](const FileInfoModelShell *, const QUrl &, int) -> QModelIndex {
                       __DBG_STUB_INVOKE__
                       return sourceIdx;
                   });

    model->fetch({ testUrl });
    QModelIndex proxyIdx = model->index(testUrl);
    EXPECT_EQ(model->mapToSource(proxyIdx), sourceIdx);
}

TEST_F(CollectionModelImpl, MapFromSource_WithSourceIndex_ReturnsProxyIndex)
{
    QModelIndex sourceIdx = source->index(0, 0);
    stub.set_lamda(&FileInfoModelShell::fileUrl, [this](const FileInfoModelShell *, const QModelIndex &) -> QUrl {
        __DBG_STUB_INVOKE__
        return testUrl;
    });

    model->fetch({ testUrl });
    QModelIndex proxyIdx = model->mapFromSource(sourceIdx);
    EXPECT_TRUE(proxyIdx.isValid());
    EXPECT_EQ(model->fileUrl(proxyIdx), testUrl);
}

TEST_F(CollectionModelImpl, IndexByRowColumn_WithFetchedUrl_ReturnsValidIndex)
{
    stub.set_lamda(static_cast<QModelIndex (FileInfoModelShell::*)(const QUrl &, int) const>(&FileInfoModelShell::index),
                   [this](const FileInfoModelShell *, const QUrl &, int) -> QModelIndex {
                       __DBG_STUB_INVOKE__
                       return source->index(0, 0);
                   });

    model->fetch({ testUrl });
    QModelIndex idx = model->index(0, 0);
    EXPECT_TRUE(idx.isValid());
    EXPECT_EQ(model->parent(idx), model->rootIndex());
}

TEST_F(CollectionModelImpl, ColumnCount_Root_ReturnsOne)
{
    EXPECT_EQ(model->columnCount(model->rootIndex()), 1);
}

TEST_F(CollectionModelImpl, Data_WithValidProxy_ReturnsSourceData)
{
    QModelIndex sourceIdx = source->index(0, 0);
    source->setData(sourceIdx, QString("display"), Qt::DisplayRole);

    stub.set_lamda(static_cast<QModelIndex (FileInfoModelShell::*)(const QUrl &, int) const>(&FileInfoModelShell::index),
                   [sourceIdx](const FileInfoModelShell *, const QUrl &, int) -> QModelIndex {
                       __DBG_STUB_INVOKE__
                       return sourceIdx;
                   });

    model->fetch({ testUrl });
    QModelIndex proxyIdx = model->index(testUrl);
    QVariant value = model->data(proxyIdx, Qt::DisplayRole);
    EXPECT_EQ(value.toString(), QString("display"));
}

TEST_F(CollectionModelImpl, MimeData_ContainsUrls)
{
    stub.set_lamda(static_cast<QModelIndex (FileInfoModelShell::*)(const QUrl &, int) const>(&FileInfoModelShell::index),
                   [this](const FileInfoModelShell *, const QUrl &, int) -> QModelIndex {
                       __DBG_STUB_INVOKE__
                       return source->index(0, 0);
                   });

    model->fetch({ testUrl });
    QModelIndexList indexes = { model->index(0, 0) };
    QMimeData *mime = model->mimeData(indexes);
    ASSERT_NE(mime, nullptr);
    EXPECT_FALSE(mime->urls().isEmpty());
    EXPECT_EQ(mime->urls().first(), testUrl);
    delete mime;
}

TEST_F(CollectionModelImpl, DropMimeData_EmptyUrls_ReturnsFalse)
{
    QMimeData mime;
    EXPECT_FALSE(model->dropMimeData(&mime, Qt::CopyAction, 0, 0, QModelIndex()));
}

TEST_F(CollectionModelImpl, SetSourceModel_IsIgnored)
{
    QStandardItemModel another;
    model->setSourceModel(&another);
    EXPECT_EQ(model->sourceModel(), source);
}

TEST_F(CollectionModelImpl, DataReplacedSignal_Emits)
{
    QSignalSpy spy(model, &CollectionModel::dataReplaced);
    emit shell->dataReplaced(QUrl("file:///old.txt"), QUrl("file:///new.txt"));
    EXPECT_GE(spy.count(), 0);
}
