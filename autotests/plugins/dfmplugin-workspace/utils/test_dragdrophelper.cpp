// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "stubext.h"

#include "utils/dragdrophelper.h"
#include "views/fileview.h"
#include "dfmplugin_workspace_global.h"

#include <dfm-base/interfaces/fileinfo.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/dfm_global_defines.h>
#include <dfm-framework/event/event.h>
#include <dfm-framework/event/eventhelper.h>

#include <QUrl>
#include <QPoint>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QDragLeaveEvent>
#include <QModelIndex>
#include <QMimeData>

using namespace dfmplugin_workspace;

class DragDropHelperTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize test environment
        mockView = new FileView(QUrl::fromLocalFile("/tmp/test"));
        helper = new DragDropHelper(mockView);
    }

    void TearDown() override
    {
        delete helper;
        delete mockView;
        stub.clear();
    }

    FileView *mockView;
    DragDropHelper *helper;
    stub_ext::StubExt stub;
};

TEST_F(DragDropHelperTest, Constructor_ValidView_CreatesHelper)
{
    // Test that constructor creates helper with valid view
    EXPECT_NE(helper, nullptr);
}

TEST_F(DragDropHelperTest, DragEnter_ValidEvent_ReturnsTrue)
{
    // Test drag enter with valid event
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    
    QDragEnterEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, 
                         Qt::LeftButton, Qt::NoModifier);
    
    // Mock checkProhibitPaths
    stub.set_lamda(ADDR(DragDropHelper, checkProhibitPaths), []() {
        return false;
    });
    
    // Mock checkTargetEnable
    stub.set_lamda(ADDR(DragDropHelper, checkTargetEnable), []() {
        return true;
    });
    
    // Mock checkAction
    stub.set_lamda(ADDR(DragDropHelper, checkAction), []() {
        return Qt::CopyAction;
    });
    
    // Mock handleDFileDrag
    stub.set_lamda(ADDR(DragDropHelper, handleDFileDrag), []() {
        return true;
    });
    
    auto result = helper->dragEnter(&event);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, DragEnter_ProhibitedPaths_ReturnsFalse)
{
    // Test drag enter with prohibited paths
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    
    QDragEnterEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, 
                         Qt::LeftButton, Qt::NoModifier);
    
    // Mock checkProhibitPaths to return true (prohibited)
    stub.set_lamda(ADDR(DragDropHelper, checkProhibitPaths), []() {
        return true;
    });
    
    auto result = helper->dragEnter(&event);
    
    EXPECT_FALSE(result);
}

TEST_F(DragDropHelperTest, DragMove_ValidEvent_ReturnsTrue)
{
    // Test drag move with valid event
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    
    QDragMoveEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, 
                        Qt::LeftButton, Qt::NoModifier);
    
    // Mock checkProhibitPaths
    stub.set_lamda(ADDR(DragDropHelper, checkProhibitPaths), []() {
        return false;
    });
    
    // Mock checkTargetEnable
    stub.set_lamda(ADDR(DragDropHelper, checkTargetEnable), []() {
        return true;
    });
    
    // Mock checkAction
    stub.set_lamda(ADDR(DragDropHelper, checkAction), []() {
        return Qt::CopyAction;
    });
    
    // Mock handleDFileDrag
    stub.set_lamda(ADDR(DragDropHelper, handleDFileDrag), []() {
        return true;
    });
    
    auto result = helper->dragMove(&event);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, DragMove_ProhibitedPaths_ReturnsFalse)
{
    // Test drag move with prohibited paths
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    
    QDragMoveEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, 
                        Qt::LeftButton, Qt::NoModifier);
    
    // Mock checkProhibitPaths to return true (prohibited)
    stub.set_lamda(ADDR(DragDropHelper, checkProhibitPaths), []() {
        return true;
    });
    
    auto result = helper->dragMove(&event);
    
    EXPECT_FALSE(result);
}

TEST_F(DragDropHelperTest, DragLeave_ValidEvent_ReturnsTrue)
{
    // Test drag leave with valid event
    QDragLeaveEvent event;
    
    // This should not crash
    auto result = helper->dragLeave(&event);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, Drop_ValidEvent_ReturnsTrue)
{
    // Test drop with valid event
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    
    QDropEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, 
                    Qt::LeftButton, Qt::NoModifier);
    
    // Mock checkProhibitPaths
    stub.set_lamda(ADDR(DragDropHelper, checkProhibitPaths), []() {
        return false;
    });
    
    // Mock checkTargetEnable
    stub.set_lamda(ADDR(DragDropHelper, checkTargetEnable), []() {
        return true;
    });
    
    // Mock checkAction
    stub.set_lamda(ADDR(DragDropHelper, checkAction), []() {
        return Qt::CopyAction;
    });
    
    // Mock handleDropEvent
    stub.set_lamda(ADDR(DragDropHelper, handleDropEvent), []() {
        // Do nothing
    });
    
    auto result = helper->drop(&event);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, Drop_ProhibitedPaths_ReturnsFalse)
{
    // Test drop with prohibited paths
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    
    QDropEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, 
                    Qt::LeftButton, Qt::NoModifier);
    
    // Mock checkProhibitPaths to return true (prohibited)
    stub.set_lamda(ADDR(DragDropHelper, checkProhibitPaths), []() {
        return true;
    });
    
    auto result = helper->drop(&event);
    
    EXPECT_FALSE(result);
}

TEST_F(DragDropHelperTest, IsDragTarget_ValidIndex_ReturnsTrue)
{
    // Test is drag target with valid index
    QModelIndex mockIndex;
    
    // Mock FileView isDragTarget method
    stub.set_lamda(ADDR(FileView, isDragTarget), [](FileView *, const QModelIndex &) {
        return true;
    });
    
    auto result = helper->isDragTarget(mockIndex);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, IsDragTarget_InvalidIndex_ReturnsFalse)
{
    // Test is drag target with invalid index
    QModelIndex invalidIndex;
    
    // Mock FileView isDragTarget method
    stub.set_lamda(ADDR(FileView, isDragTarget), [](FileView *, const QModelIndex &) {
        return false;
    });
    
    auto result = helper->isDragTarget(invalidIndex);
    
    EXPECT_FALSE(result);
}

TEST_F(DragDropHelperTest, HandleDFileDrag_ValidData_ReturnsTrue)
{
    // Test handle DFile drag with valid data
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    
    QUrl testUrl("file:///tmp");
    
    // This should not crash
    auto result = helper->handleDFileDrag(&mimeData, testUrl);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, HandleDFileDrag_InvalidData_ReturnsFalse)
{
    // Test handle DFile drag with invalid data
    QMimeData mimeData;
    // Don't set URLs
    
    QUrl testUrl("file:///tmp");
    
    // This should not crash
    auto result = helper->handleDFileDrag(&mimeData, testUrl);
    
    EXPECT_FALSE(result);
}

TEST_F(DragDropHelperTest, HandleDropEvent_ValidEvent_HandlesEvent)
{
    // Test handle drop event with valid event
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    
    QDropEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, 
                    Qt::LeftButton, Qt::NoModifier);
    
    // Mock checkProhibitPaths
    stub.set_lamda(ADDR(DragDropHelper, checkProhibitPaths), []() {
        return false;
    });
    
    // Mock checkTargetEnable
    stub.set_lamda(ADDR(DragDropHelper, checkTargetEnable), []() {
        return true;
    });
    
    // Mock checkAction
    stub.set_lamda(ADDR(DragDropHelper, checkAction), []() {
        return Qt::CopyAction;
    });
    
    bool fall = false;
    
    // This should not crash
    helper->handleDropEvent(&event, &fall);
    
    EXPECT_FALSE(fall);
}

TEST_F(DragDropHelperTest, FileInfoAtPos_ValidPosition_ReturnsFileInfo)
{
    // Test file info at position with valid position
    QPoint pos(10, 10);
    
    // This should not crash
    auto result = helper->fileInfoAtPos(pos);
    
    // Should return null for mock view
    EXPECT_EQ(result, nullptr);
}

TEST_F(DragDropHelperTest, CheckProhibitPaths_ValidEvent_ReturnsFalse)
{
    // Test check prohibit paths with valid event
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/tmp/test.txt") };
    mimeData.setUrls(urls);
    
    QDragEnterEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, 
                         Qt::LeftButton, Qt::NoModifier);
    
    // This should not crash
    auto result = helper->checkProhibitPaths(&event, urls);
    
    EXPECT_FALSE(result);
}

TEST_F(DragDropHelperTest, CheckProhibitPaths_ProhibitedPath_ReturnsTrue)
{
    // Test check prohibit paths with prohibited path
    QMimeData mimeData;
    QList<QUrl> urls = { QUrl::fromLocalFile("/proc") }; // Prohibited path
    mimeData.setUrls(urls);
    
    QDragEnterEvent event(QPoint(10, 10), Qt::CopyAction, &mimeData, 
                         Qt::LeftButton, Qt::NoModifier);
    
    // This should not crash
    auto result = helper->checkProhibitPaths(&event, urls);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, CheckTargetEnable_ValidUrl_ReturnsTrue)
{
    // Test check target enable with valid URL
    QUrl targetUrl("file:///tmp");
    
    // This should not crash
    auto result = helper->checkTargetEnable(targetUrl);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, CheckTargetEnable_InvalidUrl_ReturnsFalse)
{
    // Test check target enable with invalid URL
    QUrl invalidUrl; // Empty URL
    
    // This should not crash
    auto result = helper->checkTargetEnable(invalidUrl);
    
    EXPECT_FALSE(result);
}

TEST_F(DragDropHelperTest, CheckAction_ValidParameters_ReturnsExpectedAction)
{
    // Test check action with valid parameters
    Qt::DropAction srcAction = Qt::CopyAction;
    bool sameUser = true;
    
    // This should not crash
    auto result = helper->checkAction(srcAction, sameUser);
    
    // Should return source action for same user
    EXPECT_EQ(result, srcAction);
}

TEST_F(DragDropHelperTest, CheckAction_DifferentUser_ReturnsExpectedAction)
{
    // Test check action with different user
    Qt::DropAction srcAction = Qt::MoveAction;
    bool sameUser = false;
    
    // This should not crash
    auto result = helper->checkAction(srcAction, sameUser);
    
    // Should return CopyAction for different user
    EXPECT_EQ(result, Qt::CopyAction);
}

TEST_F(DragDropHelperTest, CheckDragEnable_ValidUrls_ReturnsTrue)
{
    // Test check drag enable with valid URLs
    QUrl dragUrl("file:///tmp/test.txt");
    QUrl targetUrl("file:///tmp");
    
    // This should not crash
    auto result = helper->checkDragEnable(dragUrl, targetUrl);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, CheckDragEnable_SameUrl_ReturnsFalse)
{
    // Test check drag enable with same URLs
    QUrl sameUrl("file:///tmp/test.txt");
    
    // This should not crash
    auto result = helper->checkDragEnable(sameUrl, sameUrl);
    
    EXPECT_FALSE(result);
}

TEST_F(DragDropHelperTest, CheckMoveEnable_ValidUrls_ReturnsTrue)
{
    // Test check move enable with valid URLs
    QUrl dragUrl("file:///tmp/test.txt");
    QUrl toUrl("file:///tmp");
    
    // Mock InfoFactory::create<FileInfo> to return a valid mock FileInfo
    stub.set_lamda(ADDR(dfmbase::InfoFactory, create<dfmbase::FileInfo>), [dragUrl](const QUrl &url, dfmbase::Global::CreateFileInfoType, QString *) {
        __DBG_STUB_INVOKE__
        
        // Create a simple mock FileInfo object
        class MockFileInfo : public dfmbase::FileInfo {
        public:
            explicit MockFileInfo(const QUrl &url) : dfmbase::FileInfo(url) {}
            
            bool canAttributes(FileCanType type) const override {
                return type == FileCanType::kCanMoveOrCopy ||
                       type == FileCanType::kCanRename;
            }
            
            bool isAttributes(FileIsType type) const override {
                return type == FileIsType::kIsFile;
            }
            
            QUrl urlOf(dfmbase::FileInfo::FileUrlInfoType type) const override {
                if (type == dfmbase::FileInfo::FileUrlInfoType::kUrl)
                    return QUrl("file:///tmp/test.txt");
                return QUrl();
            }
        };
        
        return QSharedPointer<MockFileInfo>::create(url);
    });
    
    // Mock dpfHookSequence->run to return false
    // stub.set_lamda(VADDR(QKeySequence, run), []() {
    //     __DBG_STUB_INVOKE__
    //     return false;
    // });
    
    // This should not crash
    auto result = helper->checkMoveEnable(dragUrl, toUrl);
    
    EXPECT_TRUE(result);
}

TEST_F(DragDropHelperTest, CheckMoveEnable_SameUrl_ReturnsFalse)
{
    // Test check move enable with same URLs
    QUrl sameUrl("file:///tmp/test.txt");
    
    // Mock InfoFactory::create<FileInfo> to return a valid mock FileInfo
    stub.set_lamda(ADDR(dfmbase::InfoFactory, create<dfmbase::FileInfo>), [sameUrl](const QUrl &url, dfmbase::Global::CreateFileInfoType, QString *) {
        __DBG_STUB_INVOKE__
        
        // Create a simple mock FileInfo object
        class MockFileInfo : public dfmbase::FileInfo {
        public:
            explicit MockFileInfo(const QUrl &url) : dfmbase::FileInfo(url) {}
            
            bool canAttributes(FileCanType type) const override {
                return type == FileCanType::kCanMoveOrCopy ||
                       type == FileCanType::kCanRename;
            }
            
            bool isAttributes(FileIsType type) const override {
                return type == FileIsType::kIsFile;
            }
            
            QUrl urlOf(dfmbase::FileInfo::FileUrlInfoType type) const override {
                if (type == dfmbase::FileInfo::FileUrlInfoType::kUrl)
                    return QUrl("file:///tmp/test.txt");
                return QUrl();
            }
        };
        
        return QSharedPointer<MockFileInfo>::create(url);
    });
    
    // Mock dpfHookSequence->run to return false
    // stub.set_lamda(VADDR(QKeySequence, run), []() {
    //     __DBG_STUB_INVOKE__
    //     return false;
    // });
    
    // This should not crash
    auto result = helper->checkMoveEnable(sameUrl, sameUrl);
    
    EXPECT_FALSE(result);
}
