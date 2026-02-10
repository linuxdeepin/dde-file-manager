// SPDX-FileCopyrightText: 2025 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stubext.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel.h"
#include "plugins/desktop/ddplugin-canvas/model/fileinfomodel_p.h"
#include "plugins/desktop/ddplugin-canvas/model/fileprovider.h"

#include <dfm-base/file/local/syncfileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/thumbnail/thumbnailfactory.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/file/local/desktopfileinfo.h>

#include <QUrl>
#include <QModelIndex>
#include <QMimeData>
#include <QStringList>
#include <QSignalSpy>
#include <QTimer>

#include <gtest/gtest.h>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_canvas;

// Test class specifically for FileInfoModel private methods
class UT_FileInfoModelPrivate : public testing::Test
{
protected:
    virtual void SetUp() override
    {
        // Stub critical functions to avoid complex dependencies
        stub.set_lamda(static_cast<FileInfoPointer(*)(const QUrl&, dfmbase::Global::CreateFileInfoType, QString*)>(&InfoFactory::create<FileInfo>), 
                       [this](const QUrl &url, dfmbase::Global::CreateFileInfoType cache, QString *errString) -> FileInfoPointer {
            __DBG_STUB_INVOKE__
            if (errString) *errString = "";
            
            // Create a simple SyncFileInfo to avoid recursive constructor calls
            // Using SyncFileInfo instead of DesktopFileInfo to prevent InfoFactory::create recursion
            auto fileInfo = QSharedPointer<SyncFileInfo>(new SyncFileInfo(url));
            return fileInfo;
        });
        
        // Stub ThumbnailFactory to avoid threading issues
        stub.set_lamda(&ThumbnailFactory::joinThumbnailJob, [](ThumbnailFactory*, const QUrl&, dfmbase::Global::ThumbnailSize) {
            __DBG_STUB_INVOKE__
            // Do nothing
        });
        
        // Stub FileUtils::isDesktopFileSuffix
        stub.set_lamda(&FileUtils::isDesktopFileSuffix, [](const QUrl& url) -> bool {
            __DBG_STUB_INVOKE__
            return url.toLocalFile().endsWith(".desktop");
        });
        
        // Stub FileUtils::refreshIconCache
        stub.set_lamda(&FileUtils::refreshIconCache, []() {
            __DBG_STUB_INVOKE__
            // Do nothing
        });
        
        model = new FileInfoModel(nullptr);
        fileProvider = model->d->fileProvider;
    }

    virtual void TearDown() override
    {
        delete model;
        model = nullptr;
        fileProvider = nullptr;
        stub.clear();
    }

public:
    stub_ext::StubExt stub;
    FileInfoModel *model = nullptr;
    FileProvider *fileProvider = nullptr;
};

TEST_F(UT_FileInfoModelPrivate, resetData_WithValidUrls_ResetsModelData)
{
    // Set up model with root URL
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    // Create test URLs
    QList<QUrl> testUrls;
    testUrls << QUrl("file:///tmp/test1.txt") 
             << QUrl("file:///tmp/test2.txt") 
             << QUrl("file:///tmp/app.desktop");
    
    // Monitor model reset signals
    QSignalSpy beginResetSpy(model, &QAbstractItemModel::modelAboutToBeReset);
    QSignalSpy endResetSpy(model, &QAbstractItemModel::modelReset);
    
    // Trigger resetData via FileProvider::refreshEnd signal
    emit fileProvider->refreshEnd(testUrls);
    
    // Verify model reset signals were emitted
    EXPECT_EQ(beginResetSpy.count(), 1);
    EXPECT_EQ(endResetSpy.count(), 1);
    
    // Verify model state is updated
    EXPECT_EQ(model->modelState(), FileInfoModelPrivate::NormalState);
    
    // Verify files are added to model
    QList<QUrl> files = model->files();
    EXPECT_EQ(files.size(), 3);
    EXPECT_TRUE(files.contains(QUrl("file:///tmp/test1.txt")));
    EXPECT_TRUE(files.contains(QUrl("file:///tmp/test2.txt")));
    EXPECT_TRUE(files.contains(QUrl("file:///tmp/app.desktop")));
}

TEST_F(UT_FileInfoModelPrivate, insertData_WithNewUrl_InsertsFileToModel)
{
    // Set up model with root URL
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    // Initial file count should be 0
    EXPECT_EQ(model->rowCount(model->rootIndex()), 0);
    
    // Monitor row insertion signals
    QSignalSpy beginInsertSpy(model, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy endInsertSpy(model, &QAbstractItemModel::rowsInserted);
    
    // Trigger insertData via FileProvider::fileInserted signal
    QUrl testUrl("file:///tmp/newfile.txt");
    emit fileProvider->fileInserted(testUrl);
    
    // Verify row insertion signals were emitted
    EXPECT_EQ(beginInsertSpy.count(), 1);
    EXPECT_EQ(endInsertSpy.count(), 1);
    
    // Verify file count increased
    EXPECT_EQ(model->rowCount(model->rootIndex()), 1);
    
    // Verify the file was added
    QList<QUrl> files = model->files();
    EXPECT_TRUE(files.contains(testUrl));
}

TEST_F(UT_FileInfoModelPrivate, insertData_WithExistingUrl_RefreshesFileInfo)
{
    // Set up model with existing file
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    QUrl testUrl("file:///tmp/existing.txt");
    emit fileProvider->fileInserted(testUrl);
    
    // Verify initial state
    EXPECT_EQ(model->rowCount(model->rootIndex()), 1);
    
    // Monitor data changed signal
    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
    
    // Try to insert the same file again
    emit fileProvider->fileInserted(testUrl);
    
    // Should not increase count but should emit dataChanged
    EXPECT_EQ(model->rowCount(model->rootIndex()), 1);
    EXPECT_EQ(dataChangedSpy.count(), 1);
}

TEST_F(UT_FileInfoModelPrivate, insertData_WithDesktopFile_CallsDesktopIconRefresh)
{
    // Set up model with root URL
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    // Stub checkAndRefreshDesktopIcon to verify it's called
    bool desktopIconRefreshCalled = false;
    stub.set_lamda(&FileInfoModelPrivate::checkAndRefreshDesktopIcon, 
                   [&desktopIconRefreshCalled](FileInfoModelPrivate*, const FileInfoPointer&, int) {
        __DBG_STUB_INVOKE__
        desktopIconRefreshCalled = true;
    });
    
    // Insert a desktop file
    QUrl desktopUrl("file:///tmp/app.desktop");
    emit fileProvider->fileInserted(desktopUrl);
    
    // Verify desktop icon refresh was called
    EXPECT_TRUE(desktopIconRefreshCalled);
}

TEST_F(UT_FileInfoModelPrivate, removeData_WithExistingUrl_RemovesFileFromModel)
{
    // Set up model with existing file
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    QUrl testUrl("file:///tmp/toremove.txt");
    emit fileProvider->fileInserted(testUrl);
    
    // Verify initial state
    EXPECT_EQ(model->rowCount(model->rootIndex()), 1);
    
    // Monitor row removal signals
    QSignalSpy beginRemoveSpy(model, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy endRemoveSpy(model, &QAbstractItemModel::rowsRemoved);
    
    // Trigger removeData via FileProvider::fileRemoved signal
    emit fileProvider->fileRemoved(testUrl);
    
    // Verify row removal signals were emitted
    EXPECT_EQ(beginRemoveSpy.count(), 1);
    EXPECT_EQ(endRemoveSpy.count(), 1);
    
    // Verify file count decreased
    EXPECT_EQ(model->rowCount(model->rootIndex()), 0);
    
    // Verify the file was removed
    QList<QUrl> files = model->files();
    EXPECT_FALSE(files.contains(testUrl));
}

TEST_F(UT_FileInfoModelPrivate, removeData_WithNonExistentUrl_DoesNothing)
{
    // Set up model with root URL
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    // Monitor row removal signals
    QSignalSpy beginRemoveSpy(model, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy endRemoveSpy(model, &QAbstractItemModel::rowsRemoved);
    
    // Try to remove non-existent file
    QUrl nonExistentUrl("file:///tmp/nonexistent.txt");
    emit fileProvider->fileRemoved(nonExistentUrl);
    
    // Verify no signals were emitted
    EXPECT_EQ(beginRemoveSpy.count(), 0);
    EXPECT_EQ(endRemoveSpy.count(), 0);
    
    // Verify row count unchanged
    EXPECT_EQ(model->rowCount(model->rootIndex()), 0);
}

TEST_F(UT_FileInfoModelPrivate, replaceData_WithEmptyNewUrl_RemovesOldFile)
{
    // Set up model with existing file
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    QUrl oldUrl("file:///tmp/old.txt");
    emit fileProvider->fileInserted(oldUrl);
    
    // Verify initial state
    EXPECT_EQ(model->rowCount(model->rootIndex()), 1);
    
    // Monitor row removal signals
    QSignalSpy beginRemoveSpy(model, &QAbstractItemModel::rowsAboutToBeRemoved);
    QSignalSpy endRemoveSpy(model, &QAbstractItemModel::rowsRemoved);
    
    // Trigger replaceData with empty new URL
    emit fileProvider->fileRenamed(oldUrl, QUrl());
    
    // Verify file was removed
    EXPECT_EQ(beginRemoveSpy.count(), 1);
    EXPECT_EQ(endRemoveSpy.count(), 1);
    EXPECT_EQ(model->rowCount(model->rootIndex()), 0);
}

TEST_F(UT_FileInfoModelPrivate, replaceData_WithValidNewUrl_ReplacesFile)
{
    // Set up model with existing file
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    QUrl oldUrl("file:///tmp/old.txt");
    emit fileProvider->fileInserted(oldUrl);
    
    // Verify initial state
    EXPECT_EQ(model->rowCount(model->rootIndex()), 1);
    
    // Monitor dataReplaced signal
    QSignalSpy dataReplacedSpy(model, &FileInfoModel::dataReplaced);
    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
    
    // Trigger replaceData
    QUrl newUrl("file:///tmp/new.txt");
    emit fileProvider->fileRenamed(oldUrl, newUrl);
    
    // Verify dataReplaced signal was emitted
    EXPECT_EQ(dataReplacedSpy.count(), 1);
    EXPECT_EQ(dataChangedSpy.count(), 1);
    
    // Verify file was replaced
    QList<QUrl> files = model->files();
    EXPECT_FALSE(files.contains(oldUrl));
    EXPECT_TRUE(files.contains(newUrl));
}

TEST_F(UT_FileInfoModelPrivate, replaceData_OldUrlNotInModel_InsertsNewUrl)
{
    // Set up model with root URL
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    // Monitor row insertion signals
    QSignalSpy beginInsertSpy(model, &QAbstractItemModel::rowsAboutToBeInserted);
    QSignalSpy endInsertSpy(model, &QAbstractItemModel::rowsInserted);
    
    // Try to replace non-existent file
    QUrl oldUrl("file:///tmp/nonexistent.txt");
    QUrl newUrl("file:///tmp/new.txt");
    emit fileProvider->fileRenamed(oldUrl, newUrl);
    
    // Should insert new file
    EXPECT_EQ(beginInsertSpy.count(), 1);
    EXPECT_EQ(endInsertSpy.count(), 1);
    EXPECT_EQ(model->rowCount(model->rootIndex()), 1);
    
    // Verify new file was added
    QList<QUrl> files = model->files();
    EXPECT_TRUE(files.contains(newUrl));
}

TEST_F(UT_FileInfoModelPrivate, updateData_WithExistingUrl_EmitsDataChanged)
{
    // Set up model with existing file
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    QUrl testUrl("file:///tmp/test.txt");
    emit fileProvider->fileInserted(testUrl);
    
    // Monitor dataChanged signal
    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
    
    // Trigger updateData via FileProvider::fileUpdated signal
    emit fileProvider->fileUpdated(testUrl);
    
    // Should emit dataChanged signal
    EXPECT_EQ(dataChangedSpy.count(), 1);
}

TEST_F(UT_FileInfoModelPrivate, updateData_WithNonExistentUrl_DoesNothing)
{
    // Set up model with root URL
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    // Monitor dataChanged signal
    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
    
    // Try to update non-existent file
    QUrl nonExistentUrl("file:///tmp/nonexistent.txt");
    emit fileProvider->fileUpdated(nonExistentUrl);
    
    // Should not emit any signals
    EXPECT_EQ(dataChangedSpy.count(), 0);
}

TEST_F(UT_FileInfoModelPrivate, dataUpdated_WithExistingUrl_RefreshesIcon)
{
    // Set up model with existing file
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    QUrl testUrl("file:///tmp/test.txt");
    emit fileProvider->fileInserted(testUrl);
    
    // Monitor dataChanged signal
    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
    
    // Trigger dataUpdated via FileProvider::fileInfoUpdated signal
    emit fileProvider->fileInfoUpdated(testUrl, false);
    
    // Should emit dataChanged signal
    EXPECT_EQ(dataChangedSpy.count(), 1);
}

TEST_F(UT_FileInfoModelPrivate, dataUpdated_WithNonExistentUrl_DoesNothing)
{
    // Set up model with root URL
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    // Monitor dataChanged signal
    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
    
    // Try to update non-existent file
    QUrl nonExistentUrl("file:///tmp/nonexistent.txt");
    emit fileProvider->fileInfoUpdated(nonExistentUrl, false);
    
    // Should not emit any signals
    EXPECT_EQ(dataChangedSpy.count(), 0);
}

TEST_F(UT_FileInfoModelPrivate, thumbUpdated_WithValidThumbnail_UpdatesFileIcon)
{
    // Set up model with existing file
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    QUrl testUrl("file:///tmp/image.jpg");
    
    // Add the file to the model first by calling insertData directly
    // This ensures the file is in fileMap before we try to update its thumbnail
    model->d->insertData(testUrl);
    
    // Create a valid thumbnail file for testing
    QString thumbnailPath = "/tmp/valid_thumbnail.png";
    
    // Stub QIcon::isNull to make the icon appear valid
    stub.set_lamda(&QIcon::isNull, [](QIcon*) -> bool {
        __DBG_STUB_INVOKE__
        return false; // Always return false to make the icon appear valid
    });
    
    // Monitor dataChanged signal
    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
    
    // Trigger thumbUpdated via FileProvider::fileThumbUpdated signal
    emit fileProvider->fileThumbUpdated(testUrl, thumbnailPath);
    
    // Should emit dataChanged signal for icon role
    EXPECT_EQ(dataChangedSpy.count(), 1);
}

TEST_F(UT_FileInfoModelPrivate, thumbUpdated_WithNonExistentUrl_DoesNothing)
{
    // Set up model with root URL
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    // Monitor dataChanged signal
    QSignalSpy dataChangedSpy(model, &QAbstractItemModel::dataChanged);
    
    // Try to update thumbnail for non-existent file
    QUrl nonExistentUrl("file:///tmp/nonexistent.jpg");
    QString thumbnailPath = "/tmp/thumbnail.png";
    emit fileProvider->fileThumbUpdated(nonExistentUrl, thumbnailPath);
    
    // Should not emit any signals
    EXPECT_EQ(dataChangedSpy.count(), 0);
}

TEST_F(UT_FileInfoModelPrivate, fileIcon_WithValidFileInfo_ReturnsIcon)
{
    // Set up model with root URL
    QUrl rootUrl("file:///tmp");
    model->setRootUrl(rootUrl);
    
    // Create test file info
    QUrl testUrl("file:///tmp/test.txt");
    auto fileInfo = InfoFactory::create<FileInfo>(testUrl);
    ASSERT_NE(fileInfo, nullptr);
    
    // Test fileIcon method - this will call the private method
    QIcon icon = model->d->fileIcon(fileInfo);
    
    // Should return a valid icon
    EXPECT_FALSE(icon.isNull());
}

TEST_F(UT_FileInfoModelPrivate, checkAndRefreshDesktopIcon_WithValidDesktopFile_HandlesIconRefresh)
{
    // This method is complex and involves QTimer, so we'll test basic invocation
    QUrl desktopUrl("file:///tmp/app.desktop");
    auto desktopInfo = InfoFactory::create<FileInfo>(desktopUrl);
    ASSERT_NE(desktopInfo, nullptr);
    
    // Stub QIcon::fromTheme to simulate missing icon
    stub.set_lamda(static_cast<QIcon(*)(const QString&)>(&QIcon::fromTheme), [](const QString&) -> QIcon {
        __DBG_STUB_INVOKE__
        return QIcon(); // Return null icon to trigger retry logic
    });
    
    // Call checkAndRefreshDesktopIcon with positive retry count
    // This will trigger the retry logic and QTimer::singleShot
    EXPECT_NO_THROW(model->d->checkAndRefreshDesktopIcon(desktopInfo, 3));
}

TEST_F(UT_FileInfoModelPrivate, checkAndRefreshDesktopIcon_WithRetriesExhausted_FallsBackToXDG)
{
    // Test the fallback path when retries are exhausted
    QUrl desktopUrl("file:///tmp/app.desktop");
    auto desktopInfo = InfoFactory::create<FileInfo>(desktopUrl);
    ASSERT_NE(desktopInfo, nullptr);
    
    // Stub FileUtils::findIconFromXdg to simulate XDG icon search
    bool xdgSearchCalled = false;
    stub.set_lamda(&FileUtils::findIconFromXdg, [&xdgSearchCalled](const QString&) -> QString {
        __DBG_STUB_INVOKE__
        xdgSearchCalled = true;
        return "/usr/share/icons/app-icon.png"; // Simulate found icon
    });
    
    // Call checkAndRefreshDesktopIcon with retries exhausted
    EXPECT_NO_THROW(model->d->checkAndRefreshDesktopIcon(desktopInfo, -1));
    
    // Verify XDG search was called
    EXPECT_TRUE(xdgSearchCalled);
}
