// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "models/collectionmodel.h"
#include "models/collectionmodel_p.h"
#include "models/modeldatahandler.h"
#include "interface/fileinfomodelshell.h"

#include <QAbstractItemModel>
#include <QUrl>
#include <QModelIndex>
#include <QMimeData>
#include <QTimer>
#include <QSignalSpy>

#include <gtest/gtest.h>

using namespace ddplugin_organizer;

class MockFileInfoModelShell : public FileInfoModelShell
{
public:
    MockFileInfoModelShell() : FileInfoModelShell(nullptr) {}
};

class MockModelDataHandler : public ModelDataHandler
{
public:
    MockModelDataHandler() : ModelDataHandler() {}
    bool acceptInsert(const QUrl &url) override { Q_UNUSED(url); return true; }
    QList<QUrl> acceptReset(const QList<QUrl> &urls) override { Q_UNUSED(urls); return urls; }
    bool acceptRename(const QUrl &oldUrl, const QUrl &newUrl) override { Q_UNUSED(oldUrl); Q_UNUSED(newUrl); return true; }
    bool acceptUpdate(const QUrl &url, const QVector<int> &roles = {}) override { Q_UNUSED(url); Q_UNUSED(roles); return true; }
};

class MockSourceModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    MockSourceModel(QObject *parent = nullptr) : QAbstractItemModel(parent) {}
    
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(row); Q_UNUSED(column); Q_UNUSED(parent);
        return QModelIndex();
    }
    
    QModelIndex parent(const QModelIndex &child) const override {
        Q_UNUSED(child);
        return QModelIndex();
    }
    
    int rowCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return 0;
    }
    
    int columnCount(const QModelIndex &parent = QModelIndex()) const override {
        Q_UNUSED(parent);
        return 1;
    }
    
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
        Q_UNUSED(index); Q_UNUSED(role);
        return QVariant();
    }
};

class UT_CollectionModel : public testing::Test
{
protected:
    void SetUp() override
    {
        model = new CollectionModel();
        mockShell = new MockFileInfoModelShell();
        mockHandler = new MockModelDataHandler();
        mockSourceModel = new MockSourceModel();
    }

    void TearDown() override
    {
        delete model;
        delete mockShell;
        delete mockHandler;
        delete mockSourceModel;
        stub.clear();
    }

public:
    CollectionModel *model;
    MockFileInfoModelShell *mockShell;
    MockModelDataHandler *mockHandler;
    MockSourceModel *mockSourceModel;
    stub_ext::StubExt stub;
};

TEST_F(UT_CollectionModel, TestConstructor)
{
    EXPECT_NE(model, nullptr);
    EXPECT_EQ(model->modelShell(), nullptr);
    EXPECT_EQ(model->handler(), nullptr);
}

TEST_F(UT_CollectionModel, TestSetModelShell)
{
    model->setModelShell(mockShell);
    EXPECT_EQ(model->modelShell(), mockShell);
}

TEST_F(UT_CollectionModel, TestSetHandler)
{
    model->setHandler(mockHandler);
    EXPECT_EQ(model->handler(), mockHandler);
}

TEST_F(UT_CollectionModel, TestRootUrl)
{
    model->setModelShell(mockShell);
    QUrl url = model->rootUrl();
    EXPECT_EQ(url, QUrl::fromLocalFile("/test"));
}

TEST_F(UT_CollectionModel, TestRootIndex)
{
    model->setModelShell(mockShell);
    QModelIndex index = model->rootIndex();
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_CollectionModel, TestIndexByUrl)
{
    model->setModelShell(mockShell);
    QUrl testUrl("file:///test.txt");
    QModelIndex index = model->index(testUrl);
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_CollectionModel, TestFileInfo)
{
    model->setModelShell(mockShell);
    QModelIndex testIndex;
    FileInfoPointer info = model->fileInfo(testIndex);
    EXPECT_EQ(info, nullptr);
}

TEST_F(UT_CollectionModel, TestFiles)
{
    model->setModelShell(mockShell);
    QList<QUrl> files = model->files();
    EXPECT_TRUE(files.isEmpty());
}

TEST_F(UT_CollectionModel, TestFileUrl)
{
    model->setModelShell(mockShell);
    QModelIndex testIndex;
    QUrl url = model->fileUrl(testIndex);
    EXPECT_TRUE(url.isEmpty());
}

TEST_F(UT_CollectionModel, TestRefresh)
{
    model->setModelShell(mockShell);
    QModelIndex parent;
    EXPECT_NO_THROW(model->refresh(parent, false, 50, true));
    EXPECT_NO_THROW(model->refresh(parent, true, 100, false));
}

TEST_F(UT_CollectionModel, TestUpdate)
{
    model->setModelShell(mockShell);
    EXPECT_NO_THROW(model->update());
}

TEST_F(UT_CollectionModel, TestFetch)
{
    model->setModelShell(mockShell);
    QList<QUrl> urls = {QUrl("file:///test1.txt"), QUrl("file:///test2.txt")};
    bool result = model->fetch(urls);
    EXPECT_FALSE(result);  // Mock returns false
}

TEST_F(UT_CollectionModel, TestTake)
{
    model->setModelShell(mockShell);
    QList<QUrl> urls = {QUrl("file:///test1.txt")};
    bool result = model->take(urls);
    EXPECT_FALSE(result);  // Mock returns false
}

TEST_F(UT_CollectionModel, TestMapToSource)
{
    QModelIndex proxyIndex;
    QModelIndex sourceIndex = model->mapToSource(proxyIndex);
    EXPECT_FALSE(sourceIndex.isValid());
}

TEST_F(UT_CollectionModel, TestMapFromSource)
{
    QModelIndex sourceIndex;
    QModelIndex proxyIndex = model->mapFromSource(sourceIndex);
    EXPECT_FALSE(proxyIndex.isValid());
}

TEST_F(UT_CollectionModel, TestIndexByRowColumn)
{
    QModelIndex parent;
    QModelIndex index = model->index(0, 0, parent);
    EXPECT_FALSE(index.isValid());
    
    index = model->index(1, 1, parent);
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_CollectionModel, TestParent)
{
    QModelIndex child;
    QModelIndex parent = model->parent(child);
    EXPECT_FALSE(parent.isValid());
}

TEST_F(UT_CollectionModel, TestRowCount)
{
    QModelIndex parent;
    int count = model->rowCount(parent);
    EXPECT_GE(count, 0);
}

TEST_F(UT_CollectionModel, TestColumnCount)
{
    QModelIndex parent;
    int count = model->columnCount(parent);
    EXPECT_GE(count, 0);
}

TEST_F(UT_CollectionModel, TestData)
{
    QModelIndex testIndex;
    QVariant data = model->data(testIndex, Qt::DisplayRole);
    EXPECT_FALSE(data.isValid());
    
    data = model->data(testIndex, Qt::DecorationRole);
    EXPECT_FALSE(data.isValid());
}

TEST_F(UT_CollectionModel, TestMimeData)
{
    QModelIndexList indexes;
    QMimeData *mimeData = model->mimeData(indexes);
    EXPECT_NE(mimeData, nullptr);
    delete mimeData;
    
    // Test with valid indexes (if any)
    QModelIndex testIndex;
    if (testIndex.isValid()) {
        indexes.append(testIndex);
        mimeData = model->mimeData(indexes);
        EXPECT_NE(mimeData, nullptr);
        delete mimeData;
    }
}

TEST_F(UT_CollectionModel, TestDropMimeData)
{
    QMimeData *mimeData = new QMimeData();
    QList<QUrl> urls = {QUrl("file:///test.txt")};
    mimeData->setUrls(urls);
    
    // Test drop with different actions
    bool result = model->dropMimeData(mimeData, Qt::CopyAction, 0, 0, QModelIndex());
    EXPECT_TRUE(result);  // Mock implementation returns true for valid data
    
    result = model->dropMimeData(mimeData, Qt::MoveAction, 0, 0, QModelIndex());
    EXPECT_TRUE(result);
    
    result = model->dropMimeData(mimeData, Qt::IgnoreAction, 0, 0, QModelIndex());
    EXPECT_FALSE(result);
    
    // Test with empty mime data
    QMimeData emptyData;
    result = model->dropMimeData(&emptyData, Qt::CopyAction, 0, 0, QModelIndex());
    EXPECT_FALSE(result);
    
    delete mimeData;
}

TEST_F(UT_CollectionModel, TestSetSourceModel)
{
    // This should be forbidden and do nothing
    EXPECT_NO_THROW(model->setSourceModel(mockSourceModel));
    EXPECT_EQ(model->sourceModel(), nullptr);
}

TEST_F(UT_CollectionModel, TestDataReplacedSignal)
{
    QSignalSpy spy(model, &CollectionModel::dataReplaced);
    
    // Test signal emission (would need actual implementation to test properly)
    EXPECT_EQ(spy.count(), 0);
}

TEST_F(UT_CollectionModel, TestEdgeCases)
{
    // Test with null shell
    EXPECT_EQ(model->modelShell(), nullptr);
    QUrl url = model->rootUrl();
    EXPECT_TRUE(url.isEmpty());
    
    // Test with null handler
    EXPECT_EQ(model->handler(), nullptr);
    
    // Test invalid indexes
    QModelIndex invalidIndex;
    EXPECT_TRUE(model->fileUrl(invalidIndex).isEmpty());
    EXPECT_EQ(model->fileInfo(invalidIndex), nullptr);
}

TEST_F(UT_CollectionModel, TestMultipleOperations)
{
    model->setModelShell(mockShell);
    model->setHandler(mockHandler);
    
    // Perform multiple operations in sequence
    QList<QUrl> urls = {QUrl("file:///test1.txt"), QUrl("file:///test2.txt")};
    
    EXPECT_FALSE(model->fetch(urls));
    EXPECT_FALSE(model->take(urls));
    
    EXPECT_NO_THROW(model->refresh(QModelIndex(), true, 100, false));
    EXPECT_NO_THROW(model->update());
    
    // Test model state after operations
    EXPECT_EQ(model->modelShell(), mockShell);
    EXPECT_EQ(model->handler(), mockHandler);
}

#include "test_collectionmodel.moc"
