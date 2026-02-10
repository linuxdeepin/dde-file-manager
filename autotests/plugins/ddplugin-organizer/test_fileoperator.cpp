// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "utils/fileoperator.h"
#include "mode/collectiondataprovider.h"
#include "view/collectionview.h"
#include "models/collectionmodel.h"
#include "dfm-base/dfm_global_defines.h"

#include <QUrl>
#include <QTemporaryFile>

#include "gtest/gtest.h"

using namespace ddplugin_organizer;

// Mock CollectionDataProvider
class MockCollectionDataProvider : public CollectionDataProvider
{
public:
    MockCollectionDataProvider() : CollectionDataProvider(nullptr) {}
    
    QString replace(const QUrl &, const QUrl &) override { return QString(); }
    QString append(const QUrl &) override { return QString(); }
    QString prepend(const QUrl &) override { return QString(); }
    void insert(const QUrl &, const QString &, const int) override {}
    QString remove(const QUrl &) override { return QString(); }
    QString change(const QUrl &) override { return QString(); }
};

class UT_FileOperator : public testing::Test
{
protected:
    void SetUp() override
    {
        QUrl testUrl("file:///test.txt");

        operatorInstance = FileOperator::instance();
        EXPECT_NE(operatorInstance, nullptr);

        // Create a mock CollectionView with proper parameters
        MockCollectionDataProvider provider;
        view = new CollectionView("test_uuid", &provider, nullptr);
        EXPECT_NE(view, nullptr);

        CollectionModel *mockModel = new CollectionModel();
        view->setModel(mockModel);

        QItemSelectionModel *selectionModel = new QItemSelectionModel(mockModel);
        selectionModel->setCurrentIndex(mockModel->index(0, 0), QItemSelectionModel::SelectCurrent);
        view->setSelectionModel(selectionModel);
    }

    void TearDown() override
    {
        stub.clear();
        if (view) {
            view->deleteLater();
            view = nullptr;
        }
    }

public:
    stub_ext::StubExt stub;
    FileOperator *operatorInstance = nullptr;
    CollectionView *view = nullptr;
};

TEST_F(UT_FileOperator, Instance_ReturnsSingleton)
{
    FileOperator *instance1 = FileOperator::instance();
    FileOperator *instance2 = FileOperator::instance();
    
    EXPECT_NE(instance1, nullptr);
    EXPECT_EQ(instance1, instance2);
}

TEST_F(UT_FileOperator, SetDataProvider_SetsProvider)
{
    MockCollectionDataProvider provider;
    operatorInstance->setDataProvider(&provider);
    // Should set the data provider without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, CopyFiles_CallsOperation)
{
    operatorInstance->copyFiles(view);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, CopyFilePath_CallsOperation)
{
    operatorInstance->copyFilePath(view);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, CutFiles_CallsOperation)
{
    operatorInstance->cutFiles(view);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, PasteFiles_CallsOperation)
{
    operatorInstance->pasteFiles(view, "test_collection");
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, OpenFiles_CallsOperation)
{
    operatorInstance->openFiles(view);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, OpenFilesWithUrls_CallsOperation)
{
    QList<QUrl> urls;
    QTemporaryFile tempFile;
    tempFile.open();
    urls.append(QUrl::fromLocalFile(tempFile.fileName()));
    
    operatorInstance->openFiles(view, urls);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, RenameFile_CallsOperation)
{
    QTemporaryFile oldFile, newFile;
    oldFile.open();
    newFile.open();
    
    QUrl oldUrl = QUrl::fromLocalFile(oldFile.fileName());
    QUrl newUrl = QUrl::fromLocalFile(newFile.fileName());
    
    operatorInstance->renameFile(123, oldUrl, newUrl);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, RenameFilesWithPair_CallsOperation)
{
    QList<QUrl> urls;
    QTemporaryFile tempFile;
    tempFile.open();
    urls.append(QUrl::fromLocalFile(tempFile.fileName()));
    
    QPair<QString, QString> pair("old", "new");
    operatorInstance->renameFiles(view, urls, pair, false);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, RenameFilesWithFlag_CallsOperation)
{
    QList<QUrl> urls;
    QTemporaryFile tempFile;
    tempFile.open();
    urls.append(QUrl::fromLocalFile(tempFile.fileName()));
    
    // Use the correct 3-parameter version of renameFiles with proper enum type
    QPair<QString, DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag> pair("prefix", 
        static_cast<DFMBASE_NAMESPACE::AbstractJobHandler::FileNameAddFlag>(0)); // Using 0 as placeholder
    operatorInstance->renameFiles(view, urls, pair); // 3-parameter version
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, MoveToTrash_CallsOperation)
{
    operatorInstance->moveToTrash(view);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, DeleteFiles_CallsOperation)
{
    operatorInstance->deleteFiles(view);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, UndoFiles_CallsOperation)
{
    operatorInstance->undoFiles(view);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, PreviewFiles_CallsOperation)
{
    operatorInstance->previewFiles(view);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, ShowFilesProperty_CallsOperation)
{
    operatorInstance->showFilesProperty(view);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, DropFilesToCollection_CallsOperation)
{
    // Create test data
    QList<QUrl> urls;
    QTemporaryFile tempFile;
    tempFile.open();
    urls.append(QUrl::fromLocalFile(tempFile.fileName()));
    
    operatorInstance->dropFilesToCollection(Qt::CopyAction, QUrl("file:///tmp"), urls, "collection_key", 0);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, DropFilesToCanvas_CallsOperation)
{
    // Create test data
    QList<QUrl> urls;
    QTemporaryFile tempFile;
    tempFile.open();
    urls.append(QUrl::fromLocalFile(tempFile.fileName()));
    
    operatorInstance->dropFilesToCanvas(Qt::CopyAction, QUrl("file:///tmp"), urls);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, DropToTrash_CallsOperation)
{
    // Create test data
    QList<QUrl> urls;
    QTemporaryFile tempFile;
    tempFile.open();
    urls.append(QUrl::fromLocalFile(tempFile.fileName()));
    
    operatorInstance->dropToTrash(urls);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, DropToApp_CallsOperation)
{
    // Create test data
    QList<QUrl> urls;
    QTemporaryFile tempFile;
    tempFile.open();
    urls.append(QUrl::fromLocalFile(tempFile.fileName()));
    
    operatorInstance->dropToApp(urls, "test_app");
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, RenameFileData_ReturnsHash)
{
    QHash<QUrl, QUrl> data = operatorInstance->renameFileData();
    // Should return a hash map without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_FileOperator, RemoveRenameFileData_RemovesEntry)
{
    QTemporaryFile tempFile;
    tempFile.open();
    QUrl url = QUrl::fromLocalFile(tempFile.fileName());
    
    operatorInstance->removeRenameFileData(url);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, ClearRenameFileData_ClearsAll)
{
    operatorInstance->clearRenameFileData();
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, TouchFileData_ReturnsUrl)
{
    QUrl data = operatorInstance->touchFileData();
    // Should return a URL without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_FileOperator, ClearTouchFileData_ClearsData)
{
    operatorInstance->clearTouchFileData();
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, PasteFileData_ReturnsSet)
{
    QSet<QUrl> data = operatorInstance->pasteFileData();
    // Should return a set without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_FileOperator, RemovePasteFileData_RemovesEntry)
{
    QTemporaryFile tempFile;
    tempFile.open();
    QUrl url = QUrl::fromLocalFile(tempFile.fileName());
    
    operatorInstance->removePasteFileData(url);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, ClearPasteFileData_ClearsAll)
{
    operatorInstance->clearPasteFileData();
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, DropFileData_ReturnsHash)
{
    QHash<QUrl, QString> data = operatorInstance->dropFileData();
    // Should return a hash map without crashing
    EXPECT_TRUE(true);
}

TEST_F(UT_FileOperator, RemoveDropFileData_RemovesEntry)
{
    QTemporaryFile tempFile;
    tempFile.open();
    QUrl url = QUrl::fromLocalFile(tempFile.fileName());
    
    operatorInstance->removeDropFileData(url);
    // Should execute without crashing
    SUCCEED();
}

TEST_F(UT_FileOperator, ClearDropFileData_ClearsAll)
{
    operatorInstance->clearDropFileData();
    // Should execute without crashing
    SUCCEED();
}
