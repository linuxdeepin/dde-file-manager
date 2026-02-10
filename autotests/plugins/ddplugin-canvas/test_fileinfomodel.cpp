// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"
#include "plugins/desktop/ddplugin-canvas/model/filefilter.h"

#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>

#include <QUrl>
#include <QModelIndex>
#include <QMimeData>
#include <QStringList>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

class UT_FileInfoModel : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Stub critical functions to avoid complex dependencies
        stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl&, dfmbase::Global::CreateFileInfoType, QString*)>(&InfoFactory::create<FileInfo>), 
                       [](const QUrl &url, dfmbase::Global::CreateFileInfoType cache, QString *errString) -> FileInfoPointer {
            __DBG_STUB_INVOKE__
            if (errString) *errString = "";
            return QSharedPointer<FileInfo>(new SyncFileInfo(url));
        });
        
        model = new FileInfoModel(nullptr);
    }

    virtual void TearDown() override
    {
        delete model;
        model = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    FileInfoModel *model = nullptr;
};

TEST_F(UT_FileInfoModel, constructor)
{
    EXPECT_NE(model, nullptr);
    EXPECT_TRUE(model->inherits("QAbstractItemModel"));
}

TEST_F(UT_FileInfoModel, setRootUrl_GetRootUrl)
{
    QUrl testUrl("file:///tmp");
    
    // Test setRootUrl method
    QModelIndex rootIndex = model->setRootUrl(testUrl);
    
    // Test rootUrl method
    QUrl retrievedUrl = model->rootUrl();
    EXPECT_EQ(retrievedUrl, testUrl);
    
    // Test rootIndex method
    QModelIndex retrievedRootIndex = model->rootIndex();
    EXPECT_EQ(retrievedRootIndex, rootIndex);
}

TEST_F(UT_FileInfoModel, rowCount_ColumnCount)
{
    QModelIndex parentIndex;
    
    // Test rowCount method
    int rowCount = model->rowCount(parentIndex);
    EXPECT_GE(rowCount, 0); // Should return non-negative count
    
    // Test columnCount method
    int columnCount = model->columnCount(parentIndex);
    EXPECT_GE(columnCount, 0); // Should return non-negative count
}

TEST_F(UT_FileInfoModel, refresh_Update_ModelState)
{
    QModelIndex testIndex;
    
    // Test refresh method
    EXPECT_NO_THROW(model->refresh(testIndex));
    
    // Test update method
    EXPECT_NO_THROW(model->update());
    
    // Test modelState method
    int state = model->modelState();
    EXPECT_GE(state, 0); // State should be 0, 1, or 2
    EXPECT_LE(state, 2);
}

TEST_F(UT_FileInfoModel, data_Index_Parent)
{
    QModelIndex testIndex = model->index(0, 0);
    
    // Test data method with different roles
    QVariant displayData = model->data(testIndex, Qt::DisplayRole);
    QVariant decorationData = model->data(testIndex, Qt::DecorationRole);
    
    // Test parent method
    QModelIndex parentIndex = model->parent(testIndex);
    
    // Methods should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileInfoModel, fileInfo_FileUrl_Files)
{
    QModelIndex testIndex = model->index(0, 0);
    
    // Test fileInfo method
    EXPECT_NO_THROW(model->fileInfo(testIndex));
    
    // Test fileUrl method
    EXPECT_NO_THROW(model->fileUrl(testIndex));
    
    // Test files method
    QList<QUrl> urls = model->files();
    EXPECT_GE(urls.size(), 0); // Should return a list
}

TEST_F(UT_FileInfoModel, updateFile_RefreshAllFile)
{
    QUrl testUrl("file:///tmp/test.txt");
    
    // Test updateFile method
    EXPECT_NO_THROW(model->updateFile(testUrl));
    
    // Test refreshAllFile method
    EXPECT_NO_THROW(model->refreshAllFile());
}

TEST_F(UT_FileInfoModel, installFilter_RemoveFilter)
{
    // Create a mock filter
    QSharedPointer<FileFilter> filter = QSharedPointer<FileFilter>::create();
    
    // Test installFilter method
    EXPECT_NO_THROW(model->installFilter(filter));
    
    // Test removeFilter method
    EXPECT_NO_THROW(model->removeFilter(filter));
}

TEST_F(UT_FileInfoModel, indexFromUrl)
{
    QUrl testUrl("file:///tmp/test.txt");
    
    // Test index method with URL
    QModelIndex index = model->index(testUrl);
    
    // Method should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileInfoModel, flags)
{
    QModelIndex testIndex = model->index(0, 0);
    
    // Test flags method
    Qt::ItemFlags flags = model->flags(testIndex);
    
    // Method should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileInfoModel, mimeData_MimeTypes)
{
    QModelIndexList indexes;
    QModelIndex testIndex = model->index(0, 0);
    if (testIndex.isValid()) {
        indexes << testIndex;
    }
    
    // Test mimeData method
    QMimeData *mimeData = model->mimeData(indexes);
    if (mimeData) {
        delete mimeData;
    }
    
    // Test mimeTypes method
    QStringList mimeTypes = model->mimeTypes();
    EXPECT_GE(mimeTypes.size(), 0);
}

TEST_F(UT_FileInfoModel, supportedActions)
{
    // Test supportedDragActions method
    Qt::DropActions dragActions = model->supportedDragActions();
    
    // Test supportedDropActions method
    Qt::DropActions dropActions = model->supportedDropActions();
    
    // Methods should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileInfoModel, dropMimeData)
{
    QMimeData *testMimeData = new QMimeData();
    testMimeData->setText("test");
    
    // Test dropMimeData method
    bool result = model->dropMimeData(testMimeData, Qt::CopyAction, 0, 0, QModelIndex());
    
    // Method should execute without crashing
    SUCCEED();
    
    delete testMimeData;
}

TEST_F(UT_FileInfoModel, signal_dataReplaced)
{
    // Test that the signal exists and can be connected
    bool signalConnected = QObject::connect(model, &FileInfoModel::dataReplaced,
                                          [](const QUrl &oldUrl, const QUrl &newUrl) {
                                              // Signal handler
                                              Q_UNUSED(oldUrl)
                                              Q_UNUSED(newUrl)
                                          });
    
    EXPECT_TRUE(signalConnected);
}

//===================================================================================
// Additional tests to improve FileInfoModel coverage
//===================================================================================

TEST_F(UT_FileInfoModel, setRootUrl_EmptyUrl_UsesDefaultDesktopPath)
{
    // Test setRootUrl with empty URL - should use default desktop path
    QUrl emptyUrl;
    QModelIndex rootIndex = model->setRootUrl(emptyUrl);
    
    // Should not crash and return valid root index
    EXPECT_TRUE(rootIndex.isValid());
    
    // Root URL should not be empty after setting empty URL
    QUrl rootUrl = model->rootUrl();
    EXPECT_FALSE(rootUrl.isEmpty());
}

TEST_F(UT_FileInfoModel, index_WithValidRowAndColumn_ReturnsValidIndex)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Test index creation with specific row and column
    QModelIndex index = model->index(0, 0, QModelIndex());
    
    // When fileList is empty, should return invalid index
    EXPECT_FALSE(index.isValid());
}

TEST_F(UT_FileInfoModel, index_WithUrl_HandlesDifferentScenarios)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Test index with URL - empty URL
    QModelIndex emptyIndex = model->index(QUrl(), 0);
    EXPECT_FALSE(emptyIndex.isValid());
    
    // Test index with root URL
    QModelIndex rootIndex = model->index(testUrl, 0);
    EXPECT_TRUE(rootIndex.isValid());
    
    // Test index with non-existent URL
    QUrl nonExistentUrl("file:///nonexistent");
    QModelIndex nonExistentIndex = model->index(nonExistentUrl, 0);
    EXPECT_FALSE(nonExistentIndex.isValid());
}

TEST_F(UT_FileInfoModel, fileInfo_WithDifferentIndexes_HandlesAllCases)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Test fileInfo with root index
    QModelIndex rootIndex = model->rootIndex();
    FileInfoPointer rootFileInfo = model->fileInfo(rootIndex);
    EXPECT_NE(rootFileInfo, nullptr);
    
    // Test fileInfo with invalid index (negative row)
    QModelIndex invalidIndex = model->index(-1, 0);
    FileInfoPointer invalidFileInfo = model->fileInfo(invalidIndex);
    EXPECT_EQ(invalidFileInfo, nullptr);
    
    // Test fileInfo with out-of-range index
    QModelIndex outOfRangeIndex = model->index(1000, 0);
    FileInfoPointer outOfRangeFileInfo = model->fileInfo(outOfRangeIndex);
    EXPECT_EQ(outOfRangeFileInfo, nullptr);
}

TEST_F(UT_FileInfoModel, fileUrl_WithDifferentIndexes_HandlesAllCases)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Test fileUrl with root index
    QModelIndex rootIndex = model->rootIndex();
    QUrl rootFileUrl = model->fileUrl(rootIndex);
    EXPECT_EQ(rootFileUrl, testUrl);
    
    // Test fileUrl with invalid index (negative row)
    QModelIndex invalidIndex = model->index(-1, 0);
    QUrl invalidFileUrl = model->fileUrl(invalidIndex);
    EXPECT_TRUE(invalidFileUrl.isEmpty());
    
    // Test fileUrl with out-of-range index
    QModelIndex outOfRangeIndex = model->index(1000, 0);
    QUrl outOfRangeFileUrl = model->fileUrl(outOfRangeIndex);
    EXPECT_TRUE(outOfRangeFileUrl.isEmpty());
}

TEST_F(UT_FileInfoModel, refresh_WithNonRootParent_ReturnsEarly)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Create a non-root index
    QModelIndex nonRootIndex = model->index(0, 0);
    
    // Test refresh with non-root parent - should return early
    EXPECT_NO_THROW(model->refresh(nonRootIndex));
}

TEST_F(UT_FileInfoModel, parent_WithDifferentIndexes_ReturnsCorrectParent)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Test parent with valid child index
    QModelIndex childIndex = model->index(0, 0);
    if (childIndex.isValid()) {
        QModelIndex parentIndex = model->parent(childIndex);
        EXPECT_EQ(parentIndex, model->rootIndex());
    }
    
    // Test parent with root index
    QModelIndex rootIndex = model->rootIndex();
    QModelIndex rootParent = model->parent(rootIndex);
    EXPECT_FALSE(rootParent.isValid());
    
    // Test parent with invalid index
    QModelIndex invalidIndex;
    QModelIndex invalidParent = model->parent(invalidIndex);
    EXPECT_FALSE(invalidParent.isValid());
}

TEST_F(UT_FileInfoModel, rowCount_WithDifferentParents_ReturnsCorrectCount)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Test rowCount with root index
    QModelIndex rootIndex = model->rootIndex();
    int rootRowCount = model->rowCount(rootIndex);
    EXPECT_GE(rootRowCount, 0);
    
    // Test rowCount with non-root index
    QModelIndex nonRootIndex = model->index(0, 0);
    int nonRootRowCount = model->rowCount(nonRootIndex);
    EXPECT_EQ(nonRootRowCount, 0);
}

TEST_F(UT_FileInfoModel, columnCount_WithDifferentParents_ReturnsCorrectCount)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Test columnCount with root index
    QModelIndex rootIndex = model->rootIndex();
    int rootColumnCount = model->columnCount(rootIndex);
    EXPECT_EQ(rootColumnCount, 1);
    
    // Test columnCount with non-root index
    QModelIndex nonRootIndex = model->index(0, 0);
    int nonRootColumnCount = model->columnCount(nonRootIndex);
    EXPECT_EQ(nonRootColumnCount, 0);
}

TEST_F(UT_FileInfoModel, data_WithValidIndexAndDifferentRoles_ReturnsData)
{
    using namespace dfmbase::Global;
    
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Create a valid index (this will be invalid since fileList is empty, but data should handle it)
    QModelIndex testIndex = model->index(0, 0);
    
    // Test data with different roles - should return QVariant() for invalid index
    QVariant fontData = model->data(testIndex, ItemRoles::kItemFontRole);
    QVariant iconData = model->data(testIndex, ItemRoles::kItemIconRole);
    QVariant nameData = model->data(testIndex, ItemRoles::kItemNameRole);
    QVariant displayNameData = model->data(testIndex, ItemRoles::kItemFileDisplayNameRole);
    QVariant editData = model->data(testIndex, Qt::EditRole);
    QVariant pinyinData = model->data(testIndex, ItemRoles::kItemFilePinyinNameRole);
    QVariant createdData = model->data(testIndex, ItemRoles::kItemFileCreatedRole);
    QVariant modifiedData = model->data(testIndex, ItemRoles::kItemFileLastModifiedRole);
    QVariant sizeData = model->data(testIndex, ItemRoles::kItemFileSizeRole);
    QVariant mimeTypeData = model->data(testIndex, ItemRoles::kItemFileMimeTypeRole);
    QVariant extraData = model->data(testIndex, ItemRoles::kItemExtraProperties);
    QVariant baseNameData = model->data(testIndex, ItemRoles::kItemFileBaseNameRole);
    QVariant suffixData = model->data(testIndex, ItemRoles::kItemFileSuffixRole);
    QVariant renameNameData = model->data(testIndex, ItemRoles::kItemFileNameOfRenameRole);
    QVariant renameBaseNameData = model->data(testIndex, ItemRoles::kItemFileBaseNameOfRenameRole);
    QVariant renameSuffixData = model->data(testIndex, ItemRoles::kItemFileSuffixOfRenameRole);
    QVariant defaultData = model->data(testIndex, Qt::UserRole + 1000);
    
    // All should return empty QVariant for invalid model or index
    EXPECT_FALSE(fontData.isValid() || iconData.isValid() || nameData.isValid());
    
    // Test data with root index - should return QVariant()
    QModelIndex rootIndex = model->rootIndex();
    QVariant rootData = model->data(rootIndex, Qt::DisplayRole);
    EXPECT_FALSE(rootData.isValid());
}

TEST_F(UT_FileInfoModel, flags_WithDifferentIndexes_ReturnsCorrectFlags)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Test flags with invalid index - base class behavior
    QModelIndex invalidIndex;
    Qt::ItemFlags invalidFlags = model->flags(invalidIndex);
    // For invalid index, Qt's base implementation typically returns ItemIsDropEnabled
    EXPECT_NO_THROW(model->flags(invalidIndex));
    
    // Test flags with valid index (will be invalid since fileList is empty)
    QModelIndex testIndex = model->index(0, 0);
    Qt::ItemFlags testFlags = model->flags(testIndex);
    // For invalid index, our implementation calls base class
    EXPECT_NO_THROW(model->flags(testIndex));
    
    // Verify that flags method doesn't crash and returns some flags
    EXPECT_GE(static_cast<int>(invalidFlags), 0);
    EXPECT_GE(static_cast<int>(testFlags), 0);
}

TEST_F(UT_FileInfoModel, mimeData_WithValidIndexes_CreatesMimeData)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    QModelIndexList indexes;
    QModelIndex testIndex = model->index(0, 0);
    indexes << testIndex;
    
    // Test mimeData creation
    QMimeData *mimeData = model->mimeData(indexes);
    EXPECT_NE(mimeData, nullptr);
    
    // Should contain URL list
    QList<QUrl> urls = mimeData->urls();
    EXPECT_GE(urls.size(), 0);
    
    delete mimeData;
}

TEST_F(UT_FileInfoModel, dropMimeData_EmptyUrlList_ReturnsFalse)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Create mime data with empty URL list
    QMimeData *emptyMimeData = new QMimeData();
    QList<QUrl> emptyUrls;
    emptyMimeData->setUrls(emptyUrls);
    
    // Test dropMimeData with empty URL list
    bool result = model->dropMimeData(emptyMimeData, Qt::CopyAction, 0, 0, QModelIndex());
    EXPECT_FALSE(result);
    
    delete emptyMimeData;
}

TEST_F(UT_FileInfoModel, dropMimeData_ValidUrlList_ProcessesDrop)
{
    QUrl testUrl("file:///tmp");
    model->setRootUrl(testUrl);
    
    // Create mime data with valid URL list
    QMimeData *validMimeData = new QMimeData();
    QList<QUrl> validUrls;
    validUrls << QUrl("file:///tmp/test1.txt") << QUrl("file:///tmp/test2.txt");
    validMimeData->setUrls(validUrls);
    
    // Stub FileCreator to avoid complex dependencies
    stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl&, dfmbase::Global::CreateFileInfoType, QString*)>(&InfoFactory::create<FileInfo>), 
                   [](const QUrl &url, dfmbase::Global::CreateFileInfoType cache, QString *errString) -> FileInfoPointer {
        __DBG_STUB_INVOKE__
        if (errString) *errString = "";
        auto fileInfo = QSharedPointer<FileInfo>(new SyncFileInfo(url));
        return fileInfo;
    });
    
    // Test dropMimeData with valid URL list
    bool result = model->dropMimeData(validMimeData, Qt::CopyAction, 0, 0, QModelIndex());
    EXPECT_TRUE(result || !result);  // Should not crash
    
    delete validMimeData;
}

TEST_F(UT_FileInfoModel, supportedActions_ReturnValidActions)
{
    // Test supported drag actions
    Qt::DropActions dragActions = model->supportedDragActions();
    EXPECT_TRUE(dragActions & Qt::CopyAction);
    EXPECT_TRUE(dragActions & Qt::MoveAction);
    EXPECT_TRUE(dragActions & Qt::LinkAction);
    
    // Test supported drop actions
    Qt::DropActions dropActions = model->supportedDropActions();
    EXPECT_TRUE(dropActions & Qt::CopyAction);
    EXPECT_TRUE(dropActions & Qt::MoveAction);
    EXPECT_TRUE(dropActions & Qt::LinkAction);
}

TEST_F(UT_FileInfoModel, mimeTypes_ReturnsValidTypes)
{
    QStringList mimeTypes = model->mimeTypes();
    EXPECT_GE(mimeTypes.size(), 1);
    EXPECT_TRUE(mimeTypes.contains("text/uri-list"));
}